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

	// --- Roads (in ExoLevelBuilderRoads.cpp) ---
	void BuildRoads();
	void SpawnRoadSegment(const FVector& Start, const FVector& End, float Width);
	void SpawnBridge(const FVector& Start, const FVector& End, float Width, float Height);

	// --- Water & foliage (in ExoLevelBuilderNature.cpp) ---
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
	void SpawnGroundMist(const FVector& Center, float Radius, const FLinearColor& Color);
	void SpawnGroundClutter(const FVector& Center, float Radius, int32 Count);

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

	// --- Open field terrain & cover (in ExoLevelBuilderFieldCover.cpp) ---
	void BuildFieldCover();
	void SpawnRockFormation(const FVector& Center, float Scale, float Yaw);
	void SpawnRuinedStructure(const FVector& Center, float Scale, float Yaw);

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

	// --- Jump pads, drones, steam vents (in ExoLevelBuilderMechanics.cpp) ---
	void PlaceJumpPads();
	void PlaceDrones();
	void PlaceSteamVents();

	// --- Environmental storytelling (in ExoLevelBuilderStorytelling.cpp) ---
	void BuildStorytelling();

	// --- POIs / landmarks (in ExoLevelBuilderPOIs.cpp) ---
	void PlacePOIs();

	// --- Helpers ---
	UStaticMeshComponent* SpawnStaticMesh(const FVector& Location, const FVector& Scale,
		const FRotator& Rotation, UStaticMesh* Mesh, const FLinearColor& Color);

	/** Spawn a mesh keeping its imported materials (for KayKit/marketplace assets). */
	UStaticMeshComponent* SpawnRawMesh(const FVector& Location, const FVector& Scale,
		const FRotator& Rotation, UStaticMesh* Mesh);

	/** Scans known Fab/Megascans/Paragon paths and auto-binds found assets. */
	void ScanMarketplaceAssets();

	// Engine primitive meshes (fallbacks when marketplace assets unavailable)
	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
	UPROPERTY()
	UStaticMesh* CylinderMesh = nullptr;
	UPROPERTY()
	UStaticMesh* SphereMesh = nullptr;
	// Marketplace asset soft references — swap in Megascans/City Sample when available.
	// These are TSoftObjectPtr so they don't fail if the assets aren't installed.
	UPROPERTY(EditDefaultsOnly, Category = "MarketplaceAssets")
	TSoftObjectPtr<UMaterialInterface> GroundMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "MarketplaceAssets")
	TSoftObjectPtr<UMaterialInterface> ConcreteWallMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "MarketplaceAssets")
	TSoftObjectPtr<UMaterialInterface> MetalPanelMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "MarketplaceAssets")
	TSoftObjectPtr<UStaticMesh> RockMesh;
	UPROPERTY(EditDefaultsOnly, Category = "MarketplaceAssets")
	TSoftObjectPtr<UStaticMesh> CrateMesh;
	UPROPERTY(EditDefaultsOnly, Category = "MarketplaceAssets")
	TSoftObjectPtr<UStaticMesh> BarrelMesh;

	// --- KayKit Space Base meshes (auto-detected by ScanMarketplaceAssets) ---
	UPROPERTY() UStaticMesh* KK_BaseModule = nullptr;
	UPROPERTY() UStaticMesh* KK_BaseGarage = nullptr;
	UPROPERTY() UStaticMesh* KK_CargoDepot = nullptr;
	UPROPERTY() UStaticMesh* KK_Cargo = nullptr;
	UPROPERTY() UStaticMesh* KK_Container = nullptr;
	UPROPERTY() UStaticMesh* KK_StructureTall = nullptr;
	UPROPERTY() UStaticMesh* KK_StructureLow = nullptr;
	UPROPERTY() UStaticMesh* KK_RoofModule = nullptr;
	UPROPERTY() UStaticMesh* KK_SolarPanel = nullptr;
	UPROPERTY() UStaticMesh* KK_DrillStructure = nullptr;
	UPROPERTY() UStaticMesh* KK_LandingPadLarge = nullptr;
	UPROPERTY() UStaticMesh* KK_Tunnel = nullptr;
	UPROPERTY() UStaticMesh* KK_Rock = nullptr;
	UPROPERTY() UStaticMesh* KK_TerrainLow = nullptr;
	UPROPERTY() UStaticMesh* KK_TerrainTall = nullptr;
	UPROPERTY() UStaticMesh* KK_WindTurbine = nullptr;
	UPROPERTY() UStaticMesh* KK_Lander = nullptr;
	UPROPERTY() UStaticMesh* KK_SpaceTruck = nullptr;
	UPROPERTY() UStaticMesh* KK_Lights = nullptr;
	/** True if KayKit assets were found and loaded. */
	bool bHasKayKitAssets = false;

	// --- Kenney Space Kit meshes (auto-detected by ScanMarketplaceAssets) ---
	UPROPERTY() UStaticMesh* KN_Corridor = nullptr;
	UPROPERTY() UStaticMesh* KN_CorridorCorner = nullptr;
	UPROPERTY() UStaticMesh* KN_CorridorWide = nullptr;
	UPROPERTY() UStaticMesh* KN_CorridorWideCorner = nullptr;
	UPROPERTY() UStaticMesh* KN_CorridorIntersection = nullptr;
	UPROPERTY() UStaticMesh* KN_RoomSmall = nullptr;
	UPROPERTY() UStaticMesh* KN_RoomLarge = nullptr;
	UPROPERTY() UStaticMesh* KN_RoomWide = nullptr;
	UPROPERTY() UStaticMesh* KN_Gate = nullptr;
	UPROPERTY() UStaticMesh* KN_GateDoor = nullptr;
	UPROPERTY() UStaticMesh* KN_GateLasers = nullptr;
	UPROPERTY() UStaticMesh* KN_Stairs = nullptr;
	UPROPERTY() UStaticMesh* KN_StairsWide = nullptr;
	UPROPERTY() UStaticMesh* KN_Door = nullptr;
	UPROPERTY() UStaticMesh* KN_Cables = nullptr;
	bool bHasKenneyAssets = false;

	// --- Quaternius Sci-Fi meshes (auto-detected by ScanMarketplaceAssets) ---
	UPROPERTY() UStaticMesh* QT_Column1 = nullptr;
	UPROPERTY() UStaticMesh* QT_Column2 = nullptr;
	UPROPERTY() UStaticMesh* QT_ColumnSlim = nullptr;
	UPROPERTY() UStaticMesh* QT_DoorSingle = nullptr;
	UPROPERTY() UStaticMesh* QT_DoorDoubleL = nullptr;
	UPROPERTY() UStaticMesh* QT_FloorBasic = nullptr;
	UPROPERTY() UStaticMesh* QT_FloorCorner = nullptr;
	UPROPERTY() UStaticMesh* QT_FloorSide = nullptr;
	UPROPERTY() UStaticMesh* QT_Wall1 = nullptr;
	UPROPERTY() UStaticMesh* QT_Wall3 = nullptr;
	UPROPERTY() UStaticMesh* QT_WindowWall = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsCrate = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsComputer = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsShelf = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsContainerFull = nullptr;
	UPROPERTY() UStaticMesh* QT_RoofPipes = nullptr;
	UPROPERTY() UStaticMesh* QT_Staircase = nullptr;
	UPROPERTY() UStaticMesh* QT_DetailVent1 = nullptr;
	UPROPERTY() UStaticMesh* QT_DetailPipesLong = nullptr;
	bool bHasQuaterniusAssets = false;

	// --- SciFi Door mesh (auto-detected by ScanMarketplaceAssets) ---
	UPROPERTY() UStaticMesh* SF_Door = nullptr;
	bool bHasSciFiDoorAsset = false;

	/** All spawned mesh components for the level geometry. */
	UPROPERTY()
	TArray<UStaticMeshComponent*> LevelMeshes;

	// Map dimensions (centimeters)
	static constexpr float MapHalfSize = 200000.f; // 2km from center
	static constexpr float GroundZ = 0.f;
};
