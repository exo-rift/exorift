#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoVehicleSpawner.generated.h"

class AExoHoverVehicle;

/**
 * Level-placed actor that scatters hover vehicles across the map.
 * Similar to ExoLootSpawner — traces down to find valid ground positions.
 */
UCLASS()
class EXORIFT_API AExoVehicleSpawner : public AActor
{
	GENERATED_BODY()

public:
	AExoVehicleSpawner();

	UPROPERTY(EditAnywhere, Category = "Vehicles")
	int32 VehicleCount = 5;

	UPROPERTY(EditAnywhere, Category = "Vehicles")
	float SpawnRadius = 100000.f;

protected:
	virtual void BeginPlay() override;

private:
	bool FindGroundPosition(FVector InXY, FVector& OutPosition) const;
};
