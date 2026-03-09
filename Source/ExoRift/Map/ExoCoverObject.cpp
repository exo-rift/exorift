#include "Map/ExoCoverObject.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "ExoRift.h"

AExoCoverObject::AExoCoverObject()
{
	PrimaryActorTick.bCanEverTick = false;

	CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
	RootComponent = CoverMesh;
	CoverMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CoverMesh->SetCollisionResponseToAllChannels(ECR_Block);

	CoverMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh2"));
	CoverMesh2->SetupAttachment(CoverMesh);
	CoverMesh2->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CoverMesh2->SetCollisionResponseToAllChannels(ECR_Block);
	CoverMesh2->SetVisibility(false);
}

void AExoCoverObject::BeginPlay()
{
	Super::BeginPlay();
}

void AExoCoverObject::GenerateCover()
{
	static UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	static UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	if (!CubeMesh) return;

	switch (CoverType)
	{
	case ECoverType::Crate:         SetupCrate(); break;
	case ECoverType::Jersey_Barrier: SetupBarrier(); break;
	case ECoverType::Wall_Half:     SetupHalfWall(); break;
	case ECoverType::Wall_Full:     SetupFullWall(); break;
	case ECoverType::Rocks:         SetupRocks(); break;
	case ECoverType::Vehicle_Wreck: SetupVehicleWreck(); break;
	case ECoverType::Sandbags:      SetupSandbags(); break;
	}
}

void AExoCoverObject::SetupCrate()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	CoverMesh->SetStaticMesh(Cube);
	CoverMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.0f) * ScaleMultiplier);
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * ScaleMultiplier));
}

void AExoCoverObject::SetupBarrier()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	CoverMesh->SetStaticMesh(Cube);
	CoverMesh->SetRelativeScale3D(FVector(3.0f, 0.6f, 0.8f) * ScaleMultiplier);
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 40.f * ScaleMultiplier));
}

void AExoCoverObject::SetupHalfWall()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	CoverMesh->SetStaticMesh(Cube);
	CoverMesh->SetRelativeScale3D(FVector(4.0f, 0.3f, 1.2f) * ScaleMultiplier);
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 60.f * ScaleMultiplier));
}

void AExoCoverObject::SetupFullWall()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	CoverMesh->SetStaticMesh(Cube);
	CoverMesh->SetRelativeScale3D(FVector(5.0f, 0.3f, 2.5f) * ScaleMultiplier);
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 125.f * ScaleMultiplier));
}

void AExoCoverObject::SetupRocks()
{
	static UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	CoverMesh->SetStaticMesh(Sphere);
	float RandScale = FMath::RandRange(0.8f, 2.0f) * ScaleMultiplier;
	CoverMesh->SetRelativeScale3D(FVector(RandScale, RandScale * 0.8f, RandScale * 0.6f));
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 30.f * RandScale));

	// Second rock nearby
	if (Sphere)
	{
		CoverMesh2->SetStaticMesh(Sphere);
		CoverMesh2->SetVisibility(true);
		float S2 = FMath::RandRange(0.5f, 1.2f) * ScaleMultiplier;
		CoverMesh2->SetRelativeScale3D(FVector(S2, S2 * 0.9f, S2 * 0.5f));
		CoverMesh2->SetRelativeLocation(FVector(80.f * ScaleMultiplier, 60.f * ScaleMultiplier, 20.f * S2));
	}
}

void AExoCoverObject::SetupVehicleWreck()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	CoverMesh->SetStaticMesh(Cube);
	// Main body
	CoverMesh->SetRelativeScale3D(FVector(4.0f, 2.0f, 1.0f) * ScaleMultiplier);
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * ScaleMultiplier));
	// Slight tilt for wreck feel
	CoverMesh->SetRelativeRotation(FRotator(FMath::RandRange(-5.f, 5.f), 0.f, FMath::RandRange(-8.f, 8.f)));

	// Cabin
	CoverMesh2->SetStaticMesh(Cube);
	CoverMesh2->SetVisibility(true);
	CoverMesh2->SetRelativeScale3D(FVector(1.5f, 1.8f, 0.8f) * ScaleMultiplier);
	CoverMesh2->SetRelativeLocation(FVector(-80.f * ScaleMultiplier, 0.f, 130.f * ScaleMultiplier));
}

void AExoCoverObject::SetupSandbags()
{
	static UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	CoverMesh->SetStaticMesh(Cube);
	// Low, wide barrier
	CoverMesh->SetRelativeScale3D(FVector(3.5f, 0.8f, 0.6f) * ScaleMultiplier);
	CoverMesh->SetRelativeLocation(FVector(0.f, 0.f, 30.f * ScaleMultiplier));
}
