#include "Map/ExoBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "ExoRift.h"

AExoBuilding::AExoBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
	BuildingRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BuildingRoot"));
	RootComponent = BuildingRoot;
}

void AExoBuilding::BeginPlay()
{
	Super::BeginPlay();
}

void AExoBuilding::GenerateBuilding()
{
	// Set parameters by building type
	switch (BuildingType)
	{
	case EBuildingType::SmallHouse:
		Width = 800.f; Depth = 600.f; NumFloors = 1; break;
	case EBuildingType::MediumHouse:
		Width = 1200.f; Depth = 800.f; NumFloors = 2; break;
	case EBuildingType::LargeWarehouse:
		Width = 2000.f; Depth = 1500.f; NumFloors = 1; FloorHeight = 600.f; break;
	case EBuildingType::Tower:
		Width = 600.f; Depth = 600.f; NumFloors = 4; break;
	case EBuildingType::Bunker:
		Width = 1000.f; Depth = 800.f; NumFloors = 1; FloorHeight = 300.f; break;
	case EBuildingType::ShopFront:
		Width = 1000.f; Depth = 600.f; NumFloors = 1; break;
	case EBuildingType::Ruins:
		Width = 1200.f; Depth = 1000.f; NumFloors = 1; break;
	}

	// Ground floor slab
	CreateFloorSlab(
		FVector(0.f, 0.f, 0.f),
		FVector(Width / 100.f, Depth / 100.f, WallThickness / 100.f),
		TEXT("GroundFloor"));

	// Build each floor
	for (int32 Floor = 0; Floor < NumFloors; ++Floor)
	{
		float BaseZ = Floor * FloorHeight;
		BuildFloorWalls(Floor, BaseZ);

		// Floor slab for upper floors
		if (Floor > 0)
		{
			CreateFloorSlab(
				FVector(0.f, 0.f, BaseZ),
				FVector(Width / 100.f, Depth / 100.f, WallThickness / 100.f),
				FString::Printf(TEXT("Floor_%d"), Floor));
		}

		// Stairs between floors
		if (Floor < NumFloors - 1)
		{
			BuildStairs(Floor, BaseZ);
		}

		// Loot positions per floor (corners + center)
		FVector FloorCenter = GetActorLocation() + FVector(0.f, 0.f, BaseZ + 50.f);
		LootPositions.Add(FloorCenter);
		LootPositions.Add(FloorCenter + FVector(Width * 0.3f, Depth * 0.3f, 0.f));
		LootPositions.Add(FloorCenter + FVector(-Width * 0.3f, -Depth * 0.3f, 0.f));
	}

	// Roof
	BuildRoof(NumFloors * FloorHeight);

	UE_LOG(LogExoRift, Verbose, TEXT("Building generated: %s (%dx%d, %d floors)"),
		*UEnum::GetValueAsString(BuildingType), (int32)Width, (int32)Depth, NumFloors);
}

void AExoBuilding::BuildFloorWalls(int32 FloorIndex, float BaseZ)
{
	float HW = Width * 0.5f;
	float HD = Depth * 0.5f;
	float WH = FloorHeight;
	float WT = WallThickness;

	bool bIsRuins = (BuildingType == EBuildingType::Ruins);
	FString Prefix = FString::Printf(TEXT("F%d_"), FloorIndex);

	// Front wall (positive Y) — with door opening on ground floor
	if (!bIsRuins || FMath::RandBool())
	{
		if (bHasDoors && FloorIndex == 0)
		{
			float DoorWidth = 150.f;
			float DoorHeight = 250.f;

			// Left section
			float LeftWidth = HW - DoorWidth * 0.5f;
			CreateWallPanel(
				FVector(-HW + LeftWidth * 0.5f, HD - WT * 0.5f, BaseZ + WH * 0.5f),
				FVector(LeftWidth / 100.f, WT / 100.f, WH / 100.f),
				Prefix + TEXT("FrontL"));

			// Right section
			CreateWallPanel(
				FVector(HW - LeftWidth * 0.5f, HD - WT * 0.5f, BaseZ + WH * 0.5f),
				FVector(LeftWidth / 100.f, WT / 100.f, WH / 100.f),
				Prefix + TEXT("FrontR"));

			// Above door
			float AboveDoorH = WH - DoorHeight;
			if (AboveDoorH > 10.f)
			{
				CreateWallPanel(
					FVector(0.f, HD - WT * 0.5f, BaseZ + DoorHeight + AboveDoorH * 0.5f),
					FVector(DoorWidth / 100.f, WT / 100.f, AboveDoorH / 100.f),
					Prefix + TEXT("FrontAbove"));
			}
		}
		else
		{
			CreateWallPanel(
				FVector(0.f, HD - WT * 0.5f, BaseZ + WH * 0.5f),
				FVector(Width / 100.f, WT / 100.f, WH / 100.f),
				Prefix + TEXT("Front"));
		}
	}

	// Back wall (negative Y)
	if (!bIsRuins || FMath::RandBool())
	{
		CreateWallPanel(
			FVector(0.f, -HD + WT * 0.5f, BaseZ + WH * 0.5f),
			FVector(Width / 100.f, WT / 100.f, WH / 100.f),
			Prefix + TEXT("Back"));
	}

	// Left wall (negative X) — with windows
	if (!bIsRuins || FMath::RandBool())
	{
		if (bHasWindows && Depth > 400.f)
		{
			float WinBottom = 150.f;
			float WinHeight = 120.f;

			// Below window
			CreateWallPanel(
				FVector(-HW + WT * 0.5f, 0.f, BaseZ + WinBottom * 0.5f),
				FVector(WT / 100.f, Depth / 100.f, WinBottom / 100.f),
				Prefix + TEXT("LeftBelow"));

			// Above window
			float AboveZ = WinBottom + WinHeight;
			float AboveH = WH - AboveZ;
			if (AboveH > 10.f)
			{
				CreateWallPanel(
					FVector(-HW + WT * 0.5f, 0.f, BaseZ + AboveZ + AboveH * 0.5f),
					FVector(WT / 100.f, Depth / 100.f, AboveH / 100.f),
					Prefix + TEXT("LeftAbove"));
			}
		}
		else
		{
			CreateWallPanel(
				FVector(-HW + WT * 0.5f, 0.f, BaseZ + WH * 0.5f),
				FVector(WT / 100.f, Depth / 100.f, WH / 100.f),
				Prefix + TEXT("Left"));
		}
	}

	// Right wall (positive X)
	if (!bIsRuins || FMath::RandBool())
	{
		CreateWallPanel(
			FVector(HW - WT * 0.5f, 0.f, BaseZ + WH * 0.5f),
			FVector(WT / 100.f, Depth / 100.f, WH / 100.f),
			Prefix + TEXT("Right"));
	}
}

void AExoBuilding::BuildRoof(float BaseZ)
{
	CreateFloorSlab(
		FVector(0.f, 0.f, BaseZ),
		FVector((Width + WallThickness * 2.f) / 100.f, (Depth + WallThickness * 2.f) / 100.f, WallThickness / 100.f),
		TEXT("Roof"));
}

void AExoBuilding::BuildStairs(int32 FloorIndex, float BaseZ)
{
	const int32 StepCount = 8;
	float StepHeight = FloorHeight / StepCount;
	float StepDepth = (Depth * 0.4f) / StepCount;
	float StepWidth = Width * 0.25f;

	float StartY = -Depth * 0.3f;

	for (int32 S = 0; S < StepCount; ++S)
	{
		float StepZ = BaseZ + StepHeight * (S + 0.5f);
		float StepY = StartY + StepDepth * S;

		CreateWallPanel(
			FVector(Width * 0.3f, StepY, StepZ),
			FVector(StepWidth / 100.f, StepDepth / 100.f, StepHeight / 100.f),
			FString::Printf(TEXT("Stair_%d_%d"), FloorIndex, S));
	}
}

UStaticMeshComponent* AExoBuilding::CreateWallPanel(const FVector& Location, const FVector& Scale, const FString& Name)
{
	FName CompName(*FString::Printf(TEXT("Wall_%d_%s"), ComponentCounter++, *Name));
	UStaticMeshComponent* Panel = NewObject<UStaticMeshComponent>(this, CompName);
	if (!Panel) return nullptr;

	Panel->SetupAttachment(BuildingRoot);
	Panel->RegisterComponent();

	// Use engine cube mesh
	static UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh)
	{
		Panel->SetStaticMesh(CubeMesh);
	}

	Panel->SetRelativeLocation(Location);
	Panel->SetRelativeScale3D(Scale);
	Panel->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Panel->SetCollisionResponseToAllChannels(ECR_Block);

	return Panel;
}

UStaticMeshComponent* AExoBuilding::CreateFloorSlab(const FVector& Location, const FVector& Scale, const FString& Name)
{
	return CreateWallPanel(Location, Scale, Name);
}

TArray<FVector> AExoBuilding::GetLootSpawnPositions() const
{
	return LootPositions;
}
