// ExoWeaponViewModel.cpp — Core logic: construction, material, heat glow
// Build*Model functions are in ExoWeaponViewModelBuilders.cpp
#include "Visual/ExoWeaponViewModel.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoFPArms.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UExoWeaponViewModel::UExoWeaponViewModel()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;
}

void UExoWeaponViewModel::BuildModel(EWeaponType Type, const FLinearColor& RarityColor)
{
	switch (Type)
	{
	case EWeaponType::Rifle:          BuildRifleModel(RarityColor); break;
	case EWeaponType::Pistol:         BuildPistolModel(RarityColor); break;
	case EWeaponType::SMG:            BuildSMGModel(RarityColor); break;
	case EWeaponType::Shotgun:        BuildShotgunModel(RarityColor); break;
	case EWeaponType::Sniper:         BuildSniperModel(RarityColor); break;
	case EWeaponType::GrenadeLauncher: BuildLauncherModel(RarityColor); break;
	case EWeaponType::Melee:          BuildMeleeModel(RarityColor); break;
	default: BuildRifleModel(RarityColor); break;
	}

	// First-person arms
	Arms = NewObject<UExoFPArms>(GetOwner());
	Arms->SetupAttachment(this);
	Arms->RegisterComponent();
	Arms->BuildArms(FLinearColor(0.08f, 0.1f, 0.12f));

	// Muzzle ready indicator light
	MuzzleReadyLight = NewObject<UPointLightComponent>(GetOwner());
	MuzzleReadyLight->SetupAttachment(this);
	FVector MuzzlePos = MuzzleTip ? MuzzleTip->GetRelativeLocation() : FVector(40.f, 0.f, 0.f);
	MuzzleReadyLight->SetRelativeLocation(MuzzlePos);
	MuzzleReadyLight->SetIntensity(4500.f);
	MuzzleReadyLight->SetAttenuationRadius(280.f);
	MuzzleReadyLight->SetLightColor(RarityColor);
	MuzzleReadyLight->CastShadows = false;
	MuzzleReadyLight->RegisterComponent();

	// Dedicated barrel heat light — starts invisible, grows with weapon temperature
	HeatLight = NewObject<UPointLightComponent>(GetOwner());
	HeatLight->SetupAttachment(this);
	// Position midway along the barrel for even heat illumination
	FVector BarrelMid = MuzzlePos * 0.6f;
	HeatLight->SetRelativeLocation(BarrelMid + FVector(0.f, 0.f, 3.f));
	HeatLight->SetIntensity(0.f);
	HeatLight->SetAttenuationRadius(50.f);
	HeatLight->SetLightColor(FLinearColor(1.f, 0.35f, 0.05f));
	HeatLight->CastShadows = false;
	HeatLight->RegisterComponent();
}

FVector UExoWeaponViewModel::GetMuzzleLocation() const
{
	if (MuzzleTip)
		return MuzzleTip->GetComponentLocation();
	return GetComponentLocation() + GetForwardVector() * 40.f;
}

UStaticMeshComponent* UExoWeaponViewModel::AddPart(const FVector& Offset,
	const FVector& Scale, const FLinearColor& Color, UStaticMesh* Mesh)
{
	if (!Mesh) Mesh = CubeMesh;
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(GetOwner());
	Part->SetupAttachment(this);
	Part->SetStaticMesh(Mesh);
	Part->SetRelativeLocation(Offset);
	Part->SetRelativeScale3D(Scale);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->CastShadow = false;
	Part->SetGenerateOverlapEvents(false);

	// PBR + emissive material for weapon parts
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, GetOwner());
		if (!Mat) { return nullptr; }
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);

		float Luminance = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Luminance > 0.15f)
		{
			// Accent/glow parts: strong emissive for bloom, less metallic
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 90.f, Color.G * 90.f, Color.B * 90.f));
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.35f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.08f);
		}
		else
		{
			// Dark body/structural parts: metallic gun surface, no emissive
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.92f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.18f);
		}
		Part->SetMaterial(0, Mat);
	}

	Part->RegisterComponent();
	return Part;
}

UStaticMeshComponent* UExoWeaponViewModel::AddBarrelPart(const FVector& Offset,
	const FVector& Scale, const FLinearColor& Color, UStaticMesh* Mesh)
{
	UStaticMeshComponent* Part = AddPart(Offset, Scale, Color, Mesh);
	if (Part)
	{
		// Override with LitEmissive that tracks heat glow separately
		UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
		if (LitMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, GetOwner());
			if (!Mat) { return nullptr; }
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.92f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.18f);
			Part->SetMaterial(0, Mat);
			BarrelMats.Add(Mat);
			BarrelBaseColors.Add(Color);
		}
	}
	return Part;
}

void UExoWeaponViewModel::UpdateHeatGlow(float Heat)
{
	// Accumulate shimmer phase when hot (pulsing intensifies with heat)
	if (Heat > 0.5f && GetWorld())
	{
		float ShimmerSpeed = FMath::Lerp(3.f, 12.f, (Heat - 0.5f) / 0.5f);
		HeatShimmerPhase += GetWorld()->GetDeltaSeconds() * ShimmerSpeed;
	}
	else
	{
		HeatShimmerPhase = 0.f;
	}

	// Cool barrel — reset everything to default
	if (Heat < 0.05f)
	{
		for (int32 i = 0; i < BarrelMats.Num(); i++)
		{
			if (!BarrelMats[i]) continue;
			BarrelMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			if (i < BarrelBaseColors.Num())
				BarrelMats[i]->SetVectorParameterValue(TEXT("BaseColor"), BarrelBaseColors[i]);
			BarrelMats[i]->SetScalarParameterValue(TEXT("Roughness"), 0.18f);
		}
		if (HeatLight)
		{
			HeatLight->SetIntensity(0.f);
		}
		PrevHeat = Heat;
		return;
	}

	// --- Emissive color ramp (dramatically brighter than before) ---
	// 0.05-0.3: dark cherry red (barely visible warmth)
	// 0.3-0.55: deep red-orange (clearly glowing)
	// 0.55-0.8: bright orange (intense heat)
	// 0.8-1.0: searing white-orange (about to blow)
	FLinearColor HeatEmissive;
	if (Heat < 0.3f)
	{
		float T = (Heat - 0.05f) / 0.25f;
		HeatEmissive = FLinearColor(T * 8.f, T * 0.8f, T * 0.15f);
	}
	else if (Heat < 0.55f)
	{
		float T = (Heat - 0.3f) / 0.25f;
		HeatEmissive = FLinearColor(8.f + T * 25.f, 0.8f + T * 8.f, 0.15f + T * 1.f);
	}
	else if (Heat < 0.8f)
	{
		float T = (Heat - 0.55f) / 0.25f;
		HeatEmissive = FLinearColor(33.f + T * 55.f, 8.8f + T * 30.f, 1.15f + T * 8.f);
	}
	else
	{
		float T = (Heat - 0.8f) / 0.2f;
		HeatEmissive = FLinearColor(88.f + T * 110.f, 38.8f + T * 80.f, 9.15f + T * 60.f);
	}

	// Shimmer pulse — makes the glow throb at high heat
	if (Heat > 0.5f)
	{
		// Two overlapping sine waves for organic pulsing
		float Pulse = FMath::Sin(HeatShimmerPhase) * 0.15f
			+ FMath::Sin(HeatShimmerPhase * 2.3f + 0.7f) * 0.08f;
		float PulseStrength = FMath::Clamp((Heat - 0.5f) / 0.5f, 0.f, 1.f);
		float PulseMult = 1.f + Pulse * PulseStrength;
		HeatEmissive.R *= PulseMult;
		HeatEmissive.G *= PulseMult;
		HeatEmissive.B *= PulseMult;
	}

	// --- Base color shift: dark metal warms toward dull orange ---
	float ColorShiftAlpha = FMath::Clamp((Heat - 0.3f) / 0.7f, 0.f, 1.f);
	FLinearColor HotBaseColor(0.25f, 0.08f, 0.02f);

	// --- Roughness decrease: hot metal becomes shinier ---
	float HotRoughness = FMath::Lerp(0.18f, 0.04f, FMath::Clamp(Heat, 0.f, 1.f));

	for (int32 i = 0; i < BarrelMats.Num(); i++)
	{
		if (!BarrelMats[i]) continue;

		BarrelMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), HeatEmissive);

		// Shift base color toward hot orange
		if (i < BarrelBaseColors.Num())
		{
			FLinearColor BlendedBase = FMath::Lerp(BarrelBaseColors[i], HotBaseColor, ColorShiftAlpha);
			BarrelMats[i]->SetVectorParameterValue(TEXT("BaseColor"), BlendedBase);
		}

		BarrelMats[i]->SetScalarParameterValue(TEXT("Roughness"), HotRoughness);
	}

	// --- Dedicated barrel heat light ---
	if (HeatLight)
	{
		// Light activates above 0.3 heat, scales dramatically
		float LightAlpha = FMath::Clamp((Heat - 0.3f) / 0.7f, 0.f, 1.f);

		// Intensity ramps from 0 to 12000 (very visible cast light)
		float Intensity = LightAlpha * LightAlpha * 12000.f;

		// Shimmer on the light too
		if (Heat > 0.5f)
		{
			float LightPulse = FMath::Sin(HeatShimmerPhase * 0.8f) * 0.12f;
			float PulseStr = FMath::Clamp((Heat - 0.5f) / 0.5f, 0.f, 1.f);
			Intensity *= (1.f + LightPulse * PulseStr);
		}

		HeatLight->SetIntensity(Intensity);
		HeatLight->SetAttenuationRadius(FMath::Lerp(50.f, 400.f, LightAlpha));

		// Light color shifts from deep red to bright orange-white
		FLinearColor LightColor;
		if (Heat < 0.6f)
		{
			float T = FMath::Clamp((Heat - 0.3f) / 0.3f, 0.f, 1.f);
			LightColor = FLinearColor(0.8f + T * 0.2f, T * 0.25f, T * 0.02f);
		}
		else
		{
			float T = FMath::Clamp((Heat - 0.6f) / 0.4f, 0.f, 1.f);
			LightColor = FLinearColor(1.f, 0.25f + T * 0.55f, 0.02f + T * 0.35f);
		}
		HeatLight->SetLightColor(LightColor);
	}

	PrevHeat = Heat;
}
