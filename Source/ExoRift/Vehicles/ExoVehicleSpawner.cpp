#include "Vehicles/ExoVehicleSpawner.h"
#include "Vehicles/ExoHoverVehicle.h"
#include "ExoRift.h"

AExoVehicleSpawner::AExoVehicleSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AExoVehicleSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	const FVector Origin = GetActorLocation();
	int32 Spawned = 0;

	for (int32 i = 0; i < VehicleCount; ++i)
	{
		FVector2D RandOffset = FMath::RandPointInCircle(SpawnRadius);
		FVector CandidateXY = Origin + FVector(RandOffset.X, RandOffset.Y, 0.f);

		FVector GroundPos;
		if (!FindGroundPosition(CandidateXY, GroundPos))
		{
			continue;
		}

		// Raise above ground so hover system can engage
		GroundPos.Z += 160.f;

		FActorSpawnParameters Params;
		AExoHoverVehicle* Vehicle = GetWorld()->SpawnActor<AExoHoverVehicle>(
			AExoHoverVehicle::StaticClass(), GroundPos, FRotator::ZeroRotator, Params);

		if (Vehicle)
		{
			++Spawned;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("VehicleSpawner: placed %d / %d hover vehicles"), Spawned, VehicleCount);
}

bool AExoVehicleSpawner::FindGroundPosition(FVector InXY, FVector& OutPosition) const
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
