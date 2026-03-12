#include "Map/ExoZoneSystem.h"
#include "Player/ExoCharacter.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "ExoRift.h"

AExoZoneSystem::AExoZoneSystem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Default zone stages (compact 800m map = 400m radius)
	FZoneStage Stage1;
	Stage1.Center = FVector2D(0.f, 0.f);
	Stage1.Radius = 38000.f;
	Stage1.ShrinkDuration = 60.f;
	Stage1.HoldDuration = 90.f;
	Stage1.DamagePerSecond = 3.f;
	Stages.Add(Stage1);

	FZoneStage Stage2;
	Stage2.Center = FVector2D(FMath::RandRange(-10000.f, 10000.f), FMath::RandRange(-10000.f, 10000.f));
	Stage2.Radius = 25000.f;
	Stage2.ShrinkDuration = 45.f;
	Stage2.HoldDuration = 60.f;
	Stage2.DamagePerSecond = 8.f;
	Stages.Add(Stage2);

	FZoneStage Stage3;
	Stage3.Center = FVector2D(FMath::RandRange(-6000.f, 6000.f), FMath::RandRange(-6000.f, 6000.f));
	Stage3.Radius = 12000.f;
	Stage3.ShrinkDuration = 30.f;
	Stage3.HoldDuration = 45.f;
	Stage3.DamagePerSecond = 15.f;
	Stages.Add(Stage3);

	FZoneStage Stage4;
	Stage4.Center = FVector2D(FMath::RandRange(-3000.f, 3000.f), FMath::RandRange(-3000.f, 3000.f));
	Stage4.Radius = 5000.f;
	Stage4.ShrinkDuration = 20.f;
	Stage4.HoldDuration = 20.f;
	Stage4.DamagePerSecond = 30.f;
	Stages.Add(Stage4);

	FZoneStage StageFinal;
	StageFinal.Center = FVector2D::ZeroVector;
	StageFinal.Radius = 500.f;
	StageFinal.ShrinkDuration = 15.f;
	StageFinal.HoldDuration = 0.f;
	StageFinal.DamagePerSecond = 50.f;
	Stages.Add(StageFinal);
}

void AExoZoneSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsActive || !HasAuthority()) return;

	if (bIsShrinking)
	{
		TickShrink(DeltaTime);
	}
	else
	{
		TickHold(DeltaTime);
	}

	ApplyZoneDamage(DeltaTime);
}

void AExoZoneSystem::StartZoneSequence()
{
	bIsActive = true;
	CurrentRadius = 40000.f;
	CurrentCenter = FVector2D::ZeroVector;
	CurrentStage = -1;
	AdvanceStage();
}

void AExoZoneSystem::AdvanceStage()
{
	CurrentStage++;
	if (CurrentStage >= Stages.Num())
	{
		bIsActive = false;
		UE_LOG(LogExoRift, Log, TEXT("All zone stages complete"));
		return;
	}

	const FZoneStage& Stage = Stages[CurrentStage];
	TargetRadius = Stage.Radius;
	TargetCenter = Stage.Center;
	bIsShrinking = true;
	bWarningBroadcasted = false;
	StageTimer = 0.f;

	UE_LOG(LogExoRift, Log, TEXT("Zone stage %d: shrinking to %.0f radius over %.0fs"),
		CurrentStage, TargetRadius, Stage.ShrinkDuration);

	OnZoneShrinkStart.Broadcast(CurrentStage);
}

void AExoZoneSystem::TickShrink(float DeltaTime)
{
	if (CurrentStage < 0 || CurrentStage >= Stages.Num()) return;

	const FZoneStage& Stage = Stages[CurrentStage];
	StageTimer += DeltaTime;
	float Alpha = FMath::Clamp(StageTimer / Stage.ShrinkDuration, 0.f, 1.f);

	// Smoothly interpolate radius and center
	float StartRadius = (CurrentStage == 0) ? 40000.f : Stages[CurrentStage - 1].Radius;
	FVector2D StartCenter = (CurrentStage == 0) ? FVector2D::ZeroVector : Stages[CurrentStage - 1].Center;

	CurrentRadius = FMath::Lerp(StartRadius, TargetRadius, Alpha);
	CurrentCenter = FMath::Lerp(StartCenter, TargetCenter, Alpha);

	if (Alpha >= 1.f)
	{
		bIsShrinking = false;
		StageTimer = 0.f;
		bWarningBroadcasted = false;

		OnZoneShrinkEnd.Broadcast(CurrentStage);
	}
}

void AExoZoneSystem::TickHold(float DeltaTime)
{
	if (CurrentStage < 0 || CurrentStage >= Stages.Num()) return;

	const FZoneStage& Stage = Stages[CurrentStage];
	StageTimer += DeltaTime;

	// Fire warning when hold time remaining crosses the threshold
	float Remaining = Stage.HoldDuration - StageTimer;
	if (!bWarningBroadcasted && Remaining <= WarningLeadTime && Remaining > 0.f)
	{
		bWarningBroadcasted = true;
		int32 NextStage = CurrentStage + 1;
		OnZoneWarning.Broadcast(NextStage);
		UE_LOG(LogExoRift, Log, TEXT("Zone warning: Ring %d closing in %.0fs"), NextStage + 1, Remaining);
	}

	if (StageTimer >= Stage.HoldDuration)
	{
		AdvanceStage();
	}
}

bool AExoZoneSystem::IsInsideZone(const FVector& Location) const
{
	FVector2D Loc2D(Location.X, Location.Y);
	float DistSq = FVector2D::DistSquared(Loc2D, CurrentCenter);
	return DistSq <= (CurrentRadius * CurrentRadius);
}

float AExoZoneSystem::GetDamagePerSecond() const
{
	if (CurrentStage < 0 || CurrentStage >= Stages.Num()) return 0.f;
	return Stages[CurrentStage].DamagePerSecond;
}

float AExoZoneSystem::GetHoldTimeRemaining() const
{
	if (bIsShrinking || CurrentStage < 0 || CurrentStage >= Stages.Num()) return 0.f;
	return FMath::Max(Stages[CurrentStage].HoldDuration - StageTimer, 0.f);
}

float AExoZoneSystem::GetShrinkTimeRemaining() const
{
	if (!bIsShrinking || CurrentStage < 0 || CurrentStage >= Stages.Num()) return 0.f;
	return FMath::Max(Stages[CurrentStage].ShrinkDuration - StageTimer, 0.f);
}

void AExoZoneSystem::ApplyZoneDamage(float DeltaTime)
{
	float DPS = GetDamagePerSecond();
	if (DPS <= 0.f) return;

	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive()) continue;
		if (IsInsideZone(Char->GetActorLocation())) continue;

		// Distance-based scaling: further outside = more damage (up to 3x)
		FVector2D Loc2D(Char->GetActorLocation().X, Char->GetActorLocation().Y);
		float DistFromEdge = FVector2D::Distance(Loc2D, CurrentCenter) - CurrentRadius;
		float DistScale = 1.f + FMath::Clamp(DistFromEdge / 5000.f, 0.f, 2.f);

		float FrameDamage = DPS * DistScale * DeltaTime;
		FDamageEvent DamageEvent;
		Char->TakeDamage(FrameDamage, DamageEvent, nullptr, this);
	}
}

void AExoZoneSystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExoZoneSystem, CurrentStage);
	DOREPLIFETIME(AExoZoneSystem, CurrentRadius);
	DOREPLIFETIME(AExoZoneSystem, CurrentCenter);
	DOREPLIFETIME(AExoZoneSystem, TargetRadius);
	DOREPLIFETIME(AExoZoneSystem, TargetCenter);
}
