#include "AI/ExoBotController.h"
#include "AI/ExoBotCharacter.h"
#include "Player/ExoCharacter.h"
#include "Map/ExoZoneSystem.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Visual/ExoWeatherSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
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
// Full AI — combat, cover, strafing, loot
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

	// Cover check — low health bots flee
	if (ShouldSeekCover())
	{
		StopFiring();
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
			MoveToTarget();
		}
		else
		{
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
		}
	}
	else
	{
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

// ---------------------------------------------------------------------------
// Combat
// ---------------------------------------------------------------------------

void AExoBotController::FindTarget()
{
	CurrentTarget = nullptr;
	APawn* BotPawn = GetPawn();
	if (!BotPawn) return;

	float VisibilityMult = 1.f;
	if (AExoWeatherSystem* Weather = AExoWeatherSystem::Get(GetWorld()))
		VisibilityMult = Weather->GetVisibilityMultiplier();

	float EffectiveSight = SightRadius * VisibilityMult;

	TArray<AActor*> PerceivedActors;
	AIPerception->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

	// Score targets: prioritize low-health, close, and DBNO enemies
	float BestScore = -1.f;
	for (AActor* Actor : PerceivedActors)
	{
		AExoCharacter* Other = Cast<AExoCharacter>(Actor);
		if (!Other || !Other->IsAlive() || Other == BotPawn) continue;

		float Dist = FVector::Dist(BotPawn->GetActorLocation(), Other->GetActorLocation());
		if (Dist > EffectiveSight) continue;

		// Score: closer is better, low health is better, DBNO is priority target
		float DistScore = 1.f - FMath::Clamp(Dist / EffectiveSight, 0.f, 1.f);
		float HealthScore = 1.f - FMath::Clamp(Other->GetHealth() / Other->GetMaxHealth(), 0.f, 1.f);
		float DBNOBonus = Other->IsDBNO() ? 0.5f : 0.f;
		float Score = DistScore * 0.4f + HealthScore * 0.3f + DBNOBonus + 0.3f;

		if (Score > BestScore)
		{
			BestScore = Score;
			CurrentTarget = Other;
		}
	}

	// New target = trigger reaction delay
	if (CurrentTarget && !bReactionPending)
	{
		bReactionPending = true;
		ReactionTimer = ReactionTime;
	}
}

void AExoBotController::AimAtTarget(float DeltaTime)
{
	if (!CurrentTarget || !GetPawn()) return;

	FVector TargetLoc = CurrentTarget->GetActorLocation();

	// Jitter based on accuracy — less accurate bots aim worse
	float Jitter = (1.f - AimAccuracy) * 150.f;
	TargetLoc += FVector(
		FMath::RandRange(-Jitter, Jitter),
		FMath::RandRange(-Jitter, Jitter),
		FMath::RandRange(-Jitter * 0.3f, Jitter * 0.3f)
	);

	FVector Dir = (TargetLoc - GetPawn()->GetActorLocation()).GetSafeNormal();
	FRotator DesiredRot = Dir.Rotation();
	FRotator CurrentRot = GetPawn()->GetActorRotation();

	float InterpSpeed = FMath::Lerp(3.f, 15.f, AimAccuracy);
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, InterpSpeed);
	GetPawn()->SetActorRotation(NewRot);
	SetControlRotation(NewRot);
}

void AExoBotController::TryFire()
{
	if (AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn()))
		if (CurrentTarget) Bot->StartFire();
}

void AExoBotController::StopFiring()
{
	if (AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn()))
		Bot->StopFire();
}

// ---------------------------------------------------------------------------
// Strafing — makes bots harder to hit during combat
// ---------------------------------------------------------------------------

void AExoBotController::UpdateStrafeDirection(float DeltaTime)
{
	StrafeTimer += DeltaTime;
	if (StrafeTimer >= StrafeChangeInterval)
	{
		StrafeTimer = 0.f;
		StrafeChangeInterval = FMath::RandRange(0.8f, 2.5f);

		if (FMath::FRand() < StrafeChance)
			StrafeDirection = (FMath::FRand() < 0.5f) ? 1.f : -1.f;
		else
			StrafeDirection = 0.f; // Stand still sometimes
	}
}

void AExoBotController::ApplyStrafe(float DeltaTime)
{
	if (FMath::IsNearlyZero(StrafeDirection) || !GetPawn()) return;

	FVector Right = GetPawn()->GetActorRightVector();
	FVector StrafeOffset = Right * StrafeDirection * 300.f * DeltaTime;
	FVector NewLoc = GetPawn()->GetActorLocation() + StrafeOffset;
	GetPawn()->SetActorLocation(NewLoc, true);
}

bool AExoBotController::HasLineOfSight(AActor* Target) const
{
	if (!Target || !GetPawn()) return false;
	FHitResult Hit;
	FCollisionQueryParams QP;
	QP.AddIgnoredActor(GetPawn());
	return !GetWorld()->LineTraceSingleByChannel(Hit, GetPawn()->GetActorLocation(),
		Target->GetActorLocation(), ECC_Visibility, QP);
}

// ---------------------------------------------------------------------------
// Cover
// ---------------------------------------------------------------------------

bool AExoBotController::ShouldSeekCover() const
{
	AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn());
	if (!Bot) return false;
	return Bot->GetHealth() < CoverHealthThreshold && CurrentTarget != nullptr;
}

void AExoBotController::SeekCover()
{
	if (!GetPawn() || !CurrentTarget) return;

	// Move away from the target (simple flee)
	FVector AwayDir = (GetPawn()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
	FVector FleeTarget = GetPawn()->GetActorLocation() + AwayDir * 2000.f;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(FleeTarget, NavLoc))
		{
			MoveToLocation(NavLoc.Location, 200.f);
			return;
		}
	}
	MoveToLocation(FleeTarget, 200.f);
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

void AExoBotController::MoveTowardZone()
{
	if (!ZoneSystem || !GetPawn()) return;
	FVector2D ZoneCenter = ZoneSystem->GetCurrentCenter();
	FVector Target(ZoneCenter.X, ZoneCenter.Y, GetPawn()->GetActorLocation().Z);
	MoveToLocation(Target, 500.f);
}

void AExoBotController::WanderRandomly()
{
	WanderTimer -= GetWorld()->GetDeltaSeconds();
	if (WanderTimer > 0.f) return;

	WanderTimer = FMath::RandRange(3.f, 8.f);
	APawn* BotPawn = GetPawn();
	if (!BotPawn) return;

	FVector Origin = BotPawn->GetActorLocation();
	FVector RandomOffset(FMath::RandRange(-3000.f, 3000.f), FMath::RandRange(-3000.f, 3000.f), 0.f);
	WanderTarget = Origin + RandomOffset;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->ProjectPointToNavigation(WanderTarget, NavLoc))
		{
			MoveToLocation(NavLoc.Location, 200.f);
			return;
		}
	}
	MoveToLocation(WanderTarget, 200.f);
}

void AExoBotController::MoveToTarget()
{
	if (!CurrentTarget) return;
	MoveToActor(CurrentTarget, EngageRange * 0.8f);
}

void AExoBotController::LookForLoot()
{
	LootSearchTimer -= GetWorld()->GetDeltaSeconds();
	if (LootSearchTimer > 0.f) return;
	LootSearchTimer = 5.f;

	LootTarget = nullptr;
	APawn* BotPawn = GetPawn();
	if (!BotPawn) return;

	float BestDist = 3000.f; // Only grab nearby loot
	for (TActorIterator<AExoWeaponPickup> It(GetWorld()); It; ++It)
	{
		float Dist = FVector::Dist(BotPawn->GetActorLocation(), It->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			LootTarget = *It;
		}
	}
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
