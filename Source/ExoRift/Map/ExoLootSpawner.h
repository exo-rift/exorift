#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoLootSpawner.generated.h"

class AExoWeaponPickup;

/**
 * Level-placed actor that scatters random weapon pickups across the map.
 * Used in BR to create one-time loot spawns at match start.
 */
UCLASS()
class EXORIFT_API AExoLootSpawner : public AActor
{
	GENERATED_BODY()

public:
	AExoLootSpawner();

	/** Number of weapon pickups to scatter. */
	UPROPERTY(EditAnywhere, Category = "Loot")
	int32 SpawnCount = 30;

	/** Radius from this actor's location to scatter pickups within. */
	UPROPERTY(EditAnywhere, Category = "Loot")
	float SpawnRadius = 150000.f;

	/** Rarity weight distribution: Common, Rare, Epic, Legendary. */
	UPROPERTY(EditAnywhere, Category = "Loot")
	TArray<float> RarityWeights = { 0.5f, 0.3f, 0.15f, 0.05f };

protected:
	virtual void BeginPlay() override;

private:
	/** Pick a weighted random rarity using RarityWeights. */
	EWeaponRarity PickWeightedRarity() const;

	/** Pick a random weapon type. */
	static EWeaponType PickRandomWeaponType();

	/** Trace down from altitude to find ground position. Returns false if no ground found. */
	bool FindGroundPosition(FVector InXY, FVector& OutPosition) const;
};
