#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoLevelBuilder.generated.h"

class UStaticMeshComponent;
class UDirectionalLightComponent;

/**
 * Programmatic level generator for the ExoRift BR map.
 * Place one in the level — it spawns terrain, buildings, lighting,
 * spawn points, loot, hazards, and cover on BeginPlay.
 */
UCLASS()
class EXORIFT_API AExoLevelBuilder : public AActor
{
	GENERATED_BODY()

public:
	AExoLevelBuilder();

protected:
	virtual void BeginPlay() override;

private:
	// --- Terrain & environment ---
	void BuildTerrain();
	void BuildLighting();
	void BuildSkybox();

	// --- Structures (in ExoLevelBuilderStructures.cpp) ---
	void BuildStructures();
	void SpawnBuilding(const FVector& Center, const FVector& Size, float Rotation = 0.f);
	void SpawnTower(const FVector& Base, float Radius, float Height);
	void SpawnWall(const FVector& Start, const FVector& End, float Height, float Thickness = 100.f);
	void SpawnPlatform(const FVector& Center, const FVector& Size);
	void SpawnRamp(const FVector& Base, float Length, float Height, float Width, float Yaw);

	// --- Gameplay actors (in ExoLevelBuilderGameplay.cpp) ---
	void PlaceSpawnPoints();
	void PlaceLootSpawners();
	void PlaceHazardZones();
	void PlaceExplodingBarrels();
	void PlaceCoverElements();

	// --- Props & accent lighting (in ExoLevelBuilderProps.cpp) ---
	void BuildProps();
	void SpawnLightPost(const FVector& Base, float Height, const FLinearColor& Color);
	void SpawnAntenna(const FVector& Base, float Height);
	void SpawnPipeRun(const FVector& Start, const FVector& End, float Radius);

	// --- Roads & environment (in ExoLevelBuilderRoads.cpp) ---
	void BuildRoads();
	void SpawnRoadSegment(const FVector& Start, const FVector& End, float Width);
	void SpawnBridge(const FVector& Start, const FVector& End, float Width, float Height);
	void BuildWaterFeatures();
	void BuildFoliage();

	// --- Environmental debris (in ExoLevelBuilderDebris.cpp) ---
	void BuildEnvironmentalDebris();
	void SpawnCrashedShip(const FVector& Center, float Yaw, float Scale);
	void SpawnDebrisField(const FVector& Center, float Radius, int32 Count);
	void SpawnScorchMark(const FVector& Center, float Radius, float Yaw = 0.f);

	// --- Detail & environment (in ExoLevelBuilderDetail.cpp) ---
	void BuildGroundDetail();
	void SpawnFloorPanels(const FVector& Center, float Radius, int32 Count);
	void SpawnEnergyPylon(const FVector& Base, float Height, const FLinearColor& Color);
	void SpawnCrater(const FVector& Center, float Radius);

	// --- Interiors (in ExoLevelBuilderInteriors.cpp) ---
	void BuildInteriors();
	void SpawnConsole(const FVector& Pos, float Yaw);

	// --- Signage & markings (in ExoLevelBuilderSignage.cpp) ---
	void BuildSignage();
	void SpawnCompoundSign(const FVector& Pos, float Yaw, const FLinearColor& Color);
	void SpawnLandingPad(const FVector& Center, float Radius);
	void SpawnDirectionMarker(const FVector& Pos, float Yaw, const FLinearColor& Color);

	// --- Atmosphere (in ExoLevelBuilderAtmosphere.cpp) ---
	void BuildAtmosphere();
	void SpawnHolographicDisplay(const FVector& Pos, float Yaw, float Scale);
	void SpawnSpotlightBeam(const FVector& Base, float Height, const FLinearColor& Color);
	void SpawnEnergyConduit(const FVector& Start, const FVector& End, const FLinearColor& Color);
	void SpawnNeonTube(const FVector& Pos, const FVector& Scale, float Yaw, const FLinearColor& Color);

	// --- Catwalks & vertical gameplay (in ExoLevelBuilderCatwalks.cpp) ---
	void BuildCatwalks();
	void SpawnCatwalk(const FVector& Start, const FVector& End, float Width);
	void SpawnObservationDeck(const FVector& Center, float Radius, float Yaw);
	void SpawnZiplineAnchor(const FVector& Top);

	// --- Underground tunnels (in ExoLevelBuilderTunnels.cpp) ---
	void BuildTunnels();
	void SpawnTunnelEntrance(const FVector& Pos, float Yaw);
	void SpawnTunnelSegment(const FVector& Start, const FVector& End);

	// --- Compound ambient lighting (in ExoLevelBuilderLighting.cpp) ---
	void BuildCompoundLighting();
	void SpawnCompoundGroundMarker(const FVector& Center, const FLinearColor& Color);

	// --- Jump pads & patrol drones (placed in ExoLevelBuilderGameplay.cpp) ---
	void PlaceJumpPads();
	void PlaceDrones();

	// --- Helpers ---
	UStaticMeshComponent* SpawnStaticMesh(const FVector& Location, const FVector& Scale,
		const FRotator& Rotation, UStaticMesh* Mesh, const FLinearColor& Color);

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;

	/** All spawned mesh components for the level geometry. */
	UPROPERTY()
	TArray<UStaticMeshComponent*> LevelMeshes;

	// Map dimensions (centimeters)
	static constexpr float MapHalfSize = 200000.f; // 2km from center
	static constexpr float GroundZ = 0.f;
};
