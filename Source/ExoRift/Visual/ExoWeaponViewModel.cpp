#include "Visual/ExoWeaponViewModel.h"
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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded()) BaseMaterial = MatFind.Object;
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

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, GetOwner());
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		// Bright colors get emissive treatment for sci-fi glow
		float Luminance = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Luminance > 0.15f)
		{
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 4.f, Color.G * 4.f, Color.B * 4.f));
		}
		Part->SetMaterial(0, Mat);
	}

	Part->RegisterComponent();
	return Part;
}

void UExoWeaponViewModel::BuildRifleModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.08f, 0.08f, 0.1f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Trim(0.1f, 0.1f, 0.13f);

	// Main receiver
	AddPart(FVector(15.f, 0.f, 0.f), FVector(0.35f, 0.06f, 0.05f), Body);
	// Barrel shroud
	AddPart(FVector(32.f, 0.f, 0.f), FVector(0.12f, 0.04f, 0.04f), Dark);
	// Barrel
	MuzzleTip = AddPart(FVector(42.f, 0.f, 0.5f), FVector(0.16f, 0.02f, 0.02f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Muzzle brake
	AddPart(FVector(48.f, 0.f, 0.5f), FVector(0.025f, 0.025f, 0.015f), Trim, CylinderMesh);
	// Magazine
	AddPart(FVector(10.f, 0.f, -4.f), FVector(0.06f, 0.03f, 0.08f), Dark);
	// Grip
	AddPart(FVector(0.f, 0.f, -3.f), FVector(0.04f, 0.04f, 0.06f), Body);
	// Scope rail
	AddPart(FVector(18.f, 0.f, 3.5f), FVector(0.14f, 0.015f, 0.008f), Trim);
	// Heat vents (side slits)
	AddPart(FVector(25.f, 3.2f, 0.f), FVector(0.08f, 0.003f, 0.02f), Trim);
	AddPart(FVector(25.f, -3.2f, 0.f), FVector(0.08f, 0.003f, 0.02f), Trim);
	// Accent stripe — glowing rarity color
	AddPart(FVector(20.f, 0.f, 2.5f), FVector(0.22f, 0.065f, 0.005f), Accent);
	// Charging handle
	AddPart(FVector(5.f, 0.f, 2.8f), FVector(0.03f, 0.02f, 0.01f), Trim);
	// Ejection port cover
	AddPart(FVector(14.f, 3.f, 1.f), FVector(0.04f, 0.003f, 0.015f), Trim);
	// Forward grip nub
	AddPart(FVector(25.f, 0.f, -2.5f), FVector(0.025f, 0.025f, 0.04f), Dark);
	// Buttstock
	AddPart(FVector(-12.f, 0.f, 0.f), FVector(0.08f, 0.04f, 0.04f), Dark);
	AddPart(FVector(-16.f, 0.f, 0.f), FVector(0.02f, 0.05f, 0.04f), Trim);
	// Secondary accent dot (rear sight area)
	AddPart(FVector(2.f, 0.f, 3.2f), FVector(0.006f, 0.006f, 0.006f), Accent);
}

void UExoWeaponViewModel::BuildSMGModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.07f, 0.07f, 0.09f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Trim(0.09f, 0.09f, 0.12f);

	// Compact body
	AddPart(FVector(10.f, 0.f, 0.f), FVector(0.22f, 0.055f, 0.045f), Body);
	// Upper rail
	AddPart(FVector(14.f, 0.f, 2.8f), FVector(0.1f, 0.012f, 0.006f), Trim);
	// Short barrel
	MuzzleTip = AddPart(FVector(28.f, 0.f, 0.5f), FVector(0.08f, 0.018f, 0.018f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Barrel shroud
	AddPart(FVector(24.f, 0.f, 0.5f), FVector(0.05f, 0.022f, 0.022f), Body, CylinderMesh);
	// Magazine (straight)
	AddPart(FVector(8.f, 0.f, -4.f), FVector(0.04f, 0.025f, 0.07f), Dark);
	// Grip
	AddPart(FVector(-2.f, 0.f, -3.f), FVector(0.035f, 0.035f, 0.05f), Body);
	// Folding stock stub
	AddPart(FVector(-8.f, 0.f, 0.5f), FVector(0.06f, 0.02f, 0.02f), Dark);
	// Side heat vent
	AddPart(FVector(18.f, 2.8f, 0.f), FVector(0.06f, 0.003f, 0.015f), Trim);
	// Accent band — glowing
	AddPart(FVector(15.f, 0.f, 2.f), FVector(0.15f, 0.06f, 0.004f), Accent);
}

void UExoWeaponViewModel::BuildShotgunModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.1f, 0.08f, 0.07f);
	FLinearColor Dark(0.05f, 0.04f, 0.04f);
	FLinearColor Trim(0.12f, 0.1f, 0.09f);

	// Thick body
	AddPart(FVector(12.f, 0.f, 0.f), FVector(0.3f, 0.07f, 0.06f), Body);
	// Wide barrel
	MuzzleTip = AddPart(FVector(38.f, 0.f, 0.f), FVector(0.12f, 0.03f, 0.03f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Muzzle flare (wider at end)
	AddPart(FVector(44.f, 0.f, 0.f), FVector(0.015f, 0.035f, 0.035f), Trim, CylinderMesh);
	// Pump slide
	AddPart(FVector(22.f, 0.f, -1.5f), FVector(0.08f, 0.04f, 0.035f), Trim);
	// Pump rail
	AddPart(FVector(28.f, 0.f, -2.5f), FVector(0.12f, 0.006f, 0.006f), Dark);
	// Grip
	AddPart(FVector(-2.f, 0.f, -4.f), FVector(0.04f, 0.04f, 0.07f), Body);
	// Stock
	AddPart(FVector(-12.f, 0.f, 0.f), FVector(0.1f, 0.04f, 0.04f), Dark);
	// Stock pad
	AddPart(FVector(-17.f, 0.f, 0.f), FVector(0.02f, 0.05f, 0.05f), Trim);
	// Shell indicator lights (3 dots on side)
	AddPart(FVector(8.f, 3.8f, 1.f), FVector(0.01f, 0.005f, 0.005f), Accent);
	AddPart(FVector(11.f, 3.8f, 1.f), FVector(0.01f, 0.005f, 0.005f), Accent);
	AddPart(FVector(14.f, 3.8f, 1.f), FVector(0.01f, 0.005f, 0.005f), Accent);
	// Top accent
	AddPart(FVector(8.f, 0.f, 3.f), FVector(0.1f, 0.075f, 0.004f), Accent);
}

void UExoWeaponViewModel::BuildSniperModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.06f, 0.06f, 0.08f);
	FLinearColor Dark(0.03f, 0.03f, 0.04f);
	FLinearColor Trim(0.08f, 0.08f, 0.1f);

	// Long body
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.45f, 0.05f, 0.04f), Body);
	// Long barrel
	MuzzleTip = AddPart(FVector(55.f, 0.f, 0.5f), FVector(0.22f, 0.015f, 0.015f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Barrel stabilizer fins
	AddPart(FVector(50.f, 0.f, 2.5f), FVector(0.04f, 0.002f, 0.015f), Trim);
	AddPart(FVector(50.f, 0.f, -1.5f), FVector(0.04f, 0.002f, 0.015f), Trim);
	// Scope body (cylinder)
	AddPart(FVector(22.f, 0.f, 4.5f), FVector(0.1f, 0.025f, 0.025f), Dark, CylinderMesh);
	// Scope lens (front) — glowing accent
	AddPart(FVector(27.f, 0.f, 4.5f), FVector(0.003f, 0.02f, 0.02f), Accent, CylinderMesh);
	// Scope lens (rear)
	AddPart(FVector(17.f, 0.f, 4.5f), FVector(0.003f, 0.015f, 0.015f), Accent, CylinderMesh);
	// Magazine
	AddPart(FVector(15.f, 0.f, -4.f), FVector(0.04f, 0.025f, 0.06f), Dark);
	// Grip
	AddPart(FVector(0.f, 0.f, -3.5f), FVector(0.035f, 0.035f, 0.06f), Body);
	// Stock — adjustable
	AddPart(FVector(-15.f, 0.f, 0.f), FVector(0.12f, 0.04f, 0.05f), Dark);
	AddPart(FVector(-20.f, 0.f, 0.5f), FVector(0.04f, 0.05f, 0.04f), Trim);
	// Bipod stubs (folded)
	AddPart(FVector(30.f, 2.f, -2.f), FVector(0.005f, 0.005f, 0.04f), Dark);
	AddPart(FVector(30.f, -2.f, -2.f), FVector(0.005f, 0.005f, 0.04f), Dark);
	// Accent energy line — long glowing strip
	AddPart(FVector(25.f, 0.f, 2.f), FVector(0.32f, 0.055f, 0.003f), Accent);
}

void UExoWeaponViewModel::BuildPistolModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.07f, 0.07f, 0.09f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Trim(0.09f, 0.09f, 0.12f);

	// Slide
	AddPart(FVector(8.f, 0.f, 1.f), FVector(0.18f, 0.04f, 0.035f), Body);
	// Slide serrations (back)
	AddPart(FVector(0.f, 0.f, 1.f), FVector(0.03f, 0.042f, 0.037f), Trim);
	// Short barrel
	MuzzleTip = AddPart(FVector(22.f, 0.f, 1.f), FVector(0.06f, 0.015f, 0.015f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Frame / lower
	AddPart(FVector(6.f, 0.f, -1.f), FVector(0.14f, 0.038f, 0.02f), Dark);
	// Grip (angled)
	AddPart(FVector(2.f, 0.f, -4.f), FVector(0.035f, 0.035f, 0.07f), Dark);
	// Trigger guard
	AddPart(FVector(5.f, 0.f, -1.5f), FVector(0.04f, 0.015f, 0.02f), Dark);
	// Accessory rail
	AddPart(FVector(12.f, 0.f, -1.5f), FVector(0.06f, 0.01f, 0.006f), Trim);
	// Accent stripe — glowing
	AddPart(FVector(10.f, 0.f, 2.5f), FVector(0.12f, 0.045f, 0.003f), Accent);
	// Rear sight
	AddPart(FVector(1.f, 0.f, 2.2f), FVector(0.008f, 0.02f, 0.008f), Trim);
}

void UExoWeaponViewModel::BuildLauncherModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.1f, 0.09f, 0.08f);
	FLinearColor Dark(0.05f, 0.05f, 0.06f);
	FLinearColor Trim(0.12f, 0.11f, 0.1f);

	// Thick cylindrical barrel
	MuzzleTip = AddPart(FVector(30.f, 0.f, 0.f), FVector(0.22f, 0.04f, 0.04f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Barrel ring (muzzle end)
	AddPart(FVector(40.f, 0.f, 0.f), FVector(0.015f, 0.05f, 0.05f), Trim, CylinderMesh);
	// Body housing
	AddPart(FVector(10.f, 0.f, 0.f), FVector(0.2f, 0.07f, 0.06f), Body);
	// Drum magazine (cylinder below body)
	AddPart(FVector(12.f, 0.f, -4.f), FVector(0.06f, 0.05f, 0.05f), Dark, CylinderMesh);
	// Drum cap
	AddPart(FVector(12.f, 0.f, -7.f), FVector(0.003f, 0.04f, 0.04f), Trim, CylinderMesh);
	// Grip
	AddPart(FVector(-2.f, 0.f, -4.f), FVector(0.04f, 0.04f, 0.07f), Body);
	// Front grip
	AddPart(FVector(20.f, 0.f, -3.f), FVector(0.03f, 0.03f, 0.05f), Dark);
	// Warning stripe — hazard orange accent
	AddPart(FVector(18.f, 0.f, 3.5f), FVector(0.15f, 0.075f, 0.004f), Accent);
	// Side panel
	AddPart(FVector(8.f, 3.5f, 0.f), FVector(0.1f, 0.003f, 0.04f), Trim);
}

void UExoWeaponViewModel::BuildMeleeModel(const FLinearColor& Accent)
{
	FLinearColor Blade(0.15f, 0.15f, 0.18f);
	FLinearColor Grip(0.06f, 0.06f, 0.07f);

	// Main blade
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.25f, 0.005f, 0.04f), Blade);
	// Blade edge — glowing accent
	AddPart(FVector(20.f, 0.f, 2.5f), FVector(0.24f, 0.003f, 0.002f), Accent);
	// Blade spine (thicker back edge)
	AddPart(FVector(20.f, 0.f, -2.f), FVector(0.24f, 0.008f, 0.003f), Grip);
	// Cross guard
	AddPart(FVector(5.f, 0.f, 0.f), FVector(0.015f, 0.06f, 0.015f), Grip);
	// Guard accent dots
	AddPart(FVector(5.f, 3.f, 0.f), FVector(0.005f, 0.005f, 0.005f), Accent);
	AddPart(FVector(5.f, -3.f, 0.f), FVector(0.005f, 0.005f, 0.005f), Accent);
	// Grip handle (cylinder)
	AddPart(FVector(-5.f, 0.f, 0.f), FVector(0.08f, 0.025f, 0.025f), Grip, CylinderMesh);
	// Pommel
	AddPart(FVector(-10.f, 0.f, 0.f), FVector(0.015f, 0.03f, 0.03f), Blade);
}
