#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoPOI.h"
#include "ExoWorldBuilder.generated.h"

class AExoProceduralTerrain;
class AExoLevelLighting;
class AExoWaterPlane;
class AExoPOI;
class AExoCoverObject;
class AExoSpawnPoint;

/**
 * POI definition used by the WorldBuilder to lay out the map.
 */
USTRUCT()
struct FPOIDefinition
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	EPOIType Type = EPOIType::Outpost;

	UPROPERTY()
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY()
	float Radius = 5000.f;

	UPROPERTY()
	int32 BuildingCount = 5;

	UPROPERTY()
	float LootDensity = 1.f;
};

/**
 * Master orchestrator that builds the entire BR level.
 * Spawns terrain, lighting, water, POIs, cover, and spawn points
 * to create a complete playable 3D world.
 */
UCLASS()
class EXORIFT_API AExoWorldBuilder : public AActor
{
	GENERATED_BODY()

public:
	AExoWorldBuilder();

	/** Build the entire world. Called from ExoWorldSetup. */
	void BuildWorld();

	/** Whether the world has been built. */
	bool IsWorldBuilt() const { return bWorldBuilt; }

protected:
	virtual void BeginPlay() override;

private:
	/** Step 1: Spawn terrain. */
	void BuildTerrain();

	/** Step 2: Set up lighting and atmosphere. */
	void BuildLighting();

	/** Step 3: Create water plane. */
	void BuildWater();

	/** Step 4: Spawn all POIs. */
	void BuildPOIs();

	/** Step 5: Scatter cover objects in open areas. */
	void BuildOpenWorldCover();

	/** Step 6: Place spawn points. */
	void BuildSpawnPoints();

	/** Step 7: Place hazard zones. */
	void BuildHazardZones();

	/** Define the POI layout for the map. */
	TArray<FPOIDefinition> CreateMapLayout() const;

	/** Spawn a POI at a given location. */
	AExoPOI* SpawnPOI(const FPOIDefinition& Def);

	/** Get ground height at a world position. */
	float GetGroundZ(const FVector2D& WorldXY) const;

	UPROPERTY()
	AExoProceduralTerrain* Terrain;

	UPROPERTY()
	AExoLevelLighting* Lighting;

	UPROPERTY()
	AExoWaterPlane* Water;

	UPROPERTY()
	TArray<AExoPOI*> POIs;

	bool bWorldBuilt = false;
};
