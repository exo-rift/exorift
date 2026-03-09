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
		? FLinearColor(0.12f, 0.08f, 0.08f)  // Dark red-tint for bots
		: FLinearColor(0.08f, 0.08f, 0.12f);  // Dark blue-tint for players

	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Accent = bIsBot
		? FLinearColor(0.8f, 0.2f, 0.1f)
		: FLinearColor(0.1f, 0.5f, 1.f);

	// Torso (main body block)
	AddPart(FVector(0.f, 0.f, 20.f), FVector(0.35f, 0.2f, 0.4f), Body);

	// Shoulder pads
	AddPart(FVector(0.f, 22.f, 35.f), FVector(0.15f, 0.12f, 0.1f), Dark);
	AddPart(FVector(0.f, -22.f, 35.f), FVector(0.15f, 0.12f, 0.1f), Dark);

	// Head (sphere)
	AddPart(FVector(0.f, 0.f, 55.f), FVector(0.18f, 0.18f, 0.2f), Dark, SphereMesh);

	// Visor (thin rectangle across face)
	AddPart(FVector(10.f, 0.f, 55.f), FVector(0.03f, 0.15f, 0.05f), Accent);

	// Chest accent strip (glowing)
	AccentPart = AddPart(FVector(18.f, 0.f, 22.f), FVector(0.02f, 0.18f, 0.06f), Accent);

	// Back plate
	AddPart(FVector(-18.f, 0.f, 22.f), FVector(0.05f, 0.18f, 0.3f), Dark);

	// Upper arms
	AddPart(FVector(0.f, 28.f, 15.f), FVector(0.1f, 0.08f, 0.25f), Body, CylinderMesh);
	AddPart(FVector(0.f, -28.f, 15.f), FVector(0.1f, 0.08f, 0.25f), Body, CylinderMesh);

	// Forearms
	AddPart(FVector(5.f, 30.f, -10.f), FVector(0.08f, 0.06f, 0.2f), Dark, CylinderMesh);
	AddPart(FVector(5.f, -30.f, -10.f), FVector(0.08f, 0.06f, 0.2f), Dark, CylinderMesh);

	// Upper legs
	AddPart(FVector(0.f, 10.f, -25.f), FVector(0.12f, 0.1f, 0.3f), Body, CylinderMesh);
	AddPart(FVector(0.f, -10.f, -25.f), FVector(0.12f, 0.1f, 0.3f), Body, CylinderMesh);

	// Lower legs
	AddPart(FVector(0.f, 10.f, -55.f), FVector(0.09f, 0.08f, 0.25f), Dark, CylinderMesh);
	AddPart(FVector(0.f, -10.f, -55.f), FVector(0.09f, 0.08f, 0.25f), Dark, CylinderMesh);

	// Boots
	AddPart(FVector(5.f, 10.f, -75.f), FVector(0.14f, 0.1f, 0.08f), Dark);
	AddPart(FVector(5.f, -10.f, -75.f), FVector(0.14f, 0.1f, 0.08f), Dark);

	// Small ID light on chest — helps spot enemies at distance
	IdentityLight = NewObject<UPointLightComponent>(GetOwner());
	IdentityLight->SetupAttachment(this);
	IdentityLight->SetRelativeLocation(FVector(15.f, 0.f, 25.f));
	IdentityLight->SetIntensity(1000.f);
	IdentityLight->SetAttenuationRadius(200.f);
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
	Part->SetOwnerNoSee(false); // Visible to everyone

	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, GetOwner());
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Part->SetMaterial(0, Mat);
	}

	Part->RegisterComponent();
	return Part;
}
