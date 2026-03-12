// ExoLevelBuilderStructures.cpp — Compound layouts, towers, walls, ramps
// SpawnBuilding is in ExoLevelBuilderBuildings.cpp
#include "Map/ExoLevelBuilder.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildStructures()
{
	// === CENTRAL HUB — large command center ===
	SpawnBuilding(FVector(0.f, 0.f, 0.f), FVector(8000.f, 6000.f, 2500.f));
	SpawnBuilding(FVector(0.f, 0.f, 2500.f), FVector(4000.f, 3000.f, 1500.f));
	SpawnPlatform(FVector(5000.f, 0.f, 1500.f), FVector(3000.f, 2000.f, 100.f));
	SpawnRamp(FVector(6500.f, 0.f, 0.f), 2000.f, 1500.f, 500.f, 180.f);
	// Hub interior platforms
	SpawnPlatform(FVector(-2000.f, 0.f, 1200.f), FVector(2000.f, 1500.f, 80.f));
	SpawnRamp(FVector(-3500.f, 0.f, 0.f), 1500.f, 1200.f, 500.f, 0.f);

	// Hub comm tower (center roof)
	SpawnTower(FVector(0.f, 0.f, 4000.f), 400.f, 2000.f);
	SpawnAntenna(FVector(0.f, 0.f, 6000.f), 2000.f);

	// Landing pad (east of hub) — flat platform with emissive markings
	FVector PadCenter(8000.f, 0.f, 30.f);
	SpawnStaticMesh(PadCenter, FVector(30.f, 30.f, 0.3f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.1f, 0.11f));
	// Pad border stripes
	UStaticMeshComponent* PadBorderN = SpawnStaticMesh(
		PadCenter + FVector(0.f, 1500.f, 5.f), FVector(30.f, 0.1f, 0.05f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.4f, 0.1f));
	UStaticMeshComponent* PadBorderS = SpawnStaticMesh(
		PadCenter + FVector(0.f, -1500.f, 5.f), FVector(30.f, 0.1f, 0.05f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.4f, 0.1f));
	// Center H marking
	SpawnStaticMesh(PadCenter + FVector(0.f, 0.f, 5.f),
		FVector(0.15f, 6.f, 0.04f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.8f, 0.8f, 0.8f)); // Vertical bar of H
	SpawnStaticMesh(PadCenter + FVector(300.f, 0.f, 5.f),
		FVector(0.15f, 6.f, 0.04f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.8f, 0.8f, 0.8f)); // Second vertical
	SpawnStaticMesh(PadCenter + FVector(150.f, 0.f, 5.f),
		FVector(3.f, 0.15f, 0.04f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.8f, 0.8f, 0.8f)); // Cross bar

	// === NORTH COMPOUND — industrial facility ===
	float NY = 80000.f;
	// Main warehouse
	SpawnBuilding(FVector(-5000.f, NY, 0.f), FVector(6000.f, 4000.f, 3000.f), 15.f);
	SpawnBuilding(FVector(5000.f, NY, 0.f), FVector(4000.f, 5000.f, 2000.f));
	// Crane/gantry structure
	SpawnPlatform(FVector(0.f, NY, 4000.f), FVector(12000.f, 800.f, 200.f));
	SpawnStaticMesh(FVector(-5500.f, NY, 2000.f), FVector(0.5f, 0.5f, 40.f),
		FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
	SpawnStaticMesh(FVector(5500.f, NY, 2000.f), FVector(0.5f, 0.5f, 40.f),
		FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
	// Perimeter wall with walkway
	SpawnWall(FVector(-10000.f, NY - 4000.f, 0.f), FVector(10000.f, NY - 4000.f, 0.f), 1500.f);
	SpawnPlatform(FVector(0.f, NY - 4000.f, 1500.f), FVector(20000.f, 600.f, 80.f));
	SpawnTower(FVector(0.f, NY + 5000.f, 0.f), 800.f, 5000.f);
	// Storage tanks — with horizontal bands, top dome, and ladder strip
	for (int32 i = 0; i < 3; i++)
	{
		float TankX = -8000.f + i * 4000.f;
		float TankY = NY + 6000.f;
		SpawnStaticMesh(FVector(TankX, TankY, 1500.f),
			FVector(10.f, 10.f, 30.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));
		// Top dome cap
		SpawnStaticMesh(FVector(TankX, TankY, 3050.f),
			FVector(10.f, 10.f, 3.f), FRotator::ZeroRotator, SphereMesh,
			FLinearColor(0.09f, 0.09f, 0.11f));
		// Horizontal banding rings
		for (float BZ : {800.f, 1600.f, 2400.f})
		{
			SpawnStaticMesh(FVector(TankX, TankY, BZ),
				FVector(10.3f, 10.3f, 0.3f), FRotator::ZeroRotator, CylinderMesh,
				FLinearColor(0.08f, 0.08f, 0.1f));
		}
		// Ladder strip (thin vertical bar on tank surface)
		SpawnStaticMesh(FVector(TankX + 490.f, TankY, 1500.f),
			FVector(0.1f, 0.06f, 30.f), FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
	}
	// Industrial smokestack — landmark visible from afar
	SpawnStaticMesh(FVector(8000.f, NY + 5000.f, 2500.f),
		FVector(3.f, 3.f, 50.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.08f, 0.08f, 0.1f));
	// Smoke top ring (emissive orange, suggests active furnace)
	UStaticMeshComponent* SmokeCap = SpawnStaticMesh(
		FVector(8000.f, NY + 5000.f, 5050.f),
		FVector(3.5f, 3.5f, 0.1f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.6f, 0.3f, 0.05f));
	if (SmokeCap)
	{
		UMaterialInterface* CapEmissive = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* CM = UMaterialInstanceDynamic::Create(CapEmissive, this);
		if (!CM) { return; }
		CM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.f, 0.8f, 0.15f));
		SmokeCap->SetMaterial(0, CM);
	}

	// === SOUTH COMPOUND — research labs ===
	float SY = -80000.f;
	SpawnBuilding(FVector(3000.f, SY, 0.f), FVector(5000.f, 7000.f, 2200.f), -10.f);
	SpawnBuilding(FVector(-6000.f, SY + 2000.f, 0.f), FVector(3500.f, 3500.f, 1800.f));
	SpawnBuilding(FVector(-6000.f, SY - 3000.f, 0.f), FVector(3500.f, 3000.f, 2000.f));
	// Elevated walkway between labs
	SpawnPlatform(FVector(-1000.f, SY, 2500.f), FVector(5000.f, 1500.f, 100.f));
	// Research dome — translucent glass canopy with inner glow
	UStaticMeshComponent* Dome = SpawnStaticMesh(
		FVector(3000.f, SY, 2200.f), FVector(25.f, 25.f, 12.f),
		FRotator::ZeroRotator, SphereMesh, FLinearColor(0.02f, 0.04f, 0.06f));
	UMaterialInterface* DomeGlass = FExoMaterialFactory::GetGlassTranslucent();
	if (Dome && DomeGlass)
	{
		UMaterialInstanceDynamic* DM = UMaterialInstanceDynamic::Create(DomeGlass, this);
		if (!DM) { return; }
		DM->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.01f, 0.03f, 0.05f));
		DM->SetScalarParameterValue(TEXT("Metallic"), 0.15f);
		DM->SetScalarParameterValue(TEXT("Roughness"), 0.05f);
		DM->SetScalarParameterValue(TEXT("Specular"), 0.9f);
		DM->SetScalarParameterValue(TEXT("Opacity"), 0.25f);
		DM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.02f, 0.12f, 0.08f));
		Dome->SetMaterial(0, DM);
	}
	// Dome structural ribs (meridian arcs)
	for (float Yaw : {0.f, 45.f, 90.f, 135.f})
	{
		SpawnStaticMesh(FVector(3000.f, SY, 2200.f),
			FVector(25.2f, 0.12f, 12.2f), FRotator(0.f, Yaw, 0.f), SphereMesh,
			FLinearColor(0.06f, 0.065f, 0.08f));
	}
	// Containment area
	SpawnWall(FVector(-9000.f, SY - 5000.f, 0.f), FVector(-9000.f, SY + 5000.f, 0.f), 1800.f);
	SpawnWall(FVector(-9000.f, SY + 5000.f, 0.f), FVector(9000.f, SY + 5000.f, 0.f), 1800.f);
	// Tesla coil landmark — tall conductor with emissive tip
	SpawnStaticMesh(FVector(-6000.f, SY, 1800.f),
		FVector(0.4f, 0.4f, 36.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.12f, 0.12f, 0.15f));
	UStaticMeshComponent* TeslaOrb = SpawnStaticMesh(
		FVector(-6000.f, SY, 3650.f),
		FVector(2.f, 2.f, 2.f), FRotator::ZeroRotator, SphereMesh,
		FLinearColor(0.1f, 0.6f, 0.3f));
	if (TeslaOrb)
	{
		UMaterialInterface* TeslaEmissive = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* TM = UMaterialInstanceDynamic::Create(TeslaEmissive, this);
		if (!TM) { return; }
		TM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.3f, 2.f, 1.f));
		TeslaOrb->SetMaterial(0, TM);
	}

	// === EAST COMPOUND — power station ===
	float EX = 80000.f;
	SpawnBuilding(FVector(EX, -3000.f, 0.f), FVector(5000.f, 8000.f, 3500.f));
	SpawnTower(FVector(EX + 5000.f, 3000.f, 0.f), 1000.f, 6000.f);
	SpawnTower(FVector(EX + 5000.f, -5000.f, 0.f), 1000.f, 6000.f);
	SpawnWall(FVector(EX - 4000.f, -8000.f, 0.f), FVector(EX - 4000.f, 8000.f, 0.f), 1200.f);
	SpawnRamp(FVector(EX + 3000.f, 0.f, 0.f), 2500.f, 3500.f, 600.f, 270.f);
	// Power pylons
	for (int32 i = 0; i < 4; i++)
	{
		float PylonY = -6000.f + i * 4000.f;
		// H-frame pylon
		SpawnStaticMesh(FVector(EX + 8000.f, PylonY, 2000.f),
			FVector(0.3f, 0.3f, 40.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
		SpawnStaticMesh(FVector(EX + 8400.f, PylonY, 2000.f),
			FVector(0.3f, 0.3f, 40.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
		SpawnStaticMesh(FVector(EX + 8200.f, PylonY, 3800.f),
			FVector(5.f, 0.3f, 0.15f), FRotator::ZeroRotator, CubeMesh,
			FLinearColor(0.12f, 0.12f, 0.14f));
	}
	// Cooling towers — with top rim and interior ring
	for (float TY : {5000.f, -5000.f})
	{
		FVector TPos(EX - 5000.f, TY, 1500.f);
		SpawnStaticMesh(TPos, FVector(15.f, 15.f, 30.f),
			FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.09f, 0.1f, 0.11f));
		// Top rim (wider lip)
		SpawnStaticMesh(TPos + FVector(0.f, 0.f, 1520.f),
			FVector(16.f, 16.f, 0.6f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.07f, 0.08f, 0.09f));
		// Interior ring (suggests internal structure)
		SpawnStaticMesh(TPos + FVector(0.f, 0.f, 1400.f),
			FVector(12.f, 12.f, 0.4f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.05f, 0.05f, 0.06f));
		// Base plinth
		SpawnStaticMesh(FVector(EX - 5000.f, TY, 30.f),
			FVector(16.f, 16.f, 0.6f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.06f, 0.065f, 0.07f));
	}
	// Reactor housing — glowing core visible between towers
	SpawnStaticMesh(FVector(EX - 5000.f, 0.f, 800.f),
		FVector(10.f, 10.f, 8.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.06f, 0.06f, 0.08f));
	UStaticMeshComponent* ReactorGlow = SpawnStaticMesh(
		FVector(EX - 5000.f, 0.f, 1250.f),
		FVector(6.f, 6.f, 0.15f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.1f, 0.3f, 0.8f));
	if (ReactorGlow)
	{
		UMaterialInterface* RxEmissive = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* RM = UMaterialInstanceDynamic::Create(RxEmissive, this);
		if (!RM) { return; }
		RM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.3f, 0.8f, 1.5f));
		ReactorGlow->SetMaterial(0, RM);
	}

	// === WEST COMPOUND — barracks ===
	float WX = -80000.f;
	for (int32 i = 0; i < 4; i++)
	{
		float OffsetY = (i - 1.5f) * 5000.f;
		SpawnBuilding(FVector(WX, OffsetY, 0.f), FVector(3000.f, 3500.f, 1800.f));
	}
	// Central parade ground platform
	SpawnPlatform(FVector(WX + 5000.f, 0.f, 50.f), FVector(6000.f, 12000.f, 50.f));
	SpawnWall(FVector(WX - 3000.f, -12000.f, 0.f), FVector(WX - 3000.f, 12000.f, 0.f), 1000.f);
	SpawnTower(FVector(WX - 3000.f, -12000.f, 0.f), 600.f, 4000.f);
	SpawnTower(FVector(WX - 3000.f, 12000.f, 0.f), 600.f, 4000.f);
	// Guard posts at entrance
	SpawnBuilding(FVector(WX + 4000.f, -6000.f, 0.f), FVector(1500.f, 1500.f, 2500.f));
	SpawnBuilding(FVector(WX + 4000.f, 6000.f, 0.f), FVector(1500.f, 1500.f, 2500.f));
	// Comms array landmark — tall mast with emissive signal dish
	SpawnStaticMesh(FVector(WX + 5000.f, 0.f, 2500.f),
		FVector(0.4f, 0.4f, 50.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.1f, 0.1f, 0.12f));
	// Dish (tilted flat cylinder)
	SpawnStaticMesh(FVector(WX + 5000.f, 0.f, 4800.f),
		FVector(5.f, 5.f, 0.15f), FRotator(30.f, 45.f, 0.f), CylinderMesh,
		FLinearColor(0.14f, 0.14f, 0.16f));
	// Signal emitter at dish center
	UStaticMeshComponent* SignalOrb = SpawnStaticMesh(
		FVector(WX + 5000.f, 0.f, 5000.f),
		FVector(0.8f, 0.8f, 0.8f), FRotator::ZeroRotator, SphereMesh,
		FLinearColor(0.8f, 0.2f, 0.2f));
	if (SignalOrb)
	{
		UMaterialInterface* SigEmissive = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* SigM = UMaterialInstanceDynamic::Create(SigEmissive, this);
		if (!SigM) { return; }
		SigM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.f, 0.3f, 0.3f));
		SignalOrb->SetMaterial(0, SigM);
	}

	// === KAYKIT COMPOUND SET DRESSING ===
	if (bHasKayKitAssets)
	{
		// Central Hub — landing pad, base modules, cargo
		if (KK_LandingPadLarge)
			SpawnRawMesh(FVector(8000.f, 0.f, 40.f), FVector(3.f), FRotator::ZeroRotator, KK_LandingPadLarge);
		if (KK_BaseModule)
		{
			SpawnRawMesh(FVector(-3000.f, 4000.f, 0.f), FVector(2.5f), FRotator(0.f, -90.f, 0.f), KK_BaseModule);
			SpawnRawMesh(FVector(-3000.f, -4000.f, 0.f), FVector(2.5f), FRotator(0.f, 90.f, 0.f), KK_BaseModule);
		}
		if (KK_Cargo)
			SpawnRawMesh(FVector(4000.f, 5000.f, 0.f), FVector(2.f), FRotator(0.f, 30.f, 0.f), KK_Cargo);
		if (KK_SolarPanel)
		{
			SpawnRawMesh(FVector(12000.f, 5000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_SolarPanel);
			SpawnRawMesh(FVector(12000.f, -5000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_SolarPanel);
		}

		// North Compound — industrial depot, drill, cargo
		if (KK_CargoDepot)
			SpawnRawMesh(FVector(10000.f, NY, 0.f), FVector(3.f), FRotator(0.f, 180.f, 0.f), KK_CargoDepot);
		if (KK_DrillStructure)
			SpawnRawMesh(FVector(-10000.f, NY + 6000.f, 0.f), FVector(3.f), FRotator::ZeroRotator, KK_DrillStructure);
		if (KK_Container)
		{
			SpawnRawMesh(FVector(-7000.f, NY - 2000.f, 0.f), FVector(2.f), FRotator(0.f, 15.f, 0.f), KK_Container);
			SpawnRawMesh(FVector(-4000.f, NY - 2000.f, 0.f), FVector(2.f), FRotator(0.f, 5.f, 0.f), KK_Container);
		}
		if (KK_Lights)
			SpawnRawMesh(FVector(0.f, NY + 3000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_Lights);

		// South Compound — research, structures, lander
		if (KK_StructureTall)
			SpawnRawMesh(FVector(-10000.f, SY, 0.f), FVector(3.f), FRotator(0.f, 45.f, 0.f), KK_StructureTall);
		if (KK_Lander)
			SpawnRawMesh(FVector(8000.f, SY + 3000.f, 0.f), FVector(2.5f), FRotator(0.f, -30.f, 0.f), KK_Lander);
		if (KK_RoofModule)
			SpawnRawMesh(FVector(3000.f, SY - 5000.f, 2200.f), FVector(2.5f), FRotator::ZeroRotator, KK_RoofModule);

		// East Compound — power station, wind turbines
		if (KK_WindTurbine)
		{
			SpawnRawMesh(FVector(EX + 10000.f, 8000.f, 0.f), FVector(3.f), FRotator::ZeroRotator, KK_WindTurbine);
			SpawnRawMesh(FVector(EX + 10000.f, -8000.f, 0.f), FVector(3.f), FRotator(0.f, 20.f, 0.f), KK_WindTurbine);
		}
		if (KK_StructureLow)
			SpawnRawMesh(FVector(EX - 8000.f, 6000.f, 0.f), FVector(2.5f), FRotator(0.f, 90.f, 0.f), KK_StructureLow);

		// West Compound — barracks, garage, space truck
		if (KK_BaseGarage)
			SpawnRawMesh(FVector(WX + 8000.f, -8000.f, 0.f), FVector(2.5f), FRotator(0.f, 0.f, 0.f), KK_BaseGarage);
		if (KK_SpaceTruck)
			SpawnRawMesh(FVector(WX + 8000.f, -4000.f, 0.f), FVector(2.f), FRotator(0.f, -15.f, 0.f), KK_SpaceTruck);

		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: KayKit compound set dressing placed"));
	}

	// === KENNEY SPACE KIT HALLWAYS & ROOMS ===
	if (bHasKenneyAssets)
	{
		// Central Hub — corridor hallways leading outward from main building
		if (KN_CorridorWide)
		{
			SpawnRawMesh(FVector(0.f, 6000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_CorridorWide);
			SpawnRawMesh(FVector(0.f, 8000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_CorridorWide);
		}
		if (KN_CorridorWideCorner)
			SpawnRawMesh(FVector(2000.f, 10000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_CorridorWideCorner);
		if (KN_RoomLarge)
			SpawnRawMesh(FVector(-6000.f, -6000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_RoomLarge);
		if (KN_RoomSmall)
			SpawnRawMesh(FVector(6000.f, -6000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_RoomSmall);

		// North Compound — industrial corridors
		if (KN_Corridor)
		{
			SpawnRawMesh(FVector(0.f, NY - 2000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_Corridor);
			SpawnRawMesh(FVector(0.f, NY - 4000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_Corridor);
		}
		if (KN_CorridorIntersection)
			SpawnRawMesh(FVector(0.f, NY, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_CorridorIntersection);

		// East Compound — gate entries
		if (KN_Gate)
			SpawnRawMesh(FVector(EX - 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_Gate);
		if (KN_GateDoor)
			SpawnRawMesh(FVector(EX - 4000.f, 4000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_GateDoor);

		// West Compound — stairs between barracks levels
		if (KN_StairsWide)
			SpawnRawMesh(FVector(WX + 3000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_StairsWide);
		if (KN_Stairs)
			SpawnRawMesh(FVector(WX + 3000.f, -5000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_Stairs);

		// Cables at North industrial areas
		if (KN_Cables)
		{
			SpawnRawMesh(FVector(-3000.f, NY + 3000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), KN_Cables);
			SpawnRawMesh(FVector(5000.f, NY + 3000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_Cables);
		}

		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Kenney SpaceKit corridors and rooms placed"));
	}

	// === QUATERNIUS SCI-FI WALLS & COLUMNS ===
	if (bHasQuaterniusAssets)
	{
		// Central Hub — columns at building corners, walls on facades
		if (QT_Column1)
		{
			SpawnRawMesh(FVector(4000.f, 3000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column1);
			SpawnRawMesh(FVector(-4000.f, 3000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column1);
			SpawnRawMesh(FVector(4000.f, -3000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column1);
			SpawnRawMesh(FVector(-4000.f, -3000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column1);
		}
		if (QT_Wall1)
		{
			SpawnRawMesh(FVector(4050.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, 90.f, 0.f), QT_Wall1);
			SpawnRawMesh(FVector(-4050.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, -90.f, 0.f), QT_Wall1);
		}
		if (QT_WindowWall)
		{
			SpawnRawMesh(FVector(0.f, 3050.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_WindowWall);
			SpawnRawMesh(FVector(0.f, -3050.f, 0.f), FVector(2.f), FRotator(0.f, 180.f, 0.f), QT_WindowWall);
		}

		// North Compound — industrial walls
		if (QT_Wall3)
		{
			SpawnRawMesh(FVector(-5000.f, NY + 2000.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_Wall3);
			SpawnRawMesh(FVector(5000.f, NY + 2500.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_Wall3);
		}
		if (QT_Column2)
		{
			SpawnRawMesh(FVector(-8000.f, NY + 4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column2);
			SpawnRawMesh(FVector(8000.f, NY + 4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column2);
		}

		// South Compound — research lab columns
		if (QT_ColumnSlim)
		{
			SpawnRawMesh(FVector(6000.f, SY + 3500.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_ColumnSlim);
			SpawnRawMesh(FVector(-6000.f, SY + 3500.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_ColumnSlim);
			SpawnRawMesh(FVector(6000.f, SY - 3500.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_ColumnSlim);
			SpawnRawMesh(FVector(-6000.f, SY - 3500.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_ColumnSlim);
		}

		// West Compound — barracks entrance walls
		if (QT_Wall1)
		{
			SpawnRawMesh(FVector(WX + 4000.f, -3000.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_Wall1);
			SpawnRawMesh(FVector(WX + 4000.f, 3000.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_Wall1);
		}

		// Quaternius staircase at East power station
		if (QT_Staircase)
			SpawnRawMesh(FVector(EX + 3000.f, 3000.f, 0.f), FVector(2.f), FRotator(0.f, -90.f, 0.f), QT_Staircase);

		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Quaternius SciFi walls and columns placed"));
	}

	// === SCIFI DOOR at compound entrances ===
	if (bHasSciFiDoorAsset && SF_Door)
	{
		SpawnRawMesh(FVector(0.f, NY - 4000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), SF_Door);   // North gate
		SpawnRawMesh(FVector(0.f, SY + 5000.f, 0.f), FVector(1.5f), FRotator(0.f, 0.f, 0.f), SF_Door);   // South gate
		SpawnRawMesh(FVector(EX - 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), SF_Door);  // East gate
		SpawnRawMesh(FVector(WX + 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), SF_Door);  // West gate
		// Central hub main entrance
		SpawnRawMesh(FVector(0.f, 3100.f, 0.f), FVector(1.8f), FRotator(0.f, 0.f, 0.f), SF_Door);
		SpawnRawMesh(FVector(0.f, -3100.f, 0.f), FVector(1.8f), FRotator(0.f, 180.f, 0.f), SF_Door);
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: SciFi Door meshes placed at compound entrances"));
	}

	// === QUATERNIUS DOORS as Kenney/SciFiDoor fallback ===
	if (bHasQuaterniusAssets && !bHasSciFiDoorAsset && QT_DoorSingle)
	{
		SpawnRawMesh(FVector(0.f, NY - 4000.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_DoorSingle);
		SpawnRawMesh(FVector(0.f, SY + 5000.f, 0.f), FVector(2.f), FRotator(0.f, 0.f, 0.f), QT_DoorSingle);
		SpawnRawMesh(FVector(EX - 4000.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, 90.f, 0.f), QT_DoorSingle);
		SpawnRawMesh(FVector(WX + 4000.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, 90.f, 0.f), QT_DoorSingle);
	}

	// === COMPOUND ENTRANCE GATES — archways marking facility entry points ===
	struct FGate { FVector Pos; float Yaw; FLinearColor Color; };
	TArray<FGate> Gates = {
		{{0.f, NY - 4000.f, 0.f}, 0.f, {1.2f, 0.6f, 0.12f}},       // North — amber
		{{0.f, SY + 5000.f, 0.f}, 0.f, {0.15f, 1.0f, 0.7f}},       // South — teal
		{{EX - 4000.f, 0.f, 0.f}, 90.f, {1.2f, 0.25f, 0.15f}},     // East — red
		{{WX + 4000.f, 0.f, 0.f}, 90.f, {0.2f, 1.0f, 0.3f}},       // West — green
	};
	UMaterialInterface* GateEmissive = FExoMaterialFactory::GetEmissiveOpaque();
	for (const auto& G : Gates)
	{
		FRotator GRot(0.f, G.Yaw, 0.f);
		FLinearColor GateMetal(0.09f, 0.1f, 0.12f);
		float GateW = 3000.f;
		float GateH = 2000.f;
		float PillarW = 200.f;
		// Left pillar
		SpawnStaticMesh(G.Pos + GRot.RotateVector(FVector(-GateW * 0.5f, 0.f, GateH * 0.5f)),
			FVector(PillarW / 100.f, PillarW / 100.f, GateH / 100.f), GRot, CubeMesh, GateMetal);
		// Right pillar
		SpawnStaticMesh(G.Pos + GRot.RotateVector(FVector(GateW * 0.5f, 0.f, GateH * 0.5f)),
			FVector(PillarW / 100.f, PillarW / 100.f, GateH / 100.f), GRot, CubeMesh, GateMetal);
		// Crossbeam (lintel)
		SpawnStaticMesh(G.Pos + FVector(0.f, 0.f, GateH),
			FVector(GateW / 100.f + 1.f, PillarW / 100.f, 1.2f), GRot, CubeMesh,
			FLinearColor(0.11f, 0.12f, 0.14f));
		// Emissive strip on lintel underside
		UStaticMeshComponent* GStrip = SpawnStaticMesh(
			G.Pos + FVector(0.f, 0.f, GateH - 15.f),
			FVector(GateW / 100.f * 0.8f, 0.03f, 0.06f), GRot, CubeMesh,
			FLinearColor(G.Color.R * 0.1f, G.Color.G * 0.1f, G.Color.B * 0.1f));
		if (GStrip && GateEmissive)
		{
			UMaterialInstanceDynamic* GM = UMaterialInstanceDynamic::Create(GateEmissive, this);
			if (!GM) { return; }
			GM->SetVectorParameterValue(TEXT("EmissiveColor"), G.Color);
			GStrip->SetMaterial(0, GM);
		}
	}

	// === CORNER OUTPOSTS ===
	float CornerDist = 120000.f;
	FVector Corners[] = {
		{CornerDist, CornerDist, 0.f}, {-CornerDist, CornerDist, 0.f},
		{CornerDist, -CornerDist, 0.f}, {-CornerDist, -CornerDist, 0.f}
	};
	for (const FVector& C : Corners)
	{
		SpawnBuilding(C, FVector(4000.f, 4000.f, 2000.f), FMath::RandRange(0.f, 45.f));
		SpawnTower(C + FVector(3000.f, 3000.f, 0.f), 500.f, 3500.f);
		SpawnBuilding(C + FVector(-2500.f, 1500.f, 0.f), FVector(2000.f, 2000.f, 1200.f));
		// KayKit outpost dressing
		if (bHasKayKitAssets && KK_StructureLow)
			SpawnRawMesh(C + FVector(-4000.f, -3000.f, 0.f), FVector(2.f), FRotator(0.f, 45.f, 0.f), KK_StructureLow);
		if (bHasKayKitAssets && KK_SolarPanel)
			SpawnRawMesh(C + FVector(5000.f, -2000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, KK_SolarPanel);
	}

	// === SCATTERED STRUCTURES across the map ===
	struct FBldg { FVector Pos; FVector Size; float Rot; };
	TArray<FBldg> Scattered = {
		{{40000.f, 40000.f, 0.f},   {4000.f, 3000.f, 2000.f}, 30.f},
		{{-50000.f, 50000.f, 0.f},  {3000.f, 5000.f, 1500.f}, -20.f},
		{{60000.f, -40000.f, 0.f},  {3500.f, 3500.f, 2200.f}, 45.f},
		{{-30000.f, -50000.f, 0.f}, {5000.f, 3000.f, 1800.f}, 10.f},
		{{20000.f, -70000.f, 0.f},  {4000.f, 4000.f, 2500.f}, 0.f},
		{{-70000.f, -20000.f, 0.f}, {3000.f, 6000.f, 2000.f}, -15.f},
		{{50000.f, 70000.f, 0.f},   {3500.f, 4000.f, 1600.f}, 60.f},
		{{-100000.f, -80000.f, 0.f},{4500.f, 3500.f, 2400.f}, 25.f},
		{{90000.f, 50000.f, 0.f},   {3000.f, 3000.f, 3000.f}, 0.f},
		{{-20000.f, 120000.f, 0.f}, {5000.f, 4000.f, 2000.f}, -35.f},
		// Additional structures filling open field gaps
		{{25000.f, -25000.f, 0.f},  {2500.f, 2500.f, 1600.f}, 15.f},
		{{-25000.f, 25000.f, 0.f},  {2000.f, 3000.f, 1400.f}, -40.f},
		{{55000.f, -70000.f, 0.f},  {3000.f, 2500.f, 1800.f}, 70.f},
		{{-60000.f, 45000.f, 0.f},  {2500.f, 3500.f, 2000.f}, -25.f},
		{{70000.f, 40000.f, 0.f},   {3500.f, 2500.f, 2200.f}, 5.f},
		{{-45000.f, -30000.f, 0.f}, {2000.f, 2000.f, 1200.f}, 50.f},
		// Small shelters along major routes
		{{15000.f, 40000.f, 0.f},   {1500.f, 1500.f, 1000.f}, 0.f},
		{{40000.f, -15000.f, 0.f},  {1500.f, 2000.f, 1200.f}, 90.f},
		{{-40000.f, -15000.f, 0.f}, {2000.f, 1500.f, 1000.f}, -10.f},
		{{-15000.f, -40000.f, 0.f}, {1500.f, 1500.f, 1200.f}, 45.f},
	};
	for (const auto& B : Scattered)
	{
		SpawnBuilding(B.Pos, B.Size, B.Rot);
	}

	// === RUINS — partially destroyed structures for cover and atmosphere ===
	FLinearColor RuinColor(0.05f, 0.055f, 0.065f);
	struct FRuin { FVector Pos; float Yaw; };
	TArray<FRuin> Ruins = {
		{{30000.f, 10000.f, 0.f}, 20.f},
		{{-20000.f, -15000.f, 0.f}, -35.f},
		{{50000.f, -25000.f, 0.f}, 60.f},
		{{-40000.f, 70000.f, 0.f}, 10.f},
		{{70000.f, -60000.f, 0.f}, -50.f},
		{{-60000.f, -60000.f, 0.f}, 140.f},
		{{10000.f, -50000.f, 0.f}, 75.f},
		{{-50000.f, 10000.f, 0.f}, -20.f},
	};
	for (const auto& R : Ruins)
	{
		FRotator RRot(0.f, R.Yaw, 0.f);
		// Broken wall — irregular top (jagged from damage)
		SpawnStaticMesh(R.Pos + FVector(0.f, 0.f, 400.f),
			FVector(20.f, 0.8f, 8.f), RRot, CubeMesh, RuinColor);
		// Shorter wall section at different angle (collapsed wing)
		SpawnStaticMesh(R.Pos + RRot.RotateVector(FVector(800.f, 0.f, 250.f)),
			FVector(12.f, 0.8f, 5.f),
			FRotator(0.f, R.Yaw + 90.f, 0.f), CubeMesh, RuinColor);
		// Toppled column
		SpawnStaticMesh(R.Pos + RRot.RotateVector(FVector(-600.f, 300.f, 50.f)),
			FVector(0.5f, 0.5f, 6.f), FRotator(80.f, R.Yaw + 20.f, 0.f),
			CylinderMesh, FLinearColor(0.06f, 0.065f, 0.075f));
		// Rubble pile — multiple overlapping chunks
		SpawnStaticMesh(R.Pos + FVector(200.f, 200.f, 80.f),
			FVector(4.f, 3.f, 1.6f),
			FRotator(8.f, R.Yaw + 15.f, 5.f), CubeMesh, RuinColor);
		SpawnStaticMesh(R.Pos + FVector(-100.f, 400.f, 40.f),
			FVector(2.5f, 2.f, 1.f),
			FRotator(15.f, R.Yaw + 50.f, -8.f), CubeMesh, RuinColor);
		// Exposed rebar/beam (thin bright metal strip)
		SpawnStaticMesh(R.Pos + FVector(0.f, 0.f, 750.f),
			FVector(6.f, 0.08f, 0.08f),
			FRotator(-10.f, R.Yaw + 5.f, 0.f), CubeMesh,
			FLinearColor(0.15f, 0.12f, 0.1f));
	}
}

// SpawnTower, SpawnWall, SpawnPlatform, SpawnRamp → ExoLevelBuilderPrimitives.cpp
