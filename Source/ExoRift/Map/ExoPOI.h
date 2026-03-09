#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoPOI.generated.h"

class AExoBuilding;

UENUM(BlueprintType)
enum class EPOIType : uint8
{
	MilitaryBase,
	TownCenter,
	IndustrialZone,
	ResearchFacility,
	Outpost,
	CrashSite,
	PowerPlant,
	Marketplace
};

/**
 * Defines a named Point of Interest on the BR map.
 * Each POI spawns a cluster of buildings, cover, loot containers,
 * and environmental props to create a distinct gameplay area.
 */
UCLASS()
class EXORIFT_API AExoPOI : public AActor
{
	GENERATED_BODY()

public:
	AExoPOI();

	/** Display name for this POI. */
	UPROPERTY(EditAnywhere, Category = "POI")
	FString POIName = TEXT("Unnamed Location");

	/** Type affects building density and loot quality. */
	UPROPERTY(EditAnywhere, Category = "POI")
	EPOIType POIType = EPOIType::Outpost;

	/** Radius of the POI area. */
	UPROPERTY(EditAnywhere, Category = "POI")
	float Radius = 5000.f;

	/** Number of buildings to generate in this POI. */
	UPROPERTY(EditAnywhere, Category = "POI")
	int32 BuildingCount = 5;

	/** Loot density multiplier (1.0 = normal). */
	UPROPERTY(EditAnywhere, Category = "POI")
	float LootDensity = 1.f;

	/** Whether this POI has been populated. */
	bool IsPopulated() const { return bPopulated; }

	/** Populate the POI with buildings and props. */
	void PopulatePOI(class AExoProceduralTerrain* Terrain);

	/** Get world-space center. */
	FVector GetPOICenter() const { return GetActorLocation(); }

protected:
	virtual void BeginPlay() override;

private:
	/** Place buildings within the POI radius. */
	void SpawnBuildings(class AExoProceduralTerrain* Terrain);

	/** Place cover objects between buildings. */
	void SpawnCoverObjects(class AExoProceduralTerrain* Terrain);

	/** Place exploding barrels and hazard props. */
	void SpawnEnvironmentalProps(class AExoProceduralTerrain* Terrain);

	/** Get ground height at position, using terrain if available. */
	float GetGroundHeight(const FVector2D& WorldXY, class AExoProceduralTerrain* Terrain) const;

	bool bPopulated = false;

	UPROPERTY()
	TArray<AExoBuilding*> Buildings;
};
