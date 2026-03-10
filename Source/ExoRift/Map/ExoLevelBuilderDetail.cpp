// ExoLevelBuilderDetail.cpp — Ground panels, energy pylons, craters, interiors
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::BuildGroundDetail()
{
	// Sci-fi floor panels around each compound
	SpawnFloorPanels(FVector(0.f, 0.f, 2.f), 6000.f, 24);          // Central hub
	SpawnFloorPanels(FVector(0.f, 80000.f, 2.f), 5000.f, 18);      // North
	SpawnFloorPanels(FVector(0.f, -80000.f, 2.f), 4500.f, 16);     // South
	SpawnFloorPanels(FVector(80000.f, 0.f, 2.f), 5000.f, 18);      // East
	SpawnFloorPanels(FVector(-80000.f, 0.f, 2.f), 4000.f, 14);     // West

	// Energy pylons at key intersections and compound entries
	FLinearColor CyanGlow(0.1f, 0.6f, 1.f);
	FLinearColor TealGlow(0.05f, 0.8f, 0.5f);
	FLinearColor AmberGlow(1.f, 0.6f, 0.1f);

	// Hub perimeter pylons
	SpawnEnergyPylon(FVector(5000.f, 4000.f, 0.f), 800.f, CyanGlow);
	SpawnEnergyPylon(FVector(-5000.f, 4000.f, 0.f), 800.f, CyanGlow);
	SpawnEnergyPylon(FVector(5000.f, -4000.f, 0.f), 800.f, CyanGlow);
	SpawnEnergyPylon(FVector(-5000.f, -4000.f, 0.f), 800.f, CyanGlow);

	// North compound gate pylons
	SpawnEnergyPylon(FVector(-4000.f, 73000.f, 0.f), 1000.f, TealGlow);
	SpawnEnergyPylon(FVector(4000.f, 73000.f, 0.f), 1000.f, TealGlow);

	// East power station pylons — amber warning glow
	SpawnEnergyPylon(FVector(73000.f, -3000.f, 0.f), 1200.f, AmberGlow);
	SpawnEnergyPylon(FVector(73000.f, 3000.f, 0.f), 1200.f, AmberGlow);

	// South research lab entry
	SpawnEnergyPylon(FVector(3000.f, -73000.f, 0.f), 900.f, CyanGlow);
	SpawnEnergyPylon(FVector(-3000.f, -73000.f, 0.f), 900.f, CyanGlow);

	// West barracks
	SpawnEnergyPylon(FVector(-73000.f, 5000.f, 0.f), 700.f, TealGlow);
	SpawnEnergyPylon(FVector(-73000.f, -5000.f, 0.f), 700.f, TealGlow);

	// Craters scattered across the battlefield
	SpawnCrater(FVector(25000.f, 15000.f, 0.f), 2000.f);
	SpawnCrater(FVector(-35000.f, -25000.f, 0.f), 3000.f);
	SpawnCrater(FVector(55000.f, -30000.f, 0.f), 1500.f);
	SpawnCrater(FVector(-15000.f, 60000.f, 0.f), 2500.f);
	SpawnCrater(FVector(70000.f, 70000.f, 0.f), 1800.f);
	SpawnCrater(FVector(-90000.f, -60000.f, 0.f), 3500.f);
	SpawnCrater(FVector(40000.f, -80000.f, 0.f), 2200.f);

	// === AMBIENT GROUND LIGHTS ===
	// Scattered atmospheric glow spots across the map
	struct FGroundGlow { FVector Pos; FLinearColor Color; float Intensity; float Radius; };
	TArray<FGroundGlow> Glows = {
		// Hub approach — cool blue
		{{15000.f, 0.f, 20.f}, {0.1f, 0.3f, 0.8f}, 3000.f, 4000.f},
		{{-15000.f, 0.f, 20.f}, {0.1f, 0.3f, 0.8f}, 3000.f, 4000.f},
		{{0.f, 15000.f, 20.f}, {0.1f, 0.3f, 0.8f}, 3000.f, 4000.f},
		{{0.f, -15000.f, 20.f}, {0.1f, 0.3f, 0.8f}, 3000.f, 4000.f},
		// North industrial — warm amber
		{{0.f, 70000.f, 20.f}, {0.8f, 0.5f, 0.1f}, 2000.f, 5000.f},
		{{-8000.f, 85000.f, 20.f}, {0.8f, 0.5f, 0.1f}, 2500.f, 3000.f},
		// South research — teal
		{{0.f, -70000.f, 20.f}, {0.1f, 0.6f, 0.5f}, 2000.f, 4000.f},
		{{5000.f, -85000.f, 20.f}, {0.1f, 0.6f, 0.5f}, 1500.f, 3000.f},
		// East power — red warning
		{{70000.f, 0.f, 20.f}, {0.8f, 0.15f, 0.05f}, 2500.f, 5000.f},
		{{85000.f, 5000.f, 20.f}, {0.8f, 0.15f, 0.05f}, 2000.f, 3000.f},
		// West barracks — green
		{{-70000.f, 0.f, 20.f}, {0.1f, 0.5f, 0.2f}, 2000.f, 4000.f},
		// Open field atmospheric — purple
		{{50000.f, 50000.f, 20.f}, {0.3f, 0.1f, 0.5f}, 1500.f, 6000.f},
		{{-50000.f, -50000.f, 20.f}, {0.3f, 0.1f, 0.5f}, 1500.f, 6000.f},
		// Road intersections — white
		{{0.f, 0.f, 30.f}, {0.5f, 0.6f, 0.8f}, 4000.f, 5000.f},
	};
	for (const auto& G : Glows)
	{
		UPointLightComponent* GL = NewObject<UPointLightComponent>(this);
		GL->SetupAttachment(RootComponent);
		GL->SetWorldLocation(G.Pos);
		GL->SetIntensity(G.Intensity);
		GL->SetAttenuationRadius(G.Radius);
		GL->SetLightColor(G.Color);
		GL->CastShadows = false;
		GL->RegisterComponent();

		// Small emissive ground disk at each glow
		UStaticMeshComponent* Disk = SpawnStaticMesh(
			G.Pos - FVector(0.f, 0.f, 15.f),
			FVector(G.Radius / 2000.f, G.Radius / 2000.f, 0.02f),
			FRotator::ZeroRotator, CylinderMesh, G.Color);
		if (Disk && BaseMaterial)
		{
			UMaterialInstanceDynamic* DM = Cast<UMaterialInstanceDynamic>(
				Disk->GetMaterial(0));
			if (DM) DM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(G.Color.R * 0.3f, G.Color.G * 0.3f, G.Color.B * 0.3f));
		}
	}

	// Hazard floor markings near dangerous areas
	FLinearColor YellowStripe(0.6f, 0.5f, 0.05f);
	struct FHazardMark { FVector Pos; float Yaw; };
	TArray<FHazardMark> Marks = {
		{{-50000.f, 50000.f, 3.f}, 0.f},    // Radiation zone
		{{60000.f, -60000.f, 3.f}, 45.f},    // Electric zone
		{{-100000.f, -40000.f, 3.f}, -15.f}, // Toxic zone
		{{40000.f, 100000.f, 3.f}, 30.f},    // Fire zone
	};
	for (const auto& M : Marks)
	{
		// Warning stripe pair
		SpawnStaticMesh(M.Pos + FVector(500.f, 0.f, 0.f),
			FVector(0.5f, 20.f, 0.05f), FRotator(0.f, M.Yaw, 0.f),
			CubeMesh, YellowStripe);
		SpawnStaticMesh(M.Pos + FVector(-500.f, 0.f, 0.f),
			FVector(0.5f, 20.f, 0.05f), FRotator(0.f, M.Yaw, 0.f),
			CubeMesh, YellowStripe);
	}
}

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

void AExoLevelBuilder::SpawnFloorPanels(const FVector& Center, float Radius, int32 Count)
{
	FLinearColor PanelLight(0.055f, 0.06f, 0.07f);
	FLinearColor PanelDark(0.035f, 0.04f, 0.045f);
	FLinearColor SeamGlow(0.05f, 0.2f, 0.4f);

	for (int32 i = 0; i < Count; i++)
	{
		float Angle = (float)i / (float)Count * 2.f * PI;
		float R = Radius * (0.4f + 0.6f * FMath::Fmod(i * 0.618034f, 1.f));
		FVector Pos = Center + FVector(
			FMath::Cos(Angle) * R,
			FMath::Sin(Angle) * R, 0.f);

		// Alternating panel sizes and colors
		float PanelSize = 400.f + (i % 3) * 200.f;
		float PanelScale = PanelSize / 100.f;
		FLinearColor Col = (i % 2 == 0) ? PanelLight : PanelDark;

		SpawnStaticMesh(Pos,
			FVector(PanelScale, PanelScale, 0.02f),
			FRotator(0.f, i * 47.f, 0.f),
			CubeMesh, Col);

		// Thin glowing seam line on every other panel
		if (i % 3 == 0 && BaseMaterial)
		{
			UStaticMeshComponent* Seam = SpawnStaticMesh(
				Pos + FVector(0.f, 0.f, 1.f),
				FVector(PanelScale * 0.9f, 0.03f, 0.03f),
				FRotator(0.f, i * 47.f, 0.f),
				CubeMesh, SeamGlow);
			if (Seam)
			{
				UMaterialInstanceDynamic* SeamMat = Cast<UMaterialInstanceDynamic>(
					Seam->GetMaterial(0));
				if (SeamMat)
				{
					SeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
						FLinearColor(0.1f, 0.4f, 0.8f));
				}
			}
		}
	}
}

void AExoLevelBuilder::SpawnEnergyPylon(const FVector& Base, float Height,
	const FLinearColor& Color)
{
	// Dark metal column
	float ColH = Height / 100.f;
	SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(0.6f, 0.6f, ColH),
		FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.06f, 0.06f, 0.08f));

	// Energy ring at top
	UStaticMeshComponent* Ring = SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height * 0.85f),
		FVector(1.0f, 1.0f, 0.06f),
		FRotator::ZeroRotator, CylinderMesh, Color);
	if (Ring && BaseMaterial)
	{
		UMaterialInstanceDynamic* RingMat = Cast<UMaterialInstanceDynamic>(
			Ring->GetMaterial(0));
		if (RingMat)
		{
			RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
		}
	}

	// Energy ring at mid
	UStaticMeshComponent* MidRing = SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height * 0.45f),
		FVector(0.8f, 0.8f, 0.04f),
		FRotator::ZeroRotator, CylinderMesh, Color);
	if (MidRing && BaseMaterial)
	{
		UMaterialInstanceDynamic* MidMat = Cast<UMaterialInstanceDynamic>(
			MidRing->GetMaterial(0));
		if (MidMat)
		{
			MidMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 2.f, Color.G * 2.f, Color.B * 2.f));
		}
	}

	// Cap sphere
	SpawnStaticMesh(
		Base + FVector(0.f, 0.f, Height),
		FVector(0.7f, 0.7f, 0.5f),
		FRotator::ZeroRotator, SphereMesh,
		FLinearColor(0.08f, 0.08f, 0.1f));

	// Point light at top
	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetupAttachment(RootComponent);
	Light->SetWorldLocation(Base + FVector(0.f, 0.f, Height));
	Light->SetIntensity(4000.f);
	Light->SetAttenuationRadius(Height * 2.f);
	Light->SetLightColor(Color);
	Light->CastShadows = false;
	Light->RegisterComponent();
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

void AExoLevelBuilder::SpawnCrater(const FVector& Center, float Radius)
{
	// Crater rim — ring of raised terrain
	float RimScale = Radius / 50.f;
	SpawnStaticMesh(
		Center + FVector(0.f, 0.f, -20.f),
		FVector(RimScale, RimScale, 0.4f),
		FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.03f, 0.035f, 0.04f));

	// Inner depression — darker center
	float InnerScale = Radius * 0.7f / 50.f;
	SpawnStaticMesh(
		Center + FVector(0.f, 0.f, -40.f),
		FVector(InnerScale, InnerScale, 0.3f),
		FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.02f, 0.025f, 0.03f));

	// Scattered debris chunks around rim
	int32 NumDebris = FMath::RandRange(3, 6);
	for (int32 i = 0; i < NumDebris; i++)
	{
		float Angle = (float)i / (float)NumDebris * 2.f * PI + FMath::RandRange(-0.3f, 0.3f);
		float Dist = Radius * FMath::RandRange(0.8f, 1.2f);
		FVector DebrisPos = Center + FVector(
			FMath::Cos(Angle) * Dist,
			FMath::Sin(Angle) * Dist,
			FMath::RandRange(0.f, 30.f));
		float S = FMath::RandRange(0.3f, 1.2f);
		SpawnStaticMesh(DebrisPos,
			FVector(S, S * 0.6f, S * 0.4f),
			FRotator(FMath::RandRange(-15.f, 15.f), FMath::RandRange(0.f, 360.f), 0.f),
			CubeMesh, FLinearColor(0.04f, 0.04f, 0.05f));
	}
}
