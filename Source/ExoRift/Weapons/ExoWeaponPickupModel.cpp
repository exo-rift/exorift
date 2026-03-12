// ExoWeaponPickupModel.cpp — BuildPickupModel visual construction
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

void AExoWeaponPickup::BuildPickupModel()
{
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* CylMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));

	if (!CubeMesh) return;

	FLinearColor RarityColor = AExoWeaponBase::GetRarityColor(Rarity);

	float EmMul;
	switch (Rarity)
	{
	case EWeaponRarity::Common:    EmMul = 2.5f; break;
	case EWeaponRarity::Rare:      EmMul = 6.0f; break;
	case EWeaponRarity::Epic:      EmMul = 11.0f; break;
	case EWeaponRarity::Legendary: EmMul = 20.0f; break;
	default:                       EmMul = 2.5f; break;
	}

	UMaterialInterface* EmissiveOpaque = FExoMaterialFactory::GetEmissiveOpaque();

	auto MakePart = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator,
		bool bIsAccent = false) -> UMaterialInstanceDynamic*
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->RegisterComponent();
		UMaterialInstanceDynamic* Mat = nullptr;
		if (bIsAccent && EmissiveOpaque)
		{
			Mat = UMaterialInstanceDynamic::Create(EmissiveOpaque, this);
			if (!Mat) { return nullptr; }
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * EmMul, Color.G * EmMul, Color.B * EmMul));
			C->SetMaterial(0, Mat);
		}
		else
		{
			UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
			if (LitMat)
			{
				Mat = UMaterialInstanceDynamic::Create(LitMat, this);
				if (!Mat) { return nullptr; }
				Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
				Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
				Mat->SetScalarParameterValue(TEXT("Metallic"), 0.88f);
				Mat->SetScalarParameterValue(TEXT("Roughness"), 0.25f);
				C->SetMaterial(0, Mat);
			}
		}
		return Mat;
	};

	FLinearColor BodyColor(0.1f, 0.1f, 0.12f);

	switch (WeaponType)
	{
	case EWeaponType::Rifle:
	case EWeaponType::SMG:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.6f, 0.12f, 0.1f), BodyColor);
		MakePart(CubeMesh, FVector(25.f, 0.f, 0.f), FVector(0.35f, 0.06f, 0.06f), BodyColor);
		MakePart(CubeMesh, FVector(-10.f, 0.f, -8.f), FVector(0.08f, 0.06f, 0.12f), BodyColor);
		AccentMat = MakePart(CubeMesh, FVector(0.f, 0.f, 2.f), FVector(0.4f, 0.02f, 0.02f), RarityColor, FRotator::ZeroRotator, true);
		break;

	case EWeaponType::Pistol:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.3f, 0.1f, 0.1f), BodyColor);
		MakePart(CubeMesh, FVector(15.f, 0.f, 0.f), FVector(0.2f, 0.05f, 0.05f), BodyColor);
		MakePart(CubeMesh, FVector(-5.f, 0.f, -8.f), FVector(0.06f, 0.06f, 0.1f), BodyColor);
		AccentMat = MakePart(CubeMesh, FVector(0.f, 0.f, 3.f), FVector(0.25f, 0.02f, 0.02f), RarityColor, FRotator::ZeroRotator, true);
		break;

	case EWeaponType::Shotgun:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.5f, 0.14f, 0.12f), BodyColor);
		MakePart(CubeMesh, FVector(20.f, 0.f, 0.f), FVector(0.25f, 0.08f, 0.08f), BodyColor);
		MakePart(CubeMesh, FVector(20.f, 0.f, 3.f), FVector(0.25f, 0.08f, 0.08f), BodyColor);
		AccentMat = MakePart(CubeMesh, FVector(0.f, 0.f, 3.f), FVector(0.35f, 0.02f, 0.02f), RarityColor, FRotator::ZeroRotator, true);
		break;

	case EWeaponType::Sniper:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.7f, 0.1f, 0.08f), BodyColor);
		MakePart(CubeMesh, FVector(30.f, 0.f, 0.f), FVector(0.4f, 0.04f, 0.04f), BodyColor);
		MakePart(CubeMesh, FVector(10.f, 0.f, 6.f), FVector(0.1f, 0.05f, 0.05f), BodyColor);
		AccentMat = MakePart(CubeMesh, FVector(0.f, 0.f, 2.f), FVector(0.5f, 0.02f, 0.02f), RarityColor, FRotator::ZeroRotator, true);
		break;

	case EWeaponType::GrenadeLauncher:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.45f, 0.14f, 0.14f), BodyColor);
		if (CylMesh) MakePart(CylMesh, FVector(18.f, 0.f, 0.f), FVector(0.12f, 0.12f, 0.15f), BodyColor,
			FRotator(0.f, 0.f, 90.f));
		AccentMat = MakePart(CubeMesh, FVector(0.f, 0.f, 3.f), FVector(0.3f, 0.02f, 0.02f), RarityColor, FRotator::ZeroRotator, true);
		break;

	default:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.4f, 0.1f, 0.1f), BodyColor);
		AccentMat = MakePart(CubeMesh, FVector(0.f, 0.f, 2.f), FVector(0.3f, 0.02f, 0.02f), RarityColor, FRotator::ZeroRotator, true);
		break;
	}

	// Rarity glow light
	RarityGlow = NewObject<UPointLightComponent>(this);
	RarityGlow->SetupAttachment(RootComponent);
	RarityGlow->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	RarityGlow->SetLightColor(RarityColor);
	RarityGlow->CastShadows = false;
	RarityGlow->RegisterComponent();

	float GlowIntensity;
	float GlowRadius;
	switch (Rarity)
	{
	case EWeaponRarity::Common:    GlowIntensity = 4000.f; GlowRadius = 375.f; break;
	case EWeaponRarity::Rare:      GlowIntensity = 10000.f; GlowRadius = 525.f; break;
	case EWeaponRarity::Epic:      GlowIntensity = 20000.f; GlowRadius = 675.f; break;
	case EWeaponRarity::Legendary: GlowIntensity = 32000.f; GlowRadius = 825.f; break;
	default:                       GlowIntensity = 4000.f; GlowRadius = 375.f; break;
	}
	RarityGlow->SetIntensity(GlowIntensity);
	RarityGlow->SetAttenuationRadius(GlowRadius);

	// Holographic pedestal plate
	PedestalPlate = NewObject<UStaticMeshComponent>(this);
	PedestalPlate->SetupAttachment(RootComponent);
	PedestalPlate->SetStaticMesh(CylMesh ? CylMesh : CubeMesh);
	PedestalPlate->SetRelativeLocation(FVector(0.f, 0.f, -25.f));
	PedestalPlate->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.01f));
	PedestalPlate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PedestalPlate->CastShadow = false;
	PedestalPlate->RegisterComponent();
	if (EmissiveOpaque)
	{
		PedestalMat = UMaterialInstanceDynamic::Create(EmissiveOpaque, this);
		if (!PedestalMat) { return; }
		PedestalMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(RarityColor.R * EmMul * 0.3f, RarityColor.G * EmMul * 0.3f,
				RarityColor.B * EmMul * 0.3f));
		PedestalPlate->SetMaterial(0, PedestalMat);
	}

	// Rarity ring — thin spinning halo
	if (CylMesh)
	{
		RarityRing = NewObject<UStaticMeshComponent>(this);
		RarityRing->SetupAttachment(RootComponent);
		RarityRing->SetStaticMesh(CylMesh);
		RarityRing->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
		RarityRing->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.005f));
		RarityRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RarityRing->CastShadow = false;
		RarityRing->RegisterComponent();
		if (EmissiveOpaque)
		{
			RingMat = UMaterialInstanceDynamic::Create(EmissiveOpaque, this);
			if (!RingMat) { return; }
			RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(RarityColor.R * EmMul * 0.6f, RarityColor.G * EmMul * 0.6f,
					RarityColor.B * EmMul * 0.6f));
			RarityRing->SetMaterial(0, RingMat);
		}
	}

	// Vertical beacon beam — visible from distance (all rarities)
	UMaterialInterface* EmissiveAdditive = FExoMaterialFactory::GetEmissiveAdditive();
	if (CylMesh && EmissiveAdditive)
	{
		// Beam width scales with rarity — even Common gets a dim beacon
		float BeamWidth = 0.025f;
		float BeamHeight = 12.f;
		switch (Rarity)
		{
		case EWeaponRarity::Common:    BeamWidth = 0.025f; BeamHeight = 10.f; break;
		case EWeaponRarity::Rare:      BeamWidth = 0.035f; BeamHeight = 14.f; break;
		case EWeaponRarity::Epic:      BeamWidth = 0.045f; BeamHeight = 18.f; break;
		case EWeaponRarity::Legendary: BeamWidth = 0.06f;  BeamHeight = 22.f; break;
		default: break;
		}

		BeaconBeam = NewObject<UStaticMeshComponent>(this);
		BeaconBeam->SetupAttachment(RootComponent);
		BeaconBeam->SetStaticMesh(CylMesh);
		BeaconBeam->SetRelativeLocation(FVector(0.f, 0.f, BeamHeight * 50.f));
		BeaconBeam->SetRelativeScale3D(FVector(BeamWidth, BeamWidth, BeamHeight));
		BeaconBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BeaconBeam->CastShadow = false;
		BeaconBeam->RegisterComponent();

		BeaconMat = UMaterialInstanceDynamic::Create(EmissiveAdditive, this);
		if (!BeaconMat) { return; }
		float BeaconEm = EmMul * 4.f;
		BeaconMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(RarityColor.R * BeaconEm, RarityColor.G * BeaconEm,
				RarityColor.B * BeaconEm));
		BeaconBeam->SetMaterial(0, BeaconMat);
	}
}
