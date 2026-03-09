// ExoLevelBuilder.cpp — Terrain, lighting, skybox, and mesh helper
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "ExoRift.h"

AExoLevelBuilder::AExoLevelBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Cache engine meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphFind.Succeeded()) SphereMesh = SphFind.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded()) BaseMaterial = MatFind.Object;
}

void AExoLevelBuilder::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Generating level..."));

	BuildTerrain();
	BuildLighting();
	BuildSkybox();
	BuildStructures();
	PlaceSpawnPoints();
	PlaceLootSpawners();
	PlaceHazardZones();
	PlaceExplodingBarrels();
	PlaceCoverElements();
	BuildProps();

	UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Level complete — %d mesh components, ready."),
		LevelMeshes.Num());
}

void AExoLevelBuilder::BuildTerrain()
{
	// Large ground plane
	float GroundHalf = MapHalfSize * 1.5f; // Extend past zone
	float GroundScale = GroundHalf / 50.f;  // Cube is 100 units
	SpawnStaticMesh(
		FVector(0.f, 0.f, -50.f), // Slightly below zero
		FVector(GroundScale, GroundScale, 1.f),
		FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.04f, 0.045f, 0.05f)); // Dark sci-fi floor

	// Terrain variation — raised platforms across the map
	struct FTerrainPatch { FVector Pos; float SizeX; float SizeY; float Height; };
	TArray<FTerrainPatch> Patches = {
		{{-80000.f, -60000.f, 0.f}, 40000.f, 30000.f, 200.f},
		{{60000.f, 80000.f, 0.f}, 35000.f, 25000.f, 150.f},
		{{-40000.f, 100000.f, 0.f}, 50000.f, 20000.f, 300.f},
		{{100000.f, -50000.f, 0.f}, 30000.f, 40000.f, 250.f},
		{{-120000.f, 30000.f, 0.f}, 25000.f, 35000.f, 180.f},
	};

	for (const auto& P : Patches)
	{
		SpawnStaticMesh(
			FVector(P.Pos.X, P.Pos.Y, P.Height * 0.5f),
			FVector(P.SizeX / 100.f, P.SizeY / 100.f, P.Height / 100.f),
			FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.05f, 0.055f, 0.06f));
	}
}

void AExoLevelBuilder::BuildLighting()
{
	// Directional light (sun) — create as component on this actor
	UDirectionalLightComponent* Sun = NewObject<UDirectionalLightComponent>(this);
	Sun->SetupAttachment(RootComponent);
	Sun->RegisterComponent();
	Sun->SetWorldRotation(FRotator(-45.f, 30.f, 0.f));
	Sun->SetIntensity(3.f);
	Sun->SetLightColor(FLinearColor(0.9f, 0.85f, 0.7f));
	Sun->CastShadows = true;

	// Sky light for ambient fill
	USkyLightComponent* Sky = NewObject<USkyLightComponent>(this);
	Sky->SetupAttachment(RootComponent);
	Sky->RegisterComponent();
	Sky->SetIntensity(1.5f);
	Sky->SetLightColor(FLinearColor(0.4f, 0.5f, 0.7f));

	// Atmospheric fog
	UExponentialHeightFogComponent* Fog = NewObject<UExponentialHeightFogComponent>(this);
	Fog->SetupAttachment(RootComponent);
	Fog->RegisterComponent();
	Fog->SetFogDensity(0.001f);
	Fog->SetFogHeightFalloff(0.2f);
	Fog->SetFogInscatteringColor(FLinearColor(0.15f, 0.2f, 0.3f));
}

void AExoLevelBuilder::BuildSkybox()
{
	// Large inverted sphere as skybox
	if (!SphereMesh) return;

	UStaticMeshComponent* SkySphere = NewObject<UStaticMeshComponent>(this);
	SkySphere->SetupAttachment(RootComponent);
	SkySphere->SetStaticMesh(SphereMesh);
	SkySphere->SetWorldLocation(FVector(0.f, 0.f, 0.f));
	SkySphere->SetWorldScale3D(FVector(-5000.f, -5000.f, -5000.f)); // Inverted normals
	SkySphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkySphere->CastShadow = false;
	SkySphere->RegisterComponent();

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* SkyMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		SkyMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.02f, 0.03f, 0.06f));
		SkySphere->SetMaterial(0, SkyMat);
	}
}

UStaticMeshComponent* AExoLevelBuilder::SpawnStaticMesh(const FVector& Location,
	const FVector& Scale, const FRotator& Rotation, UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	Comp->SetupAttachment(RootComponent);
	Comp->SetStaticMesh(Mesh);
	Comp->SetWorldLocation(Location);
	Comp->SetWorldScale3D(Scale);
	Comp->SetWorldRotation(Rotation);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->CastShadow = true;
	Comp->RegisterComponent();

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		DynMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Comp->SetMaterial(0, DynMat);
	}

	LevelMeshes.Add(Comp);
	return Comp;
}
