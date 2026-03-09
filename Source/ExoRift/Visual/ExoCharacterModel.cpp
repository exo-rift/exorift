#include "Visual/ExoCharacterModel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UExoCharacterModel::UExoCharacterModel()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphFind.Succeeded()) SphereMesh = SphFind.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded()) BaseMaterial = MatFind.Object;
}

void UExoCharacterModel::BuildModel(bool bIsBot)
{
	FLinearColor Body = bIsBot
		? FLinearColor(0.12f, 0.08f, 0.08f)
		: FLinearColor(0.08f, 0.08f, 0.12f);

	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor DarkGrey(0.06f, 0.06f, 0.07f);
	FLinearColor Accent = bIsBot
		? FLinearColor(0.8f, 0.2f, 0.1f)
		: FLinearColor(0.1f, 0.5f, 1.f);

	// Torso
	AddPart(FVector(0.f, 0.f, 20.f), FVector(0.35f, 0.2f, 0.4f), Body);

	// Shoulder pads (armored)
	AddPart(FVector(0.f, 22.f, 35.f), FVector(0.15f, 0.12f, 0.1f), Dark);
	AddPart(FVector(0.f, -22.f, 35.f), FVector(0.15f, 0.12f, 0.1f), Dark);
	// Shoulder accent strips
	AddPart(FVector(0.f, 22.f, 40.f), FVector(0.12f, 0.03f, 0.02f), Accent);
	AddPart(FVector(0.f, -22.f, 40.f), FVector(0.12f, 0.03f, 0.02f), Accent);

	// Head (sphere)
	AddPart(FVector(0.f, 0.f, 55.f), FVector(0.18f, 0.18f, 0.2f), Dark, SphereMesh);

	// Visor (wider, glowing)
	AddPart(FVector(10.f, 0.f, 55.f), FVector(0.03f, 0.15f, 0.05f), Accent);
	// Visor glow — illuminates face area
	UPointLightComponent* VisorGlow = NewObject<UPointLightComponent>(GetOwner());
	VisorGlow->SetupAttachment(this);
	VisorGlow->SetRelativeLocation(FVector(12.f, 0.f, 55.f));
	VisorGlow->SetIntensity(600.f);
	VisorGlow->SetAttenuationRadius(80.f);
	VisorGlow->SetLightColor(Accent);
	VisorGlow->CastShadows = false;
	VisorGlow->RegisterComponent();

	// Antenna on right side of helmet
	AddPart(FVector(-5.f, 10.f, 68.f), FVector(0.01f, 0.01f, 0.1f), DarkGrey);
	// Antenna tip (small accent dot)
	AddPart(FVector(-5.f, 10.f, 78.f), FVector(0.02f, 0.02f, 0.02f), Accent, SphereMesh);

	// Chest accent strip (glowing)
	AccentPart = AddPart(FVector(18.f, 0.f, 22.f), FVector(0.02f, 0.18f, 0.06f), Accent);

	// Back plate
	AddPart(FVector(-18.f, 0.f, 22.f), FVector(0.05f, 0.18f, 0.3f), Dark);
	// Backpack unit
	AddPart(FVector(-22.f, 0.f, 15.f), FVector(0.08f, 0.12f, 0.18f), DarkGrey);
	// Backpack accent
	AddPart(FVector(-26.f, 0.f, 18.f), FVector(0.02f, 0.08f, 0.03f), Accent);

	// Belt
	AddPart(FVector(0.f, 0.f, -2.f), FVector(0.32f, 0.22f, 0.04f), DarkGrey);
	// Belt buckle
	AddPart(FVector(16.f, 0.f, -2.f), FVector(0.04f, 0.06f, 0.04f), Accent);

	// Upper arms
	AddPart(FVector(0.f, 28.f, 15.f), FVector(0.1f, 0.08f, 0.25f), Body, CylinderMesh);
	AddPart(FVector(0.f, -28.f, 15.f), FVector(0.1f, 0.08f, 0.25f), Body, CylinderMesh);

	// Forearms (armored)
	AddPart(FVector(5.f, 30.f, -10.f), FVector(0.08f, 0.06f, 0.2f), Dark, CylinderMesh);
	AddPart(FVector(5.f, -30.f, -10.f), FVector(0.08f, 0.06f, 0.2f), Dark, CylinderMesh);
	// Forearm accent strips
	AddPart(FVector(5.f, 34.f, -8.f), FVector(0.02f, 0.02f, 0.12f), Accent);
	AddPart(FVector(5.f, -34.f, -8.f), FVector(0.02f, 0.02f, 0.12f), Accent);

	// Upper legs
	AddPart(FVector(0.f, 10.f, -25.f), FVector(0.12f, 0.1f, 0.3f), Body, CylinderMesh);
	AddPart(FVector(0.f, -10.f, -25.f), FVector(0.12f, 0.1f, 0.3f), Body, CylinderMesh);

	// Knee pads
	AddPart(FVector(6.f, 10.f, -42.f), FVector(0.06f, 0.08f, 0.06f), DarkGrey);
	AddPart(FVector(6.f, -10.f, -42.f), FVector(0.06f, 0.08f, 0.06f), DarkGrey);

	// Lower legs
	AddPart(FVector(0.f, 10.f, -55.f), FVector(0.09f, 0.08f, 0.25f), Dark, CylinderMesh);
	AddPart(FVector(0.f, -10.f, -55.f), FVector(0.09f, 0.08f, 0.25f), Dark, CylinderMesh);

	// Boots (heavier)
	AddPart(FVector(5.f, 10.f, -75.f), FVector(0.14f, 0.1f, 0.08f), Dark);
	AddPart(FVector(5.f, -10.f, -75.f), FVector(0.14f, 0.1f, 0.08f), Dark);
	// Boot soles (slightly lighter)
	AddPart(FVector(5.f, 10.f, -80.f), FVector(0.15f, 0.11f, 0.03f), DarkGrey);
	AddPart(FVector(5.f, -10.f, -80.f), FVector(0.15f, 0.11f, 0.03f), DarkGrey);

	// Small ID light on chest
	IdentityLight = NewObject<UPointLightComponent>(GetOwner());
	IdentityLight->SetupAttachment(this);
	IdentityLight->SetRelativeLocation(FVector(15.f, 0.f, 25.f));
	IdentityLight->SetIntensity(1500.f);
	IdentityLight->SetAttenuationRadius(250.f);
	IdentityLight->SetLightColor(Accent);
	IdentityLight->CastShadows = false;
	IdentityLight->RegisterComponent();
}

void UExoCharacterModel::SetAccentColor(const FLinearColor& Color)
{
	if (AccentPart && BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, GetOwner());
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		AccentPart->SetMaterial(0, Mat);
	}
	if (IdentityLight)
	{
		IdentityLight->SetLightColor(Color);
	}
}

UStaticMeshComponent* UExoCharacterModel::AddPart(const FVector& Offset,
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
	Part->CastShadow = true;
	Part->SetGenerateOverlapEvents(false);
	Part->SetOwnerNoSee(false);

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, GetOwner());
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		// Bright accent colors get emissive glow
		float Luminance = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Luminance > 0.15f)
		{
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 1.5f, Color.G * 1.5f, Color.B * 1.5f));
		}
		Part->SetMaterial(0, Mat);
	}

	Part->RegisterComponent();
	return Part;
}
