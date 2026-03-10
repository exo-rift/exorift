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
