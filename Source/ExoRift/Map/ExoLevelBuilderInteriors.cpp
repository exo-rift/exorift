// ExoLevelBuilderInteriors.cpp — Building interior furnishings
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::BuildInteriors()
{
	// Central hub — command consoles
	SpawnConsole(FVector(2000.f, 1500.f, 10.f), 0.f);
	SpawnConsole(FVector(2000.f, -1500.f, 10.f), 0.f);
	SpawnConsole(FVector(-1500.f, 0.f, 10.f), 90.f);
	SpawnConsole(FVector(-2000.f, 500.f, 1210.f), 180.f);

	// North compound — industrial workstations
	SpawnConsole(FVector(-2000.f, 81000.f, 10.f), -15.f);
	SpawnConsole(FVector(3000.f, 79000.f, 10.f), 15.f);

	// South research — lab benches
	SpawnConsole(FVector(3500.f, -80500.f, 10.f), 0.f);
	SpawnConsole(FVector(3500.f, -79500.f, 10.f), 0.f);
	SpawnConsole(FVector(-5500.f, -78500.f, 10.f), 90.f);

	// East power station
	SpawnConsole(FVector(81000.f, 0.f, 10.f), -45.f);
	SpawnConsole(FVector(79000.f, 2500.f, 10.f), 0.f);

	// West barracks
	SpawnConsole(FVector(-81500.f, -3000.f, 10.f), 90.f);
	SpawnConsole(FVector(-78500.f, 3000.f, 10.f), -90.f);

	FLinearColor DarkMetal(0.06f, 0.06f, 0.08f);
	FLinearColor CrateColor(0.08f, 0.07f, 0.06f);

	// === SERVER RACKS — tall narrow units with emissive status lights ===
	auto SpawnServerRack = [&](const FVector& Pos, float Yaw)
	{
		FRotator R(0.f, Yaw, 0.f);
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 100.f),
			FVector(0.7f, 2.f, 2.f), R, CubeMesh, DarkMetal);
		// Status light strip
		UStaticMeshComponent* Strip = SpawnStaticMesh(
			Pos + R.RotateVector(FVector(38.f, 0.f, 100.f)),
			FVector(0.02f, 1.6f, 0.08f), R, CubeMesh,
			FLinearColor(0.05f, 0.4f, 0.1f));
		if (Strip && BaseMaterial)
		{
			UMaterialInstanceDynamic* SM = Cast<UMaterialInstanceDynamic>(
				Strip->GetMaterial(0));
			if (SM) SM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.2f, 2.f, 0.4f));
		}
	};
	// Hub server room
	SpawnServerRack(FVector(-3000.f, -1500.f, 10.f), 0.f);
	SpawnServerRack(FVector(-3000.f, -900.f, 10.f), 0.f);
	SpawnServerRack(FVector(-3000.f, -300.f, 10.f), 0.f);
	// East power station
	SpawnServerRack(FVector(79500.f, -2000.f, 10.f), 90.f);
	SpawnServerRack(FVector(79500.f, -1200.f, 10.f), 90.f);

	// === WORKBENCHES — flat tables with tools ===
	auto SpawnBench = [&](const FVector& Pos, float Yaw, const FLinearColor& ToolCol)
	{
		FRotator R(0.f, Yaw, 0.f);
		// Table top
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 45.f),
			FVector(2.f, 0.8f, 0.06f), R, CubeMesh, DarkMetal);
		// Legs
		for (float LX : {-80.f, 80.f})
			for (float LY : {-30.f, 30.f})
				SpawnStaticMesh(Pos + R.RotateVector(FVector(LX, LY, 22.f)),
					FVector(0.08f, 0.08f, 0.45f), R, CylinderMesh, DarkMetal);
		// Tool tray
		UStaticMeshComponent* Tray = SpawnStaticMesh(
			Pos + R.RotateVector(FVector(50.f, 0.f, 52.f)),
			FVector(0.6f, 0.3f, 0.04f), R, CubeMesh, ToolCol);
		if (Tray && BaseMaterial)
		{
			UMaterialInstanceDynamic* TM = Cast<UMaterialInstanceDynamic>(
				Tray->GetMaterial(0));
			if (TM) TM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(ToolCol.R * 2.f, ToolCol.G * 2.f, ToolCol.B * 2.f));
		}
	};
	// North warehouse workbenches
	SpawnBench(FVector(-7000.f, 80000.f, 10.f), 0.f,
		FLinearColor(0.6f, 0.3f, 0.05f));
	SpawnBench(FVector(-7000.f, 81500.f, 10.f), 0.f,
		FLinearColor(0.6f, 0.3f, 0.05f));
	// South lab benches
	SpawnBench(FVector(1000.f, -80000.f, 10.f), 90.f,
		FLinearColor(0.05f, 0.4f, 0.6f));
	SpawnBench(FVector(1000.f, -81500.f, 10.f), 90.f,
		FLinearColor(0.05f, 0.4f, 0.6f));

	// === MEDICAL STATION — red-cross marked cabinet with green glow ===
	auto SpawnMedStation = [&](const FVector& Pos, float Yaw)
	{
		FRotator R(0.f, Yaw, 0.f);
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 70.f),
			FVector(1.2f, 0.5f, 1.4f), R, CubeMesh,
			FLinearColor(0.08f, 0.08f, 0.08f));
		// Cross mark (horizontal + vertical bars)
		UStaticMeshComponent* CrossH = SpawnStaticMesh(
			Pos + R.RotateVector(FVector(0.f, 26.f, 80.f)),
			FVector(0.6f, 0.02f, 0.08f), R, CubeMesh,
			FLinearColor(0.5f, 0.05f, 0.05f));
		UStaticMeshComponent* CrossV = SpawnStaticMesh(
			Pos + R.RotateVector(FVector(0.f, 26.f, 80.f)),
			FVector(0.08f, 0.02f, 0.6f), R, CubeMesh,
			FLinearColor(0.5f, 0.05f, 0.05f));
		for (UStaticMeshComponent* C : {CrossH, CrossV})
		{
			if (C && BaseMaterial)
			{
				UMaterialInstanceDynamic* CM = Cast<UMaterialInstanceDynamic>(
					C->GetMaterial(0));
				if (CM) CM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(3.f, 0.3f, 0.2f));
			}
		}
		// Status light
		UPointLightComponent* ML = NewObject<UPointLightComponent>(this);
		ML->SetupAttachment(RootComponent);
		ML->SetWorldLocation(Pos + FVector(0.f, 0.f, 150.f));
		ML->SetIntensity(1200.f);
		ML->SetAttenuationRadius(400.f);
		ML->SetLightColor(FLinearColor(0.1f, 0.8f, 0.3f));
		ML->CastShadows = false;
		ML->RegisterComponent();
	};
	SpawnMedStation(FVector(-79500.f, -7500.f, 10.f), 90.f);
	SpawnMedStation(FVector(-79500.f, 2500.f, 10.f), 90.f);
	SpawnMedStation(FVector(2500.f, -82000.f, 10.f), 0.f);
	SpawnMedStation(FVector(-2000.f, 2500.f, 10.f), 180.f);

	// === CRATE STACKS ===
	struct FIC { FVector Pos; FVector Scale; float Yaw; };
	TArray<FIC> Crates = {
		{{3500.f, 2500.f, 80.f}, {2.f, 1.f, 1.6f}, 5.f},
		{{3500.f, -2500.f, 80.f}, {1.5f, 1.2f, 1.6f}, -10.f},
		{{-3000.f, 2000.f, 80.f}, {2.f, 1.5f, 1.f}, 0.f},
		{{-3500.f, 82000.f, 80.f}, {3.f, 1.5f, 2.f}, 15.f},
		{{2000.f, 83000.f, 80.f}, {2.f, 2.f, 1.2f}, 0.f},
		{{5000.f, -81500.f, 80.f}, {1.5f, 1.5f, 2.5f}, 0.f},
		{{82000.f, -3000.f, 80.f}, {2.5f, 1.f, 1.8f}, -45.f},
		{{-79000.f, -5000.f, 80.f}, {1.5f, 1.5f, 1.5f}, 25.f},
		{{-82000.f, 5000.f, 80.f}, {2.f, 1.f, 2.f}, 0.f},
	};
	for (const auto& C : Crates)
		SpawnStaticMesh(C.Pos, C.Scale, FRotator(0.f, C.Yaw, 0.f), CubeMesh, CrateColor);

	// === WEAPON RACKS — angled display stands (west barracks) ===
	for (int32 i = 0; i < 3; i++)
	{
		FVector RackPos(-80500.f, -6000.f + i * 2500.f, 10.f);
		SpawnStaticMesh(RackPos + FVector(0.f, 0.f, 60.f),
			FVector(0.15f, 1.2f, 1.2f), FRotator(15.f, 90.f, 0.f),
			CubeMesh, FLinearColor(0.09f, 0.09f, 0.1f));
		// Accent light
		UPointLightComponent* RL = NewObject<UPointLightComponent>(this);
		RL->SetupAttachment(RootComponent);
		RL->SetWorldLocation(RackPos + FVector(0.f, 0.f, 130.f));
		RL->SetIntensity(600.f);
		RL->SetAttenuationRadius(250.f);
		RL->SetLightColor(FLinearColor(0.2f, 0.4f, 0.8f));
		RL->CastShadows = false;
		RL->RegisterComponent();
	}

	// === CHAIRS — near consoles and workstations ===
	auto SpawnChair = [&](const FVector& Pos, float Yaw)
	{
		FRotator R(0.f, Yaw, 0.f);
		// Seat
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 30.f),
			FVector(0.4f, 0.4f, 0.04f), R, CubeMesh, DarkMetal);
		// Backrest
		SpawnStaticMesh(Pos + R.RotateVector(FVector(-18.f, 0.f, 55.f)),
			FVector(0.04f, 0.35f, 0.5f), R, CubeMesh, DarkMetal);
		// Pedestal leg
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 15.f),
			FVector(0.06f, 0.06f, 0.3f), R, CylinderMesh, DarkMetal);
		// Base star (cross shape)
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 3.f),
			FVector(0.3f, 0.06f, 0.03f), R, CubeMesh, DarkMetal);
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 3.f),
			FVector(0.06f, 0.3f, 0.03f), R, CubeMesh, DarkMetal);
	};
	// Hub chairs
	SpawnChair(FVector(2000.f, 1500.f, 10.f) + FVector(100.f, 0.f, 0.f), 180.f);
	SpawnChair(FVector(2000.f, -1500.f, 10.f) + FVector(100.f, 0.f, 0.f), 180.f);
	SpawnChair(FVector(-1500.f, 0.f, 10.f) + FVector(0.f, 100.f, 0.f), -90.f);
	// North compound
	SpawnChair(FVector(-2000.f, 81000.f, 10.f) + FVector(100.f, 0.f, 0.f), 165.f);
	// East power
	SpawnChair(FVector(81000.f, 0.f, 10.f) + FVector(80.f, 80.f, 0.f), -135.f);

	// === LOCKERS — barracks personal storage ===
	auto SpawnLockerRow = [&](const FVector& Start, float Yaw, int32 Count)
	{
		FRotator R(0.f, Yaw, 0.f);
		FVector Step = R.RotateVector(FVector(0.f, 60.f, 0.f));
		for (int32 i = 0; i < Count; i++)
		{
			FVector Pos = Start + Step * (float)i;
			// Locker body
			SpawnStaticMesh(Pos + FVector(0.f, 0.f, 90.f),
				FVector(0.5f, 0.45f, 1.8f), R, CubeMesh,
				FLinearColor(0.06f, 0.065f, 0.07f));
			// Door handle
			SpawnStaticMesh(Pos + R.RotateVector(FVector(26.f, -8.f, 80.f)),
				FVector(0.015f, 0.005f, 0.08f), R, CubeMesh,
				FLinearColor(0.12f, 0.12f, 0.14f));
		}
	};
	// West barracks locker rows
	SpawnLockerRow(FVector(-81000.f, -8000.f, 10.f), 0.f, 5);
	SpawnLockerRow(FVector(-79000.f, -8000.f, 10.f), 180.f, 5);
	SpawnLockerRow(FVector(-81000.f, 6000.f, 10.f), 0.f, 4);

	// === BUNK BEDS — barracks sleeping quarters ===
	auto SpawnBunk = [&](const FVector& Pos, float Yaw)
	{
		FRotator R(0.f, Yaw, 0.f);
		FLinearColor Frame(0.07f, 0.07f, 0.08f);
		FLinearColor Mattress(0.12f, 0.1f, 0.08f);
		// Lower bed frame
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 25.f),
			FVector(2.f, 0.8f, 0.04f), R, CubeMesh, Frame);
		// Lower mattress
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 30.f),
			FVector(1.8f, 0.7f, 0.08f), R, CubeMesh, Mattress);
		// Upper bed frame
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 120.f),
			FVector(2.f, 0.8f, 0.04f), R, CubeMesh, Frame);
		// Upper mattress
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 125.f),
			FVector(1.8f, 0.7f, 0.08f), R, CubeMesh, Mattress);
		// Corner posts
		for (float BX : {-90.f, 90.f})
			for (float BY : {-35.f, 35.f})
				SpawnStaticMesh(Pos + R.RotateVector(FVector(BX, BY, 75.f)),
					FVector(0.04f, 0.04f, 1.5f), R, CylinderMesh, Frame);
	};
	// Barracks bunks
	SpawnBunk(FVector(-82000.f, -3000.f, 10.f), 0.f);
	SpawnBunk(FVector(-82000.f, -1500.f, 10.f), 0.f);
	SpawnBunk(FVector(-82000.f, 0.f, 10.f), 0.f);
	SpawnBunk(FVector(-82000.f, 1500.f, 10.f), 0.f);

	// === MONITOR ARRAY — multi-screen wall display in hub ===
	{
		FLinearColor ScreenFrame(0.04f, 0.04f, 0.05f);
		FVector ArrayCenter(0.f, -4000.f, 200.f);
		for (int32 Row = 0; Row < 2; Row++)
		{
			for (int32 Col = 0; Col < 3; Col++)
			{
				FVector Offset((Col - 1) * 200.f, 0.f, Row * 150.f);
				// Frame
				SpawnStaticMesh(ArrayCenter + Offset,
					FVector(1.6f, 0.06f, 1.2f), FRotator::ZeroRotator,
					CubeMesh, ScreenFrame);
				// Screen face
				FLinearColor ScreenCol;
				switch ((Row * 3 + Col) % 4)
				{
				case 0: ScreenCol = FLinearColor(0.05f, 0.2f, 0.4f); break;
				case 1: ScreenCol = FLinearColor(0.1f, 0.35f, 0.15f); break;
				case 2: ScreenCol = FLinearColor(0.3f, 0.15f, 0.05f); break;
				default: ScreenCol = FLinearColor(0.05f, 0.25f, 0.35f); break;
				}
				UStaticMeshComponent* MonScreen = SpawnStaticMesh(
					ArrayCenter + Offset + FVector(0.f, -4.f, 0.f),
					FVector(1.4f, 0.02f, 1.0f), FRotator::ZeroRotator,
					CubeMesh, ScreenCol);
				if (MonScreen && BaseMaterial)
				{
					UMaterialInstanceDynamic* MSM = Cast<UMaterialInstanceDynamic>(
						MonScreen->GetMaterial(0));
					if (MSM) MSM->SetVectorParameterValue(TEXT("EmissiveColor"),
						FLinearColor(ScreenCol.R * 3.f, ScreenCol.G * 3.f,
							ScreenCol.B * 3.f));
				}
			}
		}
		// Array frame border
		SpawnStaticMesh(ArrayCenter + FVector(0.f, 2.f, 75.f),
			FVector(3.5f, 0.04f, 3.2f), FRotator::ZeroRotator,
			CubeMesh, ScreenFrame);
		// Array ambient light
		UPointLightComponent* ArrayLight = NewObject<UPointLightComponent>(this);
		ArrayLight->SetupAttachment(RootComponent);
		ArrayLight->SetWorldLocation(ArrayCenter + FVector(0.f, -50.f, 100.f));
		ArrayLight->SetIntensity(2000.f);
		ArrayLight->SetAttenuationRadius(600.f);
		ArrayLight->SetLightColor(FLinearColor(0.15f, 0.3f, 0.5f));
		ArrayLight->CastShadows = false;
		ArrayLight->RegisterComponent();
	}

	// === LAB EQUIPMENT — south research compound ===
	{
		// Centrifuge — short cylinder with emissive ring
		FVector CentPos(-3000.f, -80500.f, 10.f);
		SpawnStaticMesh(CentPos + FVector(0.f, 0.f, 40.f),
			FVector(0.5f, 0.5f, 0.4f), FRotator::ZeroRotator,
			CylinderMesh, DarkMetal);
		UStaticMeshComponent* CentRing = SpawnStaticMesh(
			CentPos + FVector(0.f, 0.f, 60.f),
			FVector(0.55f, 0.55f, 0.03f), FRotator::ZeroRotator,
			CylinderMesh, FLinearColor(0.1f, 0.5f, 0.3f));
		if (CentRing && BaseMaterial)
		{
			UMaterialInstanceDynamic* CRM = Cast<UMaterialInstanceDynamic>(
				CentRing->GetMaterial(0));
			if (CRM) CRM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.3f, 2.f, 1.f));
		}

		// Specimen containers — glass-like cylinders with glow
		for (int32 i = 0; i < 3; i++)
		{
			FVector ContPos(-4000.f + i * 500.f, -81500.f, 10.f);
			SpawnStaticMesh(ContPos + FVector(0.f, 0.f, 50.f),
				FVector(0.2f, 0.2f, 1.0f), FRotator::ZeroRotator,
				CylinderMesh, FLinearColor(0.08f, 0.12f, 0.15f));
			// Glowing contents
			FLinearColor ContGlow = (i == 0) ? FLinearColor(0.1f, 0.6f, 0.3f) :
				(i == 1) ? FLinearColor(0.5f, 0.2f, 0.05f) :
				FLinearColor(0.15f, 0.3f, 0.6f);
			UPointLightComponent* CL = NewObject<UPointLightComponent>(this);
			CL->SetupAttachment(RootComponent);
			CL->SetWorldLocation(ContPos + FVector(0.f, 0.f, 50.f));
			CL->SetIntensity(1500.f);
			CL->SetAttenuationRadius(200.f);
			CL->SetLightColor(ContGlow);
			CL->CastShadows = false;
			CL->RegisterComponent();
		}
	}

	// === STORAGE SHELVES — generic throughout compounds ===
	auto SpawnShelf = [&](const FVector& Pos, float Yaw, int32 Shelves)
	{
		FRotator R(0.f, Yaw, 0.f);
		FLinearColor SC(0.065f, 0.065f, 0.075f);
		for (int32 i = 0; i < Shelves; i++)
			SpawnStaticMesh(Pos + FVector(0.f, 0.f, 20.f + i * 60.f),
				FVector(1.8f, 0.5f, 0.03f), R, CubeMesh, SC);
		for (float SY : {-22.f, 22.f})
			SpawnStaticMesh(Pos + R.RotateVector(FVector(0.f, SY, Shelves * 30.f)),
				FVector(1.8f, 0.04f, Shelves * 0.6f), R, CubeMesh, SC);
	};
	SpawnShelf(FVector(4000.f, 1000.f, 10.f), 0.f, 3);
	SpawnShelf(FVector(4000.f, -1000.f, 10.f), 0.f, 3);
	SpawnShelf(FVector(-5000.f, 79500.f, 10.f), 90.f, 4);
}

void AExoLevelBuilder::SpawnConsole(const FVector& Pos, float Yaw)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FLinearColor DarkMetal(0.06f, 0.06f, 0.08f);
	FLinearColor ScreenGlow(0.05f, 0.3f, 0.6f);

	// Console body — angled desk
	SpawnStaticMesh(Pos, FVector(1.5f, 0.8f, 0.8f), Rot, CubeMesh, DarkMetal);

	// Screen panel — thin glowing rectangle on top
	FVector ScreenOffset = Rot.RotateVector(FVector(0.f, 0.f, 85.f));
	UStaticMeshComponent* Screen = SpawnStaticMesh(
		Pos + ScreenOffset,
		FVector(1.2f, 0.6f, 0.03f),
		Rot + FRotator(20.f, 0.f, 0.f), // Tilted toward user
		CubeMesh, ScreenGlow);
	if (Screen && BaseMaterial)
	{
		UMaterialInstanceDynamic* ScrMat = Cast<UMaterialInstanceDynamic>(
			Screen->GetMaterial(0));
		if (ScrMat)
		{
			ScrMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.1f, 0.5f, 1.0f));
		}
	}

	// Small light above the console
	UPointLightComponent* ConLight = NewObject<UPointLightComponent>(this);
	ConLight->SetupAttachment(RootComponent);
	ConLight->SetWorldLocation(Pos + FVector(0.f, 0.f, 120.f));
	ConLight->SetIntensity(800.f);
	ConLight->SetAttenuationRadius(300.f);
	ConLight->SetLightColor(FLinearColor(0.2f, 0.5f, 0.8f));
	ConLight->CastShadows = false;
	ConLight->RegisterComponent();
}
