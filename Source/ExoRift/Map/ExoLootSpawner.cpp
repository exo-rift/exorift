#include "Map/ExoLootSpawner.h"
#include "Weapons/ExoWeaponPickup.h"
#include "ExoRift.h"

AExoLootSpawner::AExoLootSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AExoLootSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	const FVector Origin = GetActorLocation();
	int32 Spawned = 0;

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		// Random XY position within radius
		FVector2D RandOffset = FMath::RandPointInCircle(SpawnRadius);
		FVector CandidateXY = Origin + FVector(RandOffset.X, RandOffset.Y, 0.f);

		FVector GroundPos;
		if (!FindGroundPosition(CandidateXY, GroundPos))
		{
			continue; // No ground here, skip this spawn
		}

		// Raise slightly above ground to avoid clipping
		GroundPos.Z += 30.f;

		EWeaponType Type = PickRandomWeaponType();
		EWeaponRarity ChosenRarity = PickWeightedRarity();

		FActorSpawnParameters Params;
		AExoWeaponPickup* Pickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
			AExoWeaponPickup::StaticClass(), GroundPos, FRotator::ZeroRotator, Params);

		if (Pickup)
		{
			Pickup->WeaponType = Type;
			Pickup->Rarity = ChosenRarity;
			Pickup->bRespawns = false;
			++Spawned;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("LootSpawner: scattered %d / %d pickups"), Spawned, SpawnCount);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

EWeaponRarity AExoLootSpawner::PickWeightedRarity() const
{
	float TotalWeight = 0.f;
	for (float W : RarityWeights)
	{
		TotalWeight += W;
	}

	if (TotalWeight <= 0.f) return EWeaponRarity::Common;

	float Roll = FMath::FRand() * TotalWeight;
	float Cumulative = 0.f;

	for (int32 i = 0; i < RarityWeights.Num(); ++i)
	{
		Cumulative += RarityWeights[i];
		if (Roll <= Cumulative)
		{
			return static_cast<EWeaponRarity>(FMath::Clamp(i, 0, static_cast<int32>(EWeaponRarity::Legendary)));
		}
	}

	return EWeaponRarity::Common;
}

EWeaponType AExoLootSpawner::PickRandomWeaponType()
{
	int32 Roll = FMath::RandRange(0, 5);
	switch (Roll)
	{
	case 0: return EWeaponType::Rifle;
	case 1: return EWeaponType::Pistol;
	case 2: return EWeaponType::Shotgun;
	case 3: return EWeaponType::SMG;
	case 4: return EWeaponType::Sniper;
	case 5: return EWeaponType::GrenadeLauncher;
	default: return EWeaponType::Rifle;
	}
}

bool AExoLootSpawner::FindGroundPosition(FVector InXY, FVector& OutPosition) const
{
	const float TraceStartAltitude = 10000.f;
	FVector TraceStart = FVector(InXY.X, InXY.Y, GetActorLocation().Z + TraceStartAltitude);
	FVector TraceEnd = FVector(InXY.X, InXY.Y, GetActorLocation().Z - TraceStartAltitude);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
	if (bHit)
	{
		OutPosition = Hit.ImpactPoint;
		return true;
	}

	return false;
}
