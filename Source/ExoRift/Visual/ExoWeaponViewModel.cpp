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
	case EWeaponType::Rifle:  BuildRifleModel(RarityColor); break;
	case EWeaponType::SMG:    BuildSMGModel(RarityColor); break;
	case EWeaponType::Shotgun: BuildShotgunModel(RarityColor); break;
	case EWeaponType::Sniper: BuildSniperModel(RarityColor); break;
	case EWeaponType::Melee:  BuildMeleeModel(RarityColor); break;
	default: BuildRifleModel(RarityColor); break;
	}

	// Muzzle ready indicator light
	MuzzleReadyLight = NewObject<UPointLightComponent>(GetOwner());
	MuzzleReadyLight->SetupAttachment(this);
	FVector MuzzlePos = MuzzleTip ? MuzzleTip->GetRelativeLocation() : FVector(40.f, 0.f, 0.f);
	MuzzleReadyLight->SetRelativeLocation(MuzzlePos);
	MuzzleReadyLight->SetIntensity(500.f);
	MuzzleReadyLight->SetAttenuationRadius(80.f);
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
		Part->SetMaterial(0, Mat);
	}

	Part->RegisterComponent();
	return Part;
}

void UExoWeaponViewModel::BuildRifleModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.08f, 0.08f, 0.1f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);

	// Main body
	AddPart(FVector(15.f, 0.f, 0.f), FVector(0.35f, 0.06f, 0.05f), Body);
	// Barrel
	MuzzleTip = AddPart(FVector(40.f, 0.f, 0.5f), FVector(0.15f, 0.02f, 0.02f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Magazine
	AddPart(FVector(10.f, 0.f, -4.f), FVector(0.06f, 0.03f, 0.08f), Dark);
	// Grip
	AddPart(FVector(0.f, 0.f, -3.f), FVector(0.04f, 0.04f, 0.06f), Body);
	// Accent stripe
	AddPart(FVector(20.f, 0.f, 2.f), FVector(0.2f, 0.065f, 0.005f), Accent);
	// Scope rail
	AddPart(FVector(18.f, 0.f, 3.5f), FVector(0.12f, 0.015f, 0.01f), Dark);
}

void UExoWeaponViewModel::BuildSMGModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.07f, 0.07f, 0.09f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);

	// Compact body
	AddPart(FVector(10.f, 0.f, 0.f), FVector(0.22f, 0.055f, 0.045f), Body);
	// Short barrel
	MuzzleTip = AddPart(FVector(28.f, 0.f, 0.5f), FVector(0.08f, 0.018f, 0.018f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Magazine (straight)
	AddPart(FVector(8.f, 0.f, -4.f), FVector(0.04f, 0.025f, 0.07f), Dark);
	// Grip
	AddPart(FVector(-2.f, 0.f, -3.f), FVector(0.035f, 0.035f, 0.05f), Body);
	// Accent band
	AddPart(FVector(15.f, 0.f, 2.f), FVector(0.15f, 0.06f, 0.004f), Accent);
}

void UExoWeaponViewModel::BuildShotgunModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.1f, 0.08f, 0.07f);
	FLinearColor Dark(0.05f, 0.04f, 0.04f);

	// Thick body
	AddPart(FVector(12.f, 0.f, 0.f), FVector(0.3f, 0.07f, 0.06f), Body);
	// Wide barrel
	MuzzleTip = AddPart(FVector(38.f, 0.f, 0.f), FVector(0.1f, 0.03f, 0.03f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Pump
	AddPart(FVector(22.f, 0.f, -1.5f), FVector(0.08f, 0.04f, 0.035f), Dark);
	// Grip
	AddPart(FVector(-2.f, 0.f, -4.f), FVector(0.04f, 0.04f, 0.07f), Body);
	// Stock
	AddPart(FVector(-12.f, 0.f, 0.f), FVector(0.1f, 0.04f, 0.04f), Dark);
	// Accent
	AddPart(FVector(8.f, 0.f, 3.f), FVector(0.08f, 0.075f, 0.004f), Accent);
}

void UExoWeaponViewModel::BuildSniperModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.06f, 0.06f, 0.08f);
	FLinearColor Dark(0.03f, 0.03f, 0.04f);

	// Long body
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.45f, 0.05f, 0.04f), Body);
	// Long barrel
	MuzzleTip = AddPart(FVector(52.f, 0.f, 0.5f), FVector(0.2f, 0.015f, 0.015f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Scope
	AddPart(FVector(22.f, 0.f, 4.f), FVector(0.08f, 0.025f, 0.025f), Dark, CylinderMesh);
	// Magazine
	AddPart(FVector(15.f, 0.f, -4.f), FVector(0.04f, 0.025f, 0.06f), Dark);
	// Grip
	AddPart(FVector(0.f, 0.f, -3.5f), FVector(0.035f, 0.035f, 0.06f), Body);
	// Stock
	AddPart(FVector(-15.f, 0.f, 0.f), FVector(0.12f, 0.04f, 0.05f), Dark);
	// Accent line
	AddPart(FVector(25.f, 0.f, 2.f), FVector(0.3f, 0.055f, 0.003f), Accent);
}

void UExoWeaponViewModel::BuildMeleeModel(const FLinearColor& Accent)
{
	FLinearColor Blade(0.15f, 0.15f, 0.18f);
	FLinearColor Grip(0.06f, 0.06f, 0.07f);

	// Blade
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.25f, 0.005f, 0.04f), Blade);
	// Guard
	AddPart(FVector(5.f, 0.f, 0.f), FVector(0.015f, 0.06f, 0.015f), Grip);
	// Grip handle
	AddPart(FVector(-5.f, 0.f, 0.f), FVector(0.08f, 0.025f, 0.025f), Grip, CylinderMesh);
	// Edge glow
	AddPart(FVector(20.f, 0.f, 2.5f), FVector(0.24f, 0.003f, 0.002f), Accent);
}
