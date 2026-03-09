#include "Map/ExoZoneVisualizer.h"
#include "Map/ExoZoneSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoZoneVisualizer::AExoZoneVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;

	ZoneWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneWallMesh"));
	RootComponent = ZoneWallMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		ZoneWallMesh->SetStaticMesh(CylinderFinder.Object);
	}

	ZoneWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneWallMesh->CastShadow = false;
}

void AExoZoneVisualizer::BeginPlay()
{
	Super::BeginPlay();

	// Create emissive translucent zone wall material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded() && ZoneWallMesh)
	{
		ZoneMaterial = UMaterialInstanceDynamic::Create(MatFind.Object, this);
		ZoneMaterial->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(ZoneColor.R, ZoneColor.G, ZoneColor.B, ZoneColor.A));
		ZoneMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ZoneColor.R * 2.f, ZoneColor.G * 2.f, ZoneColor.B * 2.f));
		ZoneWallMesh->SetMaterial(0, ZoneMaterial);
	}
}

void AExoZoneVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ZoneSystem)
	{
		for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
		{
			ZoneSystem = *It;
			break;
		}
		if (!ZoneSystem) return;
	}

	UpdateZoneWall();
}

void AExoZoneVisualizer::UpdateZoneWall()
{
	if (!ZoneSystem) return;

	float Radius = ZoneSystem->GetCurrentRadius();
	FVector2D Center = ZoneSystem->GetCurrentCenter();

	if (FMath::Abs(Radius - CachedRadius) < 10.f) return;
	CachedRadius = Radius;

	float RadiusScale = Radius / 50.f;
	float HeightScale = WallHeight / 100.f;

	SetActorLocation(FVector(Center.X, Center.Y, WallHeight * 0.5f));
	ZoneWallMesh->SetWorldScale3D(FVector(RadiusScale, RadiusScale, HeightScale));

	// Update material emissive pulse
	if (ZoneMaterial)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Pulse = 1.f + 0.5f * FMath::Abs(FMath::Sin(Time * 0.8f));
		ZoneMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ZoneColor.R * Pulse, ZoneColor.G * Pulse, ZoneColor.B * Pulse));
	}
}

void AExoZoneVisualizer::GenerateCircleMesh(float Radius, FVector2D Center, float InWallHeight,
	TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UVs)
{
	float AngleStep = 2.f * PI / CircleSegments;

	for (int32 i = 0; i <= CircleSegments; i++)
	{
		float Angle = i * AngleStep;
		float X = Center.X + FMath::Cos(Angle) * Radius;
		float Y = Center.Y + FMath::Sin(Angle) * Radius;

		Vertices.Add(FVector(X, Y, 0.f));
		UVs.Add(FVector2D(static_cast<float>(i) / CircleSegments, 0.f));

		Vertices.Add(FVector(X, Y, InWallHeight));
		UVs.Add(FVector2D(static_cast<float>(i) / CircleSegments, 1.f));

		if (i < CircleSegments)
		{
			int32 Base = i * 2;
			Triangles.Add(Base);
			Triangles.Add(Base + 1);
			Triangles.Add(Base + 2);
			Triangles.Add(Base + 1);
			Triangles.Add(Base + 3);
			Triangles.Add(Base + 2);
		}
	}
}
