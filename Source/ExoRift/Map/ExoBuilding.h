#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoBuilding.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class EBuildingType : uint8
{
	SmallHouse,
	MediumHouse,
	LargeWarehouse,
	Tower,
	Bunker,
	ShopFront,
	Ruins
};

/**
 * Modular building actor that generates multi-story structures
 * from primitive meshes. Buildings provide interior cover,
 * loot spawn locations, and verticality for gameplay.
 */
UCLASS()
class EXORIFT_API AExoBuilding : public AActor
{
	GENERATED_BODY()

public:
	AExoBuilding();

	/** Type of building to generate. */
	UPROPERTY(EditAnywhere, Category = "Building")
	EBuildingType BuildingType = EBuildingType::SmallHouse;

	/** Number of floors. */
	UPROPERTY(EditAnywhere, Category = "Building", meta = (ClampMin = "1", ClampMax = "6"))
	int32 NumFloors = 1;

	/** Building footprint width (X). */
	UPROPERTY(EditAnywhere, Category = "Building")
	float Width = 800.f;

	/** Building footprint depth (Y). */
	UPROPERTY(EditAnywhere, Category = "Building")
	float Depth = 600.f;

	/** Height per floor. */
	UPROPERTY(EditAnywhere, Category = "Building")
	float FloorHeight = 400.f;

	/** Wall thickness. */
	UPROPERTY(EditAnywhere, Category = "Building")
	float WallThickness = 30.f;

	/** Whether to add door openings. */
	UPROPERTY(EditAnywhere, Category = "Building")
	bool bHasDoors = true;

	/** Whether to add window openings. */
	UPROPERTY(EditAnywhere, Category = "Building")
	bool bHasWindows = true;

	/** Build the structure procedurally. */
	void GenerateBuilding();

	/** Get positions suitable for loot spawning inside this building. */
	TArray<FVector> GetLootSpawnPositions() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Building")
	USceneComponent* BuildingRoot;

private:
	/** Create a wall panel as a box mesh component. */
	UStaticMeshComponent* CreateWallPanel(const FVector& Location, const FVector& Scale, const FString& Name);

	/** Create a floor/ceiling slab. */
	UStaticMeshComponent* CreateFloorSlab(const FVector& Location, const FVector& Scale, const FString& Name);

	/** Build walls for a single floor, with optional openings. */
	void BuildFloorWalls(int32 FloorIndex, float BaseZ);

	/** Build the roof. */
	void BuildRoof(float BaseZ);

	/** Build stairs between floors. */
	void BuildStairs(int32 FloorIndex, float BaseZ);

	/** Stored loot positions. */
	TArray<FVector> LootPositions;

	int32 ComponentCounter = 0;
};
