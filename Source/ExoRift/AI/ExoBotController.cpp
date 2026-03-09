// ExoBotController.cpp — Core AI: setup, tick dispatch, LOD, behavior trees
// Combat and navigation logic in ExoBotCombat.cpp
#include "AI/ExoBotController.h"
#include "AI/ExoBotCharacter.h"
#include "Player/ExoCharacter.h"
#include "Map/ExoZoneSystem.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoBotController::AExoBotController()
{
	PrimaryActorTick.bCanEverTick = true;

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = SightRadius * 1.2f;
	SightConfig->PeripheralVisionAngleDegrees = 80.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerception->ConfigureSense(*SightConfig);

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 3000.f;
	HearingConfig->SetMaxAge(3.f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerception->ConfigureSense(*HearingConfig);

	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	// Default to medium
	SetDifficulty(EBotDifficulty::Medium);
}

void AExoBotController::SetDifficulty(EBotDifficulty NewDifficulty)
{
	Difficulty = NewDifficulty;
	switch (Difficulty)
	{
	case EBotDifficulty::Easy:
		AimAccuracy = 0.4f; ReactionTime = 0.8f; StrafeChance = 0.1f;
		CoverHealthThreshold = 20.f; BurstDuration = 1.5f; BurstPause = 1.0f;
		break;
	case EBotDifficulty::Medium:
		AimAccuracy = 0.65f; ReactionTime = 0.4f; StrafeChance = 0.4f;
		CoverHealthThreshold = 40.f; BurstDuration = 2.0f; BurstPause = 0.6f;
		break;
	case EBotDifficulty::Hard:
		AimAccuracy = 0.85f; ReactionTime = 0.2f; StrafeChance = 0.7f;
		CoverHealthThreshold = 50.f; BurstDuration = 2.5f; BurstPause = 0.3f;
		break;
	case EBotDifficulty::Elite:
		AimAccuracy = 0.95f; ReactionTime = 0.1f; StrafeChance = 0.9f;
		CoverHealthThreshold = 60.f; BurstDuration = 3.0f; BurstPause = 0.15f;
		break;
	}
}

void AExoBotController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
	{
		ZoneSystem = *It;
		break;
	}

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AExoBotController::OnTargetPerceptionUpdated);

	// Randomize difficulty: 40% Easy, 30% Medium, 20% Hard, 10% Elite
	float Roll = FMath::FRand();
	if (Roll < 0.4f) SetDifficulty(EBotDifficulty::Easy);
	else if (Roll < 0.7f) SetDifficulty(EBotDifficulty::Medium);
	else if (Roll < 0.9f) SetDifficulty(EBotDifficulty::Hard);
	else SetDifficulty(EBotDifficulty::Elite);
}

void AExoBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AExoBotCharacter* Bot = Cast<AExoBotCharacter>(GetPawn());
	if (!Bot || !Bot->IsAlive()) return;

	LODUpdateTimer += DeltaTime;
	if (LODUpdateTimer >= LODUpdateInterval)
	{
		UpdateLODLevel();
		LODUpdateTimer = 0.f;
	}

	switch (CurrentLOD)
	{
	case EAILODLevel::Full:      TickFullAI(DeltaTime); break;
	case EAILODLevel::Simplified: TickSimplifiedAI(DeltaTime); break;
	case EAILODLevel::Basic:     TickBasicAI(DeltaTime); break;
	}
}

void AExoBotController::UpdateLODLevel()
{
	APawn* BotPawn = GetPawn();
	if (!BotPawn) return;

	float MinDist = MAX_FLT;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			float Dist = FVector::Dist(BotPawn->GetActorLocation(), PC->GetPawn()->GetActorLocation());
			MinDist = FMath::Min(MinDist, Dist);
		}
	}

	EAILODLevel NewLOD;
	if (MinDist < 10000.f)      NewLOD = EAILODLevel::Full;
	else if (MinDist < 50000.f) NewLOD = EAILODLevel::Simplified;
	else                        NewLOD = EAILODLevel::Basic;

	if (NewLOD != CurrentLOD)
	{
		CurrentLOD = NewLOD;
		if (AExoBotCharacter* Bot = Cast<AExoBotCharacter>(GetPawn()))
			Bot->SetAILODLevel(CurrentLOD);
		if (AIPerception)
			AIPerception->SetActive(CurrentLOD != EAILODLevel::Basic);
	}
}

// ---------------------------------------------------------------------------
// AI LOD tick implementations
// ---------------------------------------------------------------------------

void AExoBotController::TickFullAI(float DeltaTime)
{
	AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn());
	if (!Bot) return;

	// Zone safety first
	if (ZoneSystem && !ZoneSystem->IsInsideZone(Bot->GetActorLocation()))
	{
		StopFiring();
		MoveTowardZone();
		return;
	}

	// Cover check — low health bots flee (sprinting)
	if (ShouldSeekCover())
	{
		StopFiring();
		Bot->StartSprint();
		SeekCover();
		return;
	}

	// Target search
	TargetSearchTimer += DeltaTime;
	if (TargetSearchTimer >= TargetSearchInterval || !CurrentTarget)
	{
		FindTarget();
		TargetSearchTimer = 0.f;
	}

	// Reaction delay on new target
	if (bReactionPending)
	{
		ReactionTimer -= DeltaTime;
		if (ReactionTimer <= 0.f) bReactionPending = false;
		else return;
	}

	if (CurrentTarget)
	{
		float DistToTarget = FVector::Dist(Bot->GetActorLocation(), CurrentTarget->GetActorLocation());

		if (DistToTarget > EngageRange)
		{
			StopFiring();
			// Sprint while closing distance
			Bot->StartSprint();
			MoveToTarget();
		}
		else
		{
			Bot->StopSprint();
			StopMovement();
			AimAtTarget(DeltaTime);

			// Burst fire control
			BurstTimer += DeltaTime;
			if (bInBurst)
			{
				TryFire();
				if (BurstTimer >= BurstDuration)
				{
					StopFiring();
					bInBurst = false;
					BurstTimer = 0.f;
				}
			}
			else
			{
				if (BurstTimer >= BurstPause)
				{
					bInBurst = true;
					BurstTimer = 0.f;
				}
			}

			// Strafe during combat
			UpdateStrafeDirection(DeltaTime);
			ApplyStrafe(DeltaTime);

			// Grenade at medium range
			TryThrowGrenade();
		}
	}
	else
	{
		Bot->StopSprint();

		// Try to execute nearby DBNO enemies
		TryExecuteDBNO();

		// No target — look for loot or wander
		LookForLoot();
		if (LootTarget)
			MoveToActor(LootTarget, 100.f);
		else
			WanderRandomly();
	}
}

void AExoBotController::TickSimplifiedAI(float DeltaTime)
{
	if (ZoneSystem && GetPawn() && !ZoneSystem->IsInsideZone(GetPawn()->GetActorLocation()))
	{
		MoveTowardZone();
		return;
	}

	if (CurrentTarget)
	{
		float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
		if (DistToTarget < EngageRange * 0.5f)
		{
			AimAtTarget(DeltaTime);
			TryFire();
		}
		else
		{
			MoveToTarget();
		}
	}
	else
	{
		WanderRandomly();
	}
}

void AExoBotController::TickBasicAI(float DeltaTime)
{
	if (ZoneSystem && GetPawn() && !ZoneSystem->IsInsideZone(GetPawn()->GetActorLocation()))
	{
		MoveTowardZone();
		return;
	}
	StopFiring();
	WanderRandomly();
}

void AExoBotController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;

	AExoCharacter* Other = Cast<AExoCharacter>(Actor);
	if (Other && Other->IsAlive() && !CurrentTarget)
	{
		CurrentTarget = Other;
		bReactionPending = true;
		ReactionTimer = ReactionTime;
	}
}
