#include "AI/ExoBotController.h"
#include "AI/ExoBotCharacter.h"
#include "Player/ExoCharacter.h"
#include "Map/ExoZoneSystem.h"
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

	// Perception setup
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
}

void AExoBotController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Find zone system
	for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
	{
		ZoneSystem = *It;
		break;
	}

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AExoBotController::OnTargetPerceptionUpdated);
}

void AExoBotController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AExoBotCharacter* Bot = Cast<AExoBotCharacter>(GetPawn());
	if (!Bot || !Bot->IsAlive()) return;

	// Update LOD
	LODUpdateTimer += DeltaTime;
	if (LODUpdateTimer >= LODUpdateInterval)
	{
		UpdateLODLevel();
		LODUpdateTimer = 0.f;
	}

	switch (CurrentLOD)
	{
	case EAILODLevel::Full:
		TickFullAI(DeltaTime);
		break;
	case EAILODLevel::Simplified:
		TickSimplifiedAI(DeltaTime);
		break;
	case EAILODLevel::Basic:
		TickBasicAI(DeltaTime);
		break;
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
	if (MinDist < 10000.f)      // <100m
		NewLOD = EAILODLevel::Full;
	else if (MinDist < 50000.f) // 100-500m
		NewLOD = EAILODLevel::Simplified;
	else
		NewLOD = EAILODLevel::Basic;

	if (NewLOD != CurrentLOD)
	{
		CurrentLOD = NewLOD;
		AExoBotCharacter* Bot = Cast<AExoBotCharacter>(GetPawn());
		if (Bot) Bot->SetAILODLevel(CurrentLOD);

		// Disable perception at basic LOD
		if (AIPerception)
		{
			AIPerception->SetActive(CurrentLOD != EAILODLevel::Basic);
		}
	}
}

void AExoBotController::TickFullAI(float DeltaTime)
{
	// Check zone safety
	if (ZoneSystem && GetPawn() && !ZoneSystem->IsInsideZone(GetPawn()->GetActorLocation()))
	{
		MoveTowardZone();
		return;
	}

	// Target management
	TargetSearchTimer += DeltaTime;
	if (TargetSearchTimer >= TargetSearchInterval || !CurrentTarget)
	{
		FindTarget();
		TargetSearchTimer = 0.f;
	}

	if (CurrentTarget)
	{
		float DistToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());

		if (DistToTarget > EngageRange)
		{
			MoveToTarget();
		}
		else
		{
			StopMovement();
			AimAtTarget(DeltaTime);
			TryFire();
		}
	}
	else
	{
		WanderRandomly();
	}
}

void AExoBotController::TickSimplifiedAI(float DeltaTime)
{
	// Zone check
	if (ZoneSystem && GetPawn() && !ZoneSystem->IsInsideZone(GetPawn()->GetActorLocation()))
	{
		MoveTowardZone();
		return;
	}

	// Simplified: just move toward targets or wander, reduced combat
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
	// Minimal: just wander and avoid zone
	if (ZoneSystem && GetPawn() && !ZoneSystem->IsInsideZone(GetPawn()->GetActorLocation()))
	{
		MoveTowardZone();
		return;
	}

	StopFiring();
	WanderRandomly();
}

void AExoBotController::FindTarget()
{
	CurrentTarget = nullptr;
	APawn* BotPawn = GetPawn();
	if (!BotPawn) return;

	// Weather reduces effective sight range
	float VisibilityMult = 1.f;
	AExoWeatherSystem* Weather = AExoWeatherSystem::Get(GetWorld());
	if (Weather)
	{
		VisibilityMult = Weather->GetVisibilityMultiplier();
	}
	float EffectiveSight = SightRadius * VisibilityMult;

	TArray<AActor*> PerceivedActors;
	AIPerception->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

	float BestDist = MAX_FLT;
	for (AActor* Actor : PerceivedActors)
	{
		AExoCharacter* OtherChar = Cast<AExoCharacter>(Actor);
		if (!OtherChar || !OtherChar->IsAlive() || OtherChar == BotPawn) continue;

		float Dist = FVector::Dist(BotPawn->GetActorLocation(), OtherChar->GetActorLocation());
		if (Dist > EffectiveSight) continue; // Weather limits detection range

		if (Dist < BestDist)
		{
			BestDist = Dist;
			CurrentTarget = OtherChar;
		}
	}
}

void AExoBotController::AimAtTarget(float DeltaTime)
{
	if (!CurrentTarget || !GetPawn()) return;

	FVector TargetLoc = CurrentTarget->GetActorLocation();
	// Add some inaccuracy based on AimAccuracy
	float Jitter = (1.f - AimAccuracy) * 100.f;
	TargetLoc += FVector(
		FMath::RandRange(-Jitter, Jitter),
		FMath::RandRange(-Jitter, Jitter),
		FMath::RandRange(-Jitter * 0.5f, Jitter * 0.5f)
	);

	FVector Direction = (TargetLoc - GetPawn()->GetActorLocation()).GetSafeNormal();
	FRotator DesiredRot = Direction.Rotation();

	// Smooth rotation
	FRotator CurrentRot = GetPawn()->GetActorRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, 5.f / ReactionTime);
	GetPawn()->SetActorRotation(NewRot);
	SetControlRotation(NewRot);
}

void AExoBotController::TryFire()
{
	AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn());
	if (Bot && CurrentTarget)
	{
		Bot->StartFire();
	}
}

void AExoBotController::StopFiring()
{
	AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn());
	if (Bot) Bot->StopFire();
}

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
	FVector RandomOffset(
		FMath::RandRange(-3000.f, 3000.f),
		FMath::RandRange(-3000.f, 3000.f),
		0.f
	);

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

	// Fallback: direct movement
	MoveToLocation(WanderTarget, 200.f);
}

void AExoBotController::MoveToTarget()
{
	if (!CurrentTarget) return;
	MoveToActor(CurrentTarget, EngageRange * 0.8f);
}

void AExoBotController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;

	AExoCharacter* OtherChar = Cast<AExoCharacter>(Actor);
	if (OtherChar && OtherChar->IsAlive() && !CurrentTarget)
	{
		CurrentTarget = OtherChar;
	}
}
