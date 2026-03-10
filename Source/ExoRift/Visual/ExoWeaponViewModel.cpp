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
	MuzzleReadyLight->SetIntensity(800.f);
	MuzzleReadyLight->SetAttenuationRadius(120.f);
	MuzzleReadyLight->SetLightColor(RarityColor);
	MuzzleReadyLight->CastShadows = false;
	MuzzleReadyLight->RegisterComponent();
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

	// Use LitEmissive for proper PBR + emissive (BasicShapeMaterial has no EmissiveColor)
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, GetOwner());
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);

		float Luminance = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Luminance > 0.15f)
		{
			// Accent/glow parts: strong emissive for bloom, less metallic
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 20.f, Color.G * 20.f, Color.B * 20.f));
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.4f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.1f);
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
	if (Heat < 0.05f)
	{
		for (int32 i = 0; i < BarrelMats.Num(); i++)
		{
			if (!BarrelMats[i]) continue;
			BarrelMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		}
		return;
	}

	// Heat color ramp: 0.05-0.4 = dark red, 0.4-0.7 = red-orange, 0.7-1.0 = orange-white
	FLinearColor HeatEmissive;
	if (Heat < 0.4f)
	{
		float T = (Heat - 0.05f) / 0.35f;
		HeatEmissive = FLinearColor(T * 2.f, T * 0.3f, T * 0.05f);
	}
	else if (Heat < 0.7f)
	{
		float T = (Heat - 0.4f) / 0.3f;
		HeatEmissive = FLinearColor(2.f + T * 4.f, 0.3f + T * 2.f, 0.05f + T * 0.3f);
	}
	else
	{
		float T = (Heat - 0.7f) / 0.3f;
		HeatEmissive = FLinearColor(6.f + T * 10.f, 2.3f + T * 6.f, 0.35f + T * 4.f);
	}

	for (int32 i = 0; i < BarrelMats.Num(); i++)
	{
		if (!BarrelMats[i]) continue;
		BarrelMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), HeatEmissive);
	}
}
