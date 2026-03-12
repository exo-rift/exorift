// ExoSupplyDropVisuals.cpp — Crate mesh construction and loot spawning
#include "Map/ExoSupplyDrop.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoEnergyCellPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "Engine/StaticMesh.h"
#include "ExoRift.h"

void AExoSupplyDrop::BuildCrateMesh()
{
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	auto MakePart = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator) -> UStaticMeshComponent*
	{
		if (!Mesh) return nullptr;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = true;
		C->RegisterComponent();
		if (LitMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, this);
			if (!Mat) { return nullptr; }
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
			if (Lum > 0.15f)
			{
				Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(Color.R * 12.f, Color.G * 12.f, Color.B * 12.f));
				Mat->SetScalarParameterValue(TEXT("Metallic"), 0.5f);
				Mat->SetScalarParameterValue(TEXT("Roughness"), 0.15f);
			}
			else
			{
				Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
				Mat->SetScalarParameterValue(TEXT("Metallic"), 0.85f);
				Mat->SetScalarParameterValue(TEXT("Roughness"), 0.25f);
			}
			C->SetMaterial(0, Mat);
		}
		return C;
	};

	FLinearColor CrateColor(0.1f, 0.12f, 0.08f);      // Military olive
	FLinearColor AccentColor(0.9f, 0.7f, 0.1f);         // Gold accent
	FLinearColor ParaColor(0.8f, 0.4f, 0.1f);           // Orange chute

	// Main crate body
	CrateBody = MakePart(CubeMeshRef, FVector(0.f, 0.f, 0.f),
		FVector(1.2f, 0.8f, 0.6f), CrateColor);

	// Lid (will animate open)
	CrateLid = MakePart(CubeMeshRef, FVector(0.f, 0.f, 35.f),
		FVector(1.25f, 0.85f, 0.05f), CrateColor);

	// Gold accent stripes on sides
	MakePart(CubeMeshRef, FVector(0.f, 42.f, 0.f), FVector(1.0f, 0.02f, 0.45f), AccentColor);
	MakePart(CubeMeshRef, FVector(0.f, -42.f, 0.f), FVector(1.0f, 0.02f, 0.45f), AccentColor);

	// Corner reinforcements
	for (float cx : {-55.f, 55.f})
		for (float cy : {-35.f, 35.f})
			MakePart(CubeMeshRef, FVector(cx, cy, -20.f),
				FVector(0.08f, 0.08f, 0.15f), FLinearColor(0.15f, 0.15f, 0.15f));

	// Parachute dome (half sphere above)
	ParachuteDome = MakePart(SphereMeshRef, FVector(0.f, 0.f, 350.f),
		FVector(2.5f, 2.5f, 1.5f), ParaColor);

	// Parachute struts (lines connecting dome to crate)
	ParachuteStrut1 = MakePart(CylinderMeshRef, FVector(40.f, 0.f, 180.f),
		FVector(0.02f, 0.02f, 2.f), FLinearColor(0.3f, 0.3f, 0.3f),
		FRotator(0.f, 0.f, 8.f));
	ParachuteStrut2 = MakePart(CylinderMeshRef, FVector(-40.f, 0.f, 180.f),
		FVector(0.02f, 0.02f, 2.f), FLinearColor(0.3f, 0.3f, 0.3f),
		FRotator(0.f, 0.f, -8.f));

	// Top edge trim (gold lines)
	MakePart(CubeMeshRef, FVector(60.f, 0.f, 32.f), FVector(0.02f, 0.7f, 0.02f), AccentColor);
	MakePart(CubeMeshRef, FVector(-60.f, 0.f, 32.f), FVector(0.02f, 0.7f, 0.02f), AccentColor);

	// Hazard stripe on front face
	MakePart(CubeMeshRef, FVector(62.f, 0.f, 0.f), FVector(0.02f, 0.5f, 0.1f),
		FLinearColor(0.6f, 0.5f, 0.05f));

	// Lock hasp (front center)
	MakePart(CubeMeshRef, FVector(62.f, 0.f, 25.f), FVector(0.04f, 0.08f, 0.06f),
		FLinearColor(0.15f, 0.15f, 0.15f));

	// Inner crate glow
	CrateGlow = NewObject<UPointLightComponent>(this);
	CrateGlow->SetupAttachment(RootComponent);
	CrateGlow->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	CrateGlow->SetIntensity(0.f);
	CrateGlow->SetAttenuationRadius(500.f);
	CrateGlow->SetLightColor(FColor(255, 200, 50));
	CrateGlow->RegisterComponent();

	// Smoke trail — elongated cylinder trailing above during descent
	SmokeTrail = MakePart(CylinderMeshRef, FVector(0.f, 0.f, 500.f),
		FVector(0.8f, 0.8f, 5.f), FLinearColor(0.15f, 0.12f, 0.08f));
	if (SmokeTrail) SmokeTrail->CastShadow = false;

	// Trail light — warm glow from engine exhaust (visible from far)
	TrailLight = NewObject<UPointLightComponent>(this);
	TrailLight->SetupAttachment(RootComponent);
	TrailLight->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	TrailLight->SetIntensity(90000.f);
	TrailLight->SetAttenuationRadius(6000.f);
	TrailLight->SetLightColor(FLinearColor(1.f, 0.5f, 0.1f));
	TrailLight->CastShadows = false;
	TrailLight->RegisterComponent();

	// Descent beacon beam — bright vertical streak visible across the map
	UMaterialInterface* BeamAdditive = FExoMaterialFactory::GetEmissiveAdditive();
	if (CylinderMeshRef && BeamAdditive)
	{
		DescentBeam = NewObject<UStaticMeshComponent>(this);
		DescentBeam->SetupAttachment(RootComponent);
		DescentBeam->SetStaticMesh(CylinderMeshRef);
		DescentBeam->SetRelativeLocation(FVector(0.f, 0.f, 3000.f));
		DescentBeam->SetRelativeScale3D(FVector(0.04f, 0.04f, 60.f));
		DescentBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DescentBeam->CastShadow = false;
		DescentBeam->RegisterComponent();
		DescentBeamMat = UMaterialInstanceDynamic::Create(BeamAdditive, this);
		if (!DescentBeamMat) { return; }
		DescentBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(9.f, 4.5f, 1.2f));
		DescentBeam->SetMaterial(0, DescentBeamMat);
	}

	// Beacon column — wide glowing cylinder rising from the crate, visible at distance
	if (CylinderMeshRef && BeamAdditive)
	{
		BeaconColumn = NewObject<UStaticMeshComponent>(this);
		BeaconColumn->SetupAttachment(RootComponent);
		BeaconColumn->SetStaticMesh(CylinderMeshRef);
		// Column starts just above the crate and extends 1500 units upward
		BeaconColumn->SetRelativeLocation(FVector(0.f, 0.f, 800.f));
		BeaconColumn->SetRelativeScale3D(FVector(0.25f, 0.25f, 15.f));
		BeaconColumn->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BeaconColumn->CastShadow = false;
		BeaconColumn->RegisterComponent();
		BeaconColumnMat = UMaterialInstanceDynamic::Create(BeamAdditive, this);
		if (!BeaconColumnMat) { return; }
		BeaconColumnMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(12.f, 1.5f, 1.5f)); // Bright red beacon
		BeaconColumn->SetMaterial(0, BeaconColumnMat);
	}

	// Flashing point light at the top of the beacon column (alternates red/white)
	BeaconFlashLight = NewObject<UPointLightComponent>(this);
	BeaconFlashLight->SetupAttachment(RootComponent);
	BeaconFlashLight->SetRelativeLocation(FVector(0.f, 0.f, 1600.f));
	BeaconFlashLight->SetIntensity(4000.f);
	BeaconFlashLight->SetAttenuationRadius(15000.f);
	BeaconFlashLight->SetLightColor(FLinearColor(1.f, 0.1f, 0.1f));
	BeaconFlashLight->CastShadows = false;
	BeaconFlashLight->RegisterComponent();
	BeaconFlashTimer = 0.f;
}

void AExoSupplyDrop::SpawnLoot()
{
	FVector Origin = GetActorLocation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn 2 weapons at Epic or Legendary rarity
	const EWeaponType WeaponTypes[] = {
		EWeaponType::Rifle, EWeaponType::Pistol, EWeaponType::GrenadeLauncher };
	const EWeaponRarity HighRarities[] = {
		EWeaponRarity::Epic, EWeaponRarity::Legendary };

	for (int32 i = 0; i < 2; ++i)
	{
		float Angle = i * PI;
		FVector Offset(FMath::Cos(Angle) * 120.f, FMath::Sin(Angle) * 120.f, 30.f);
		FVector SpawnLoc = Origin + Offset;

		AExoWeaponPickup* WeaponPickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
			AExoWeaponPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (WeaponPickup)
		{
			WeaponPickup->WeaponType = WeaponTypes[FMath::RandRange(0, 2)];
			WeaponPickup->Rarity = HighRarities[FMath::RandRange(0, 1)];
		}
	}

	// Spawn 2 energy cells
	for (int32 i = 0; i < 2; ++i)
	{
		float Angle = (i * PI) + PI * 0.5f;
		FVector Offset(FMath::Cos(Angle) * 100.f, FMath::Sin(Angle) * 100.f, 20.f);
		FVector SpawnLoc = Origin + Offset;

		AExoEnergyCellPickup* CellPickup = GetWorld()->SpawnActor<AExoEnergyCellPickup>(
			AExoEnergyCellPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (CellPickup)
		{
			CellPickup->EnergyAmount = 100.f;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Supply drop spawned loot at %s"), *Origin.ToString());
}
