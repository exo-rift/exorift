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
	const float NY = NorthY, SY = SouthY, EX = EastX, WX = WestX;

	// === CENTRAL HUB ===
	SpawnBuilding(FVector(0.f, 0.f, 0.f), FVector(8000.f, 6000.f, 2500.f));
	SpawnBuilding(FVector(0.f, 0.f, 2500.f), FVector(4000.f, 3000.f, 1500.f));
	SpawnPlatform(FVector(5000.f, 0.f, 1500.f), FVector(3000.f, 2000.f, 100.f));
	SpawnRamp(FVector(6500.f, 0.f, 0.f), 2000.f, 1500.f, 500.f, 180.f);
	SpawnPlatform(FVector(-2000.f, 0.f, 1200.f), FVector(2000.f, 1500.f, 80.f));
	SpawnRamp(FVector(-3500.f, 0.f, 0.f), 1500.f, 1200.f, 500.f, 0.f);
	SpawnTower(FVector(0.f, 0.f, 4000.f), 400.f, 2000.f);
	SpawnAntenna(FVector(0.f, 0.f, 6000.f), 2000.f);
	// Landing pad
	FVector PC(8000.f, 0.f, 30.f);
	SpawnStaticMesh(PC, FVector(30.f, 30.f, 0.3f), FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.1f, 0.11f));
	SpawnStaticMesh(PC + FVector(0.f, 1500.f, 5.f), FVector(30.f, 0.1f, 0.05f), FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.4f, 0.1f));
	SpawnStaticMesh(PC + FVector(0.f, -1500.f, 5.f), FVector(30.f, 0.1f, 0.05f), FRotator::ZeroRotator, CubeMesh, FLinearColor(0.1f, 0.4f, 0.1f));

	// === NORTH COMPOUND — industrial ===
	SpawnBuilding(FVector(-5000.f, NY, 0.f), FVector(6000.f, 4000.f, 3000.f), 15.f);
	SpawnBuilding(FVector(5000.f, NY, 0.f), FVector(4000.f, 5000.f, 2000.f));
	SpawnPlatform(FVector(0.f, NY, 4000.f), FVector(12000.f, 800.f, 200.f));
	SpawnWall(FVector(-10000.f, NY - 4000.f, 0.f), FVector(10000.f, NY - 4000.f, 0.f), 1500.f);
	SpawnPlatform(FVector(0.f, NY - 4000.f, 1500.f), FVector(20000.f, 600.f, 80.f));
	SpawnTower(FVector(0.f, NY + 5000.f, 0.f), 800.f, 5000.f);
	for (int32 i = 0; i < 3; i++)
	{
		float TX = -8000.f + i * 4000.f, TY = NY + 6000.f;
		SpawnStaticMesh(FVector(TX, TY, 1500.f), FVector(10.f, 10.f, 30.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.1f, 0.1f, 0.12f));
		SpawnStaticMesh(FVector(TX, TY, 3050.f), FVector(10.f, 10.f, 3.f), FRotator::ZeroRotator, SphereMesh, FLinearColor(0.09f, 0.09f, 0.11f));
	}
	// Smokestack landmark
	SpawnStaticMesh(FVector(8000.f, NY + 5000.f, 2500.f), FVector(3.f, 3.f, 50.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.08f, 0.08f, 0.1f));
	UStaticMeshComponent* SmokeCap = SpawnStaticMesh(FVector(8000.f, NY + 5000.f, 5050.f), FVector(3.5f, 3.5f, 0.1f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.6f, 0.3f, 0.05f));
	if (SmokeCap) { auto* CM = UMaterialInstanceDynamic::Create(FExoMaterialFactory::GetEmissiveOpaque(), this); if (!CM) return; CM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.f, 0.8f, 0.15f)); SmokeCap->SetMaterial(0, CM); }

	// === SOUTH COMPOUND — research ===
	SpawnBuilding(FVector(3000.f, SY, 0.f), FVector(5000.f, 7000.f, 2200.f), -10.f);
	SpawnBuilding(FVector(-6000.f, SY + 2000.f, 0.f), FVector(3500.f, 3500.f, 1800.f));
	SpawnBuilding(FVector(-6000.f, SY - 3000.f, 0.f), FVector(3500.f, 3000.f, 2000.f));
	SpawnPlatform(FVector(-1000.f, SY, 2500.f), FVector(5000.f, 1500.f, 100.f));
	UStaticMeshComponent* Dome = SpawnStaticMesh(FVector(3000.f, SY, 2200.f), FVector(25.f, 25.f, 12.f), FRotator::ZeroRotator, SphereMesh, FLinearColor(0.02f, 0.04f, 0.06f));
	if (Dome) { auto* DM = UMaterialInstanceDynamic::Create(FExoMaterialFactory::GetGlassTranslucent(), this); if (!DM) return; DM->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.01f, 0.03f, 0.05f)); DM->SetScalarParameterValue(TEXT("Metallic"), 0.15f); DM->SetScalarParameterValue(TEXT("Roughness"), 0.05f); DM->SetScalarParameterValue(TEXT("Opacity"), 0.25f); DM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.02f, 0.12f, 0.08f)); Dome->SetMaterial(0, DM); }
	for (float Yaw : {0.f, 45.f, 90.f, 135.f})
		SpawnStaticMesh(FVector(3000.f, SY, 2200.f), FVector(25.2f, 0.12f, 12.2f), FRotator(0.f, Yaw, 0.f), SphereMesh, FLinearColor(0.06f, 0.065f, 0.08f));
	SpawnWall(FVector(-9000.f, SY - 5000.f, 0.f), FVector(-9000.f, SY + 5000.f, 0.f), 1800.f);
	SpawnWall(FVector(-9000.f, SY + 5000.f, 0.f), FVector(9000.f, SY + 5000.f, 0.f), 1800.f);
	// Tesla coil landmark
	SpawnStaticMesh(FVector(-6000.f, SY, 1800.f), FVector(0.4f, 0.4f, 36.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.15f));
	UStaticMeshComponent* TeslaOrb = SpawnStaticMesh(FVector(-6000.f, SY, 3650.f), FVector(2.f, 2.f, 2.f), FRotator::ZeroRotator, SphereMesh, FLinearColor(0.1f, 0.6f, 0.3f));
	if (TeslaOrb) { auto* TM = UMaterialInstanceDynamic::Create(FExoMaterialFactory::GetEmissiveOpaque(), this); if (!TM) return; TM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.3f, 2.f, 1.f)); TeslaOrb->SetMaterial(0, TM); }

	// === EAST COMPOUND — power station ===
	SpawnBuilding(FVector(EX, -3000.f, 0.f), FVector(5000.f, 8000.f, 3500.f));
	SpawnTower(FVector(EX + 5000.f, 3000.f, 0.f), 1000.f, 6000.f);
	SpawnTower(FVector(EX + 5000.f, -5000.f, 0.f), 1000.f, 6000.f);
	SpawnWall(FVector(EX - 4000.f, -8000.f, 0.f), FVector(EX - 4000.f, 8000.f, 0.f), 1200.f);
	SpawnRamp(FVector(EX + 3000.f, 0.f, 0.f), 2500.f, 3500.f, 600.f, 270.f);
	for (int32 i = 0; i < 4; i++)
	{
		float PY = -6000.f + i * 4000.f;
		SpawnStaticMesh(FVector(EX + 8000.f, PY, 2000.f), FVector(0.3f, 0.3f, 40.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
		SpawnStaticMesh(FVector(EX + 8400.f, PY, 2000.f), FVector(0.3f, 0.3f, 40.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.12f, 0.12f, 0.14f));
	}
	for (float TY : {5000.f, -5000.f})
	{
		SpawnStaticMesh(FVector(EX - 5000.f, TY, 1500.f), FVector(15.f, 15.f, 30.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.09f, 0.1f, 0.11f));
		SpawnStaticMesh(FVector(EX - 5000.f, TY, 3020.f), FVector(16.f, 16.f, 0.6f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.07f, 0.08f, 0.09f));
	}
	// Reactor housing
	SpawnStaticMesh(FVector(EX - 5000.f, 0.f, 800.f), FVector(10.f, 10.f, 8.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.06f, 0.06f, 0.08f));
	UStaticMeshComponent* RxGlow = SpawnStaticMesh(FVector(EX - 5000.f, 0.f, 1250.f), FVector(6.f, 6.f, 0.15f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.1f, 0.3f, 0.8f));
	if (RxGlow) { auto* RM = UMaterialInstanceDynamic::Create(FExoMaterialFactory::GetEmissiveOpaque(), this); if (!RM) return; RM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.3f, 0.8f, 1.5f)); RxGlow->SetMaterial(0, RM); }

	// === WEST COMPOUND — barracks ===
	for (int32 i = 0; i < 4; i++)
		SpawnBuilding(FVector(WX, (i - 1.5f) * 5000.f, 0.f), FVector(3000.f, 3500.f, 1800.f));
	SpawnPlatform(FVector(WX + 5000.f, 0.f, 50.f), FVector(6000.f, 12000.f, 50.f));
	SpawnWall(FVector(WX - 3000.f, -12000.f, 0.f), FVector(WX - 3000.f, 12000.f, 0.f), 1000.f);
	SpawnTower(FVector(WX - 3000.f, -12000.f, 0.f), 600.f, 4000.f);
	SpawnTower(FVector(WX - 3000.f, 12000.f, 0.f), 600.f, 4000.f);
	SpawnBuilding(FVector(WX + 4000.f, -6000.f, 0.f), FVector(1500.f, 1500.f, 2500.f));
	SpawnBuilding(FVector(WX + 4000.f, 6000.f, 0.f), FVector(1500.f, 1500.f, 2500.f));
	// Comms array landmark
	SpawnStaticMesh(FVector(WX + 5000.f, 0.f, 2500.f), FVector(0.4f, 0.4f, 50.f), FRotator::ZeroRotator, CylinderMesh, FLinearColor(0.1f, 0.1f, 0.12f));
	SpawnStaticMesh(FVector(WX + 5000.f, 0.f, 4800.f), FVector(5.f, 5.f, 0.15f), FRotator(30.f, 45.f, 0.f), CylinderMesh, FLinearColor(0.14f, 0.14f, 0.16f));
	UStaticMeshComponent* SigOrb = SpawnStaticMesh(FVector(WX + 5000.f, 0.f, 5000.f), FVector(0.8f, 0.8f, 0.8f), FRotator::ZeroRotator, SphereMesh, FLinearColor(0.8f, 0.2f, 0.2f));
	if (SigOrb) { auto* SM = UMaterialInstanceDynamic::Create(FExoMaterialFactory::GetEmissiveOpaque(), this); if (!SM) return; SM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.f, 0.3f, 0.3f)); SigOrb->SetMaterial(0, SM); }

	// === KAYKIT SET DRESSING (doubled density) ===
	if (bHasKayKitAssets)
	{
		if (KK_LandingPadLarge) SpawnRawMesh(FVector(8000.f, 0.f, 40.f), FVector(3.f), FRotator::ZeroRotator, KK_LandingPadLarge);
		if (KK_BaseModule) { SpawnRawMesh(FVector(-3000.f, 4000.f, 0.f), FVector(2.5f), FRotator(0.f, -90.f, 0.f), KK_BaseModule); SpawnRawMesh(FVector(-3000.f, -4000.f, 0.f), FVector(2.5f), FRotator(0.f, 90.f, 0.f), KK_BaseModule); SpawnRawMesh(FVector(4000.f, -4000.f, 0.f), FVector(2.5f), FRotator(0.f, 180.f, 0.f), KK_BaseModule); }
		if (KK_Cargo) { SpawnRawMesh(FVector(4000.f, 5000.f, 0.f), FVector(2.f), FRotator(0.f, 30.f, 0.f), KK_Cargo); SpawnRawMesh(FVector(2000.f, 8000.f, 0.f), FVector(2.f), FRotator(0.f, -15.f, 0.f), KK_Cargo); SpawnRawMesh(FVector(8000.f, -4000.f, 0.f), FVector(1.8f), FRotator(0.f, 60.f, 0.f), KK_Cargo); SpawnRawMesh(FVector(-8000.f, 3000.f, 0.f), FVector(1.8f), FRotator(0.f, -40.f, 0.f), KK_Cargo); }
		if (KK_SolarPanel) { SpawnRawMesh(FVector(12000.f, 5000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_SolarPanel); SpawnRawMesh(FVector(12000.f, -5000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_SolarPanel); }
		if (KK_Container) {
			SpawnRawMesh(FVector(-7000.f, NY - 2000.f, 0.f), FVector(2.f), FRotator(0.f, 15.f, 0.f), KK_Container); SpawnRawMesh(FVector(-4000.f, NY - 2000.f, 0.f), FVector(2.f), FRotator(0.f, 5.f, 0.f), KK_Container);
			SpawnRawMesh(FVector(-2000.f, NY + 3000.f, 0.f), FVector(2.f), FRotator(0.f, -10.f, 0.f), KK_Container); SpawnRawMesh(FVector(7000.f, NY + 2000.f, 0.f), FVector(2.f), FRotator(0.f, 25.f, 0.f), KK_Container);
			SpawnRawMesh(FVector(6000.f, SY - 2000.f, 0.f), FVector(2.f), FRotator(0.f, 45.f, 0.f), KK_Container); SpawnRawMesh(FVector(-3000.f, SY + 4000.f, 0.f), FVector(2.f), FRotator(0.f, -20.f, 0.f), KK_Container);
			SpawnRawMesh(FVector(EX + 3000.f, -6000.f, 0.f), FVector(2.f), FRotator(0.f, 70.f, 0.f), KK_Container); SpawnRawMesh(FVector(EX - 2000.f, 5000.f, 0.f), FVector(2.f), FRotator(0.f, -5.f, 0.f), KK_Container);
			SpawnRawMesh(FVector(WX + 6000.f, -3000.f, 0.f), FVector(2.f), FRotator(0.f, 30.f, 0.f), KK_Container); SpawnRawMesh(FVector(WX + 2000.f, 8000.f, 0.f), FVector(2.f), FRotator(0.f, -35.f, 0.f), KK_Container);
		}
		if (KK_CargoDepot) SpawnRawMesh(FVector(10000.f, NY, 0.f), FVector(3.f), FRotator(0.f, 180.f, 0.f), KK_CargoDepot);
		if (KK_DrillStructure) SpawnRawMesh(FVector(-10000.f, NY + 6000.f, 0.f), FVector(3.f), FRotator::ZeroRotator, KK_DrillStructure);
		if (KK_Lights) { SpawnRawMesh(FVector(0.f, NY + 3000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_Lights); SpawnRawMesh(FVector(EX + 2000.f, 0.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_Lights); }
		if (KK_StructureTall) SpawnRawMesh(FVector(-10000.f, SY, 0.f), FVector(3.f), FRotator(0.f, 45.f, 0.f), KK_StructureTall);
		if (KK_Lander) SpawnRawMesh(FVector(8000.f, SY + 3000.f, 0.f), FVector(2.5f), FRotator(0.f, -30.f, 0.f), KK_Lander);
		if (KK_RoofModule) SpawnRawMesh(FVector(3000.f, SY - 5000.f, 2200.f), FVector(2.5f), FRotator::ZeroRotator, KK_RoofModule);
		if (KK_WindTurbine) { SpawnRawMesh(FVector(EX + 10000.f, 8000.f, 0.f), FVector(3.f), FRotator::ZeroRotator, KK_WindTurbine); SpawnRawMesh(FVector(EX + 10000.f, -8000.f, 0.f), FVector(3.f), FRotator(0.f, 20.f, 0.f), KK_WindTurbine); SpawnRawMesh(FVector(EX + 6000.f, 0.f, 0.f), FVector(2.5f), FRotator(0.f, -10.f, 0.f), KK_WindTurbine); }
		if (KK_StructureLow) { SpawnRawMesh(FVector(EX - 8000.f, 6000.f, 0.f), FVector(2.5f), FRotator(0.f, 90.f, 0.f), KK_StructureLow); SpawnRawMesh(FVector(EX - 6000.f, -6000.f, 0.f), FVector(2.5f), FRotator(0.f, -45.f, 0.f), KK_StructureLow); }
		if (KK_BaseGarage) SpawnRawMesh(FVector(WX + 8000.f, -8000.f, 0.f), FVector(2.5f), FRotator::ZeroRotator, KK_BaseGarage);
		if (KK_SpaceTruck) { SpawnRawMesh(FVector(WX + 8000.f, -4000.f, 0.f), FVector(2.f), FRotator(0.f, -15.f, 0.f), KK_SpaceTruck); SpawnRawMesh(FVector(WX + 6000.f, 6000.f, 0.f), FVector(2.f), FRotator(0.f, 45.f, 0.f), KK_SpaceTruck); }
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: KayKit compound set dressing placed"));
	}

	// === KENNEY CORRIDORS + INTER-COMPOUND CONNECTIONS ===
	if (bHasKenneyAssets)
	{
		if (KN_CorridorWide) { SpawnRawMesh(FVector(0.f, 6000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_CorridorWide); SpawnRawMesh(FVector(0.f, -6000.f, 0.f), FVector(1.5f), FRotator(0.f, 180.f, 0.f), KN_CorridorWide); SpawnRawMesh(FVector(6000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_CorridorWide); SpawnRawMesh(FVector(-6000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, -90.f, 0.f), KN_CorridorWide); }
		if (KN_RoomLarge) { SpawnRawMesh(FVector(-6000.f, -6000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_RoomLarge); SpawnRawMesh(FVector(6000.f, 6000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_RoomLarge); }
		if (KN_RoomSmall) { SpawnRawMesh(FVector(6000.f, -6000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_RoomSmall); SpawnRawMesh(FVector(-6000.f, 6000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_RoomSmall); }
		// Hub→N/S/E/W connecting corridors (buildings are close, so 3 segments each)
		if (KN_Corridor) {
			for (float D : {8000.f, 10000.f, 12000.f}) { SpawnRawMesh(FVector(0.f, D, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_Corridor); SpawnRawMesh(FVector(0.f, -D, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_Corridor); }
			for (float D : {8000.f, 10000.f, 12000.f}) { SpawnRawMesh(FVector(D, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_Corridor); SpawnRawMesh(FVector(-D, 0.f, 0.f), FVector(1.5f), FRotator(0.f, -90.f, 0.f), KN_Corridor); }
		}
		if (KN_CorridorIntersection) { SpawnRawMesh(FVector(0.f, NY, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_CorridorIntersection); SpawnRawMesh(FVector(EX, 0.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_CorridorIntersection); }
		if (KN_Gate) { SpawnRawMesh(FVector(EX - 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_Gate); SpawnRawMesh(FVector(WX + 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, -90.f, 0.f), KN_Gate); }
		if (KN_GateDoor) { SpawnRawMesh(FVector(EX - 4000.f, 4000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_GateDoor); SpawnRawMesh(FVector(0.f, NY - 4000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_GateDoor); }
		if (KN_GateLasers) { SpawnRawMesh(FVector(0.f, NY - 6000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_GateLasers); SpawnRawMesh(FVector(0.f, SY + 6000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_GateLasers); SpawnRawMesh(FVector(EX - 6000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_GateLasers); SpawnRawMesh(FVector(WX + 6000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_GateLasers); }
		if (KN_StairsWide) SpawnRawMesh(FVector(WX + 3000.f, 0.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_StairsWide);
		if (KN_Stairs) { SpawnRawMesh(FVector(WX + 3000.f, -5000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_Stairs); SpawnRawMesh(FVector(EX + 3000.f, -3000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_Stairs); }
		if (KN_Cables) { SpawnRawMesh(FVector(-3000.f, NY + 3000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, KN_Cables); SpawnRawMesh(FVector(5000.f, NY + 3000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), KN_Cables); SpawnRawMesh(FVector(EX + 2000.f, -4000.f, 0.f), FVector(1.5f), FRotator(0.f, 45.f, 0.f), KN_Cables); }
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Kenney corridors/rooms/connections placed"));
	}

	// === QUATERNIUS WALLS, COLUMNS, INTERIOR PROPS (doubled) ===
	if (bHasQuaterniusAssets)
	{
		if (QT_Column1) { for (float CX : {4000.f, -4000.f}) for (float CY : {3000.f, -3000.f}) SpawnRawMesh(FVector(CX, CY, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column1); }
		if (QT_Wall1) { SpawnRawMesh(FVector(4050.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, 90.f, 0.f), QT_Wall1); SpawnRawMesh(FVector(-4050.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, -90.f, 0.f), QT_Wall1); SpawnRawMesh(FVector(WX + 4000.f, -3000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Wall1); SpawnRawMesh(FVector(WX + 4000.f, 3000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Wall1); }
		if (QT_WindowWall) { SpawnRawMesh(FVector(0.f, 3050.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_WindowWall); SpawnRawMesh(FVector(0.f, -3050.f, 0.f), FVector(2.f), FRotator(0.f, 180.f, 0.f), QT_WindowWall); }
		if (QT_Wall3) { SpawnRawMesh(FVector(-5000.f, NY + 2000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Wall3); SpawnRawMesh(FVector(5000.f, NY + 2500.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Wall3); }
		if (QT_Column2) { SpawnRawMesh(FVector(-8000.f, NY + 4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column2); SpawnRawMesh(FVector(8000.f, NY + 4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column2); SpawnRawMesh(FVector(EX + 4000.f, -4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Column2); }
		if (QT_ColumnSlim) { for (float CX : {6000.f, -6000.f}) for (float CY : {SY + 3500.f, SY - 3500.f}) SpawnRawMesh(FVector(CX, CY, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_ColumnSlim); }
		if (QT_Staircase) { SpawnRawMesh(FVector(EX + 3000.f, 3000.f, 0.f), FVector(2.f), FRotator(0.f, -90.f, 0.f), QT_Staircase); SpawnRawMesh(FVector(-3000.f, SY + 4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_Staircase); }
		// Interior props — crates, computers, shelves
		if (QT_PropsCrate) { SpawnRawMesh(FVector(-2000.f, NY + 1000.f, 0.f), FVector(1.5f), FRotator(0.f, 10.f, 0.f), QT_PropsCrate); SpawnRawMesh(FVector(3000.f, NY - 1000.f, 0.f), FVector(1.5f), FRotator(0.f, -25.f, 0.f), QT_PropsCrate); SpawnRawMesh(FVector(EX + 1000.f, -2000.f, 0.f), FVector(1.5f), FRotator(0.f, 45.f, 0.f), QT_PropsCrate); SpawnRawMesh(FVector(WX + 1000.f, 2000.f, 0.f), FVector(1.5f), FRotator(0.f, -15.f, 0.f), QT_PropsCrate); }
		if (QT_PropsComputer) { SpawnRawMesh(FVector(1000.f, SY + 500.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, QT_PropsComputer); SpawnRawMesh(FVector(-4000.f, SY - 1000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), QT_PropsComputer); SpawnRawMesh(FVector(EX - 1000.f, -1000.f, 0.f), FVector(1.5f), FRotator(0.f, 180.f, 0.f), QT_PropsComputer); }
		if (QT_PropsShelf) { SpawnRawMesh(FVector(-3000.f, NY - 500.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, QT_PropsShelf); SpawnRawMesh(FVector(WX + 1500.f, -2000.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), QT_PropsShelf); SpawnRawMesh(FVector(2000.f, SY - 2000.f, 0.f), FVector(1.5f), FRotator(0.f, -10.f, 0.f), QT_PropsShelf); }
		// Vents and pipes on building exteriors
		if (QT_DetailVent1) { SpawnRawMesh(FVector(-5000.f, NY - 4000.f, 1000.f), FVector(1.5f), FRotator::ZeroRotator, QT_DetailVent1); SpawnRawMesh(FVector(EX, -8000.f, 1200.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), QT_DetailVent1); SpawnRawMesh(FVector(3000.f, SY - 3500.f, 800.f), FVector(1.5f), FRotator::ZeroRotator, QT_DetailVent1); SpawnRawMesh(FVector(WX + 4000.f, -6000.f, 1000.f), FVector(1.5f), FRotator(0.f, -90.f, 0.f), QT_DetailVent1); }
		if (QT_DetailPipesLong) { SpawnRawMesh(FVector(5000.f, NY + 5000.f, 500.f), FVector(1.5f), FRotator::ZeroRotator, QT_DetailPipesLong); SpawnRawMesh(FVector(EX + 5000.f, 0.f, 600.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), QT_DetailPipesLong); SpawnRawMesh(FVector(WX - 2000.f, 0.f, 400.f), FVector(1.5f), FRotator::ZeroRotator, QT_DetailPipesLong); SpawnRawMesh(FVector(-5000.f, SY - 4000.f, 500.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), QT_DetailPipesLong); }
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Quaternius walls/columns/props placed"));
	}

	// === SCIFI DOOR at compound entrances ===
	if (bHasSciFiDoorAsset && SF_Door)
	{
		SpawnRawMesh(FVector(0.f, NY - 4000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, SF_Door);
		SpawnRawMesh(FVector(0.f, SY + 5000.f, 0.f), FVector(1.5f), FRotator::ZeroRotator, SF_Door);
		SpawnRawMesh(FVector(EX - 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), SF_Door);
		SpawnRawMesh(FVector(WX + 4000.f, 0.f, 0.f), FVector(1.5f), FRotator(0.f, 90.f, 0.f), SF_Door);
		SpawnRawMesh(FVector(0.f, 3100.f, 0.f), FVector(1.8f), FRotator::ZeroRotator, SF_Door);
		SpawnRawMesh(FVector(0.f, -3100.f, 0.f), FVector(1.8f), FRotator(0.f, 180.f, 0.f), SF_Door);
	}
	else if (bHasQuaterniusAssets && QT_DoorSingle)
	{
		SpawnRawMesh(FVector(0.f, NY - 4000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_DoorSingle);
		SpawnRawMesh(FVector(0.f, SY + 5000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, QT_DoorSingle);
		SpawnRawMesh(FVector(EX - 4000.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, 90.f, 0.f), QT_DoorSingle);
		SpawnRawMesh(FVector(WX + 4000.f, 0.f, 0.f), FVector(2.f), FRotator(0.f, 90.f, 0.f), QT_DoorSingle);
	}

	// === COMPOUND ENTRANCE GATES ===
	struct FGate { FVector Pos; float Yaw; FLinearColor Color; };
	TArray<FGate> Gates = { {{0.f, NY - 4000.f, 0.f}, 0.f, {1.2f, 0.6f, 0.12f}}, {{0.f, SY + 5000.f, 0.f}, 0.f, {0.15f, 1.0f, 0.7f}}, {{EX - 4000.f, 0.f, 0.f}, 90.f, {1.2f, 0.25f, 0.15f}}, {{WX + 4000.f, 0.f, 0.f}, 90.f, {0.2f, 1.0f, 0.3f}} };
	UMaterialInterface* GateEmissive = FExoMaterialFactory::GetEmissiveOpaque();
	for (const auto& G : Gates)
	{
		FRotator GRot(0.f, G.Yaw, 0.f);
		FLinearColor GM(0.09f, 0.1f, 0.12f);
		SpawnStaticMesh(G.Pos + GRot.RotateVector(FVector(-1500.f, 0.f, 1000.f)), FVector(2.f, 2.f, 20.f), GRot, CubeMesh, GM);
		SpawnStaticMesh(G.Pos + GRot.RotateVector(FVector(1500.f, 0.f, 1000.f)), FVector(2.f, 2.f, 20.f), GRot, CubeMesh, GM);
		SpawnStaticMesh(G.Pos + FVector(0.f, 0.f, 2000.f), FVector(31.f, 2.f, 1.2f), GRot, CubeMesh, FLinearColor(0.11f, 0.12f, 0.14f));
		UStaticMeshComponent* GS = SpawnStaticMesh(G.Pos + FVector(0.f, 0.f, 1985.f), FVector(24.f, 0.03f, 0.06f), GRot, CubeMesh, FLinearColor(G.Color.R * 0.1f, G.Color.G * 0.1f, G.Color.B * 0.1f));
		if (GS && GateEmissive) { auto* GI = UMaterialInstanceDynamic::Create(GateEmissive, this); if (!GI) return; GI->SetVectorParameterValue(TEXT("EmissiveColor"), G.Color); GS->SetMaterial(0, GI); }
	}

	// === CORNER OUTPOSTS ===
	const float CD = AExoLevelBuilder::CornerDist;
	FVector Corners[] = { {CD, CD, 0.f}, {-CD, CD, 0.f}, {CD, -CD, 0.f}, {-CD, -CD, 0.f} };
	for (const FVector& C : Corners)
	{
		SpawnBuilding(C, FVector(4000.f, 4000.f, 2000.f), FMath::RandRange(0.f, 45.f));
		SpawnTower(C + FVector(3000.f, 3000.f, 0.f), 500.f, 3500.f);
		SpawnBuilding(C + FVector(-2500.f, 1500.f, 0.f), FVector(2000.f, 2000.f, 1200.f));
		if (bHasKayKitAssets && KK_StructureLow) SpawnRawMesh(C + FVector(-4000.f, -3000.f, 0.f), FVector(2.f), FRotator(0.f, 45.f, 0.f), KK_StructureLow);
		if (bHasKayKitAssets && KK_SolarPanel) SpawnRawMesh(C + FVector(5000.f, -2000.f, 0.f), FVector(2.f), FRotator::ZeroRotator, KK_SolarPanel);
	}

	// === SCATTERED STRUCTURES (0.2x positions — dense map) ===
	struct FBldg { FVector Pos; FVector Size; float Rot; };
	TArray<FBldg> Scattered = {
		{{8000.f, 8000.f, 0.f}, {4000.f, 3000.f, 2000.f}, 30.f}, {{-10000.f, 10000.f, 0.f}, {3000.f, 5000.f, 1500.f}, -20.f},
		{{12000.f, -8000.f, 0.f}, {3500.f, 3500.f, 2200.f}, 45.f}, {{-6000.f, -10000.f, 0.f}, {5000.f, 3000.f, 1800.f}, 10.f},
		{{4000.f, -14000.f, 0.f}, {4000.f, 4000.f, 2500.f}, 0.f}, {{-14000.f, -4000.f, 0.f}, {3000.f, 6000.f, 2000.f}, -15.f},
		{{10000.f, 14000.f, 0.f}, {3500.f, 4000.f, 1600.f}, 60.f}, {{-20000.f, -16000.f, 0.f}, {4500.f, 3500.f, 2400.f}, 25.f},
		{{18000.f, 10000.f, 0.f}, {3000.f, 3000.f, 3000.f}, 0.f}, {{-4000.f, 24000.f, 0.f}, {5000.f, 4000.f, 2000.f}, -35.f},
		{{5000.f, -5000.f, 0.f}, {2500.f, 2500.f, 1600.f}, 15.f}, {{-5000.f, 5000.f, 0.f}, {2000.f, 3000.f, 1400.f}, -40.f},
		{{11000.f, -14000.f, 0.f}, {3000.f, 2500.f, 1800.f}, 70.f}, {{-12000.f, 9000.f, 0.f}, {2500.f, 3500.f, 2000.f}, -25.f},
		{{14000.f, 8000.f, 0.f}, {3500.f, 2500.f, 2200.f}, 5.f}, {{-9000.f, -6000.f, 0.f}, {2000.f, 2000.f, 1200.f}, 50.f},
		{{3000.f, 8000.f, 0.f}, {1500.f, 1500.f, 1000.f}, 0.f}, {{8000.f, -3000.f, 0.f}, {1500.f, 2000.f, 1200.f}, 90.f},
		{{-8000.f, -3000.f, 0.f}, {2000.f, 1500.f, 1000.f}, -10.f}, {{-3000.f, -8000.f, 0.f}, {1500.f, 1500.f, 1200.f}, 45.f},
	};
	for (const auto& B : Scattered) SpawnBuilding(B.Pos, B.Size, B.Rot);

	// === RUINS (0.2x positions) ===
	FLinearColor RuinColor(0.05f, 0.055f, 0.065f);
	struct FRuin { FVector Pos; float Yaw; };
	TArray<FRuin> Ruins = {
		{{6000.f, 2000.f, 0.f}, 20.f}, {{-4000.f, -3000.f, 0.f}, -35.f}, {{10000.f, -5000.f, 0.f}, 60.f}, {{-8000.f, 14000.f, 0.f}, 10.f},
		{{14000.f, -12000.f, 0.f}, -50.f}, {{-12000.f, -12000.f, 0.f}, 140.f}, {{2000.f, -10000.f, 0.f}, 75.f}, {{-10000.f, 2000.f, 0.f}, -20.f},
	};
	for (const auto& R : Ruins)
	{
		FRotator RR(0.f, R.Yaw, 0.f);
		SpawnStaticMesh(R.Pos + FVector(0.f, 0.f, 400.f), FVector(20.f, 0.8f, 8.f), RR, CubeMesh, RuinColor);
		SpawnStaticMesh(R.Pos + RR.RotateVector(FVector(800.f, 0.f, 250.f)), FVector(12.f, 0.8f, 5.f), FRotator(0.f, R.Yaw + 90.f, 0.f), CubeMesh, RuinColor);
		SpawnStaticMesh(R.Pos + RR.RotateVector(FVector(-600.f, 300.f, 50.f)), FVector(0.5f, 0.5f, 6.f), FRotator(80.f, R.Yaw + 20.f, 0.f), CylinderMesh, FLinearColor(0.06f, 0.065f, 0.075f));
		SpawnStaticMesh(R.Pos + FVector(200.f, 200.f, 80.f), FVector(4.f, 3.f, 1.6f), FRotator(8.f, R.Yaw + 15.f, 5.f), CubeMesh, RuinColor);
		SpawnStaticMesh(R.Pos + FVector(0.f, 0.f, 750.f), FVector(6.f, 0.08f, 0.08f), FRotator(-10.f, R.Yaw + 5.f, 0.f), CubeMesh, FLinearColor(0.15f, 0.12f, 0.1f));
	}
}

// SpawnTower, SpawnWall, SpawnPlatform, SpawnRamp → ExoLevelBuilderPrimitives.cpp
