#include "Map/ExoZoneVisualizer.h"
#include "Map/ExoZoneSystem.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoZoneVisualizer::AExoZoneVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Use a cylinder mesh scaled to represent the zone boundary
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

void AExoZoneVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Find zone system once
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

	// Only update when radius changes significantly
	if (FMath::Abs(Radius - CachedRadius) < 10.f) return;
	CachedRadius = Radius;

	// Scale the cylinder to match zone radius
	// UE cylinder mesh is 100 units radius, 100 units height by default
	float RadiusScale = Radius / 50.f; // SM_Cylinder has 50 unit radius
	float HeightScale = WallHeight / 100.f;

	SetActorLocation(FVector(Center.X, Center.Y, WallHeight * 0.5f));
	ZoneWallMesh->SetWorldScale3D(FVector(RadiusScale, RadiusScale, HeightScale));

	// Update material opacity scroll
	if (ZoneMaterial)
	{
		float Time = GetWorld()->GetTimeSeconds();
		ZoneMaterial->SetScalarParameterValue(TEXT("ScrollSpeed"), Time * 0.5f);
	}
}

void AExoZoneVisualizer::GenerateCircleMesh(float Radius, FVector2D Center, float InWallHeight,
	TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UVs)
{
	// Generate a vertical circle wall mesh
	float AngleStep = 2.f * PI / CircleSegments;

	for (int32 i = 0; i <= CircleSegments; i++)
	{
		float Angle = i * AngleStep;
		float X = Center.X + FMath::Cos(Angle) * Radius;
		float Y = Center.Y + FMath::Sin(Angle) * Radius;

		// Bottom vertex
		Vertices.Add(FVector(X, Y, 0.f));
		UVs.Add(FVector2D(static_cast<float>(i) / CircleSegments, 0.f));

		// Top vertex
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
