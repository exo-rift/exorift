// ExoBotCombat.cpp — Bot combat, cover, navigation, and loot behaviors
#include "AI/ExoBotController.h"
#include "AI/ExoBotCharacter.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoDecoyActor.h"
#include "Map/ExoZoneSystem.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoGrenadeComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Visual/ExoWeatherSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "ExoRift.h"

// ---------------------------------------------------------------------------
// Combat — target selection, aiming, firing
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
		if (!Actor || Actor == BotPawn) continue;

		float Dist = FVector::Dist(BotPawn->GetActorLocation(), Actor->GetActorLocation());
		if (Dist > EffectiveSight) continue;

		// Check for decoys — treat as lower-priority targets
		AExoDecoyActor* Decoy = Cast<AExoDecoyActor>(Actor);
		if (Decoy)
		{
			float DistScore = 1.f - FMath::Clamp(Dist / EffectiveSight, 0.f, 1.f);
			// Decoys score lower than real players (0.5 base vs 0.3+ for characters)
			float Score = DistScore * 0.4f + 0.1f;
			if (Score > BestScore)
			{
				BestScore = Score;
				CurrentTarget = Decoy;
			}
			continue;
		}

		AExoCharacter* Other = Cast<AExoCharacter>(Actor);
		if (!Other || !Other->IsAlive()) continue;

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
	if (AExoBotCharacter* Bot = Cast<AExoBotCharacter>(GetPawn()))
	{
		if (CurrentTarget)
		{
			// Audible fire callout — alerts nearby players when bot opens fire
			Bot->PlayFireCallout();
			Bot->StartFire();
		}
	}
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

	// Unarmed bots search more aggressively (wider radius, faster retry)
	AExoCharacter* BotChar = Cast<AExoCharacter>(GetPawn());
	bool bUnarmed = true;
	if (BotChar)
		if (UExoInventoryComponent* Inv = BotChar->GetInventoryComponent())
			bUnarmed = (Inv->GetCurrentWeapon() == nullptr);

	LootSearchTimer = bUnarmed ? 1.f : 5.f;
	float SearchRadius = bUnarmed ? 15000.f : 3000.f;

	LootTarget = nullptr;
	APawn* BotPawn = GetPawn();
	if (!BotPawn) return;

	float BestDist = SearchRadius;
	for (TActorIterator<AExoWeaponPickup> It(GetWorld()); It; ++It)
	{
		float Dist = FVector::Dist(BotPawn->GetActorLocation(), It->GetActorLocation());

		// Auto-pickup when bot is close enough
		if (Dist < 250.f && BotChar)
		{
			It->Interact(BotChar);
			LootTarget = nullptr;
			return;
		}

		if (Dist < BestDist)
		{
			BestDist = Dist;
			LootTarget = *It;
		}
	}
}

// ---------------------------------------------------------------------------
// Grenade usage — throw at clusters or stationary targets
// ---------------------------------------------------------------------------

void AExoBotController::TryThrowGrenade()
{
	GrenadeTimer -= GetWorld()->GetDeltaSeconds();
	if (GrenadeTimer > 0.f) return;

	AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn());
	if (!Bot || !CurrentTarget) return;

	float Dist = FVector::Dist(Bot->GetActorLocation(), CurrentTarget->GetActorLocation());
	if (Dist < 1500.f || Dist > 4000.f) return; // Only at medium range

	// Higher difficulty = more likely to grenade
	float Chance = FMath::Lerp(0.1f, 0.6f, AimAccuracy);
	if (FMath::FRand() > Chance) return;

	Bot->ThrowGrenade();
	GrenadeTimer = GrenadeCooldown;
	UE_LOG(LogExoRift, Verbose, TEXT("Bot threw grenade at target %.0f units away"), Dist);
}

// ---------------------------------------------------------------------------
// DBNO execution — finish off downed enemies nearby
// ---------------------------------------------------------------------------

void AExoBotController::TryExecuteDBNO()
{
	ExecutionSearchTimer -= GetWorld()->GetDeltaSeconds();
	if (ExecutionSearchTimer > 0.f) return;
	ExecutionSearchTimer = 2.f;

	AExoCharacter* Bot = Cast<AExoCharacter>(GetPawn());
	if (!Bot || Bot->IsExecuting()) return;

	// Find nearest DBNO enemy
	AExoCharacter* BestTarget = nullptr;
	float BestDist = 300.f; // Must be very close to execute

	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || Char == Bot || !Char->IsDBNO()) continue;

		float Dist = FVector::Dist(Bot->GetActorLocation(), Char->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestTarget = Char;
		}
	}

	if (BestTarget)
	{
		Bot->StartExecution(BestTarget);
	}
}
