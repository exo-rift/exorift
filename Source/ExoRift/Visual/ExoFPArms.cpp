// ExoFPArms.cpp — Procedural first-person arm geometry
#include "Visual/ExoFPArms.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UExoFPArms::UExoFPArms()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeF.Succeeded()) CubeMesh = CubeF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphF.Succeeded()) SphereMesh = SphF.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatF.Succeeded()) BaseMaterial = MatF.Object;
}

UStaticMeshComponent* UExoFPArms::AddPart(const FVector& Offset, const FVector& Scale,
	const FLinearColor& Color, const FRotator& Rot, UStaticMesh* Mesh)
{
	if (!Mesh) Mesh = CubeMesh;
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(GetOwner());
	Part->SetupAttachment(this);
	Part->SetStaticMesh(Mesh);
	Part->SetRelativeLocation(Offset);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rot);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->CastShadow = false;
	Part->SetGenerateOverlapEvents(false);

	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, GetOwner());
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);

		float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Lum > 0.12f)
		{
			// Accent/display parts: strong emissive for glow
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 15.f, Color.G * 15.f, Color.B * 15.f));
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.3f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.1f);
		}
		else
		{
			// Suit/glove parts: matte tactical fabric, slightly rough
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.15f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.6f);
		}
		Part->SetMaterial(0, Mat);
	}

	Part->RegisterComponent();
	return Part;
}

void UExoFPArms::BuildArms(const FLinearColor& SuitColor)
{
	FLinearColor Suit = SuitColor;
	FLinearColor SuitDark(Suit.R * 0.5f, Suit.G * 0.5f, Suit.B * 0.5f);
	FLinearColor Glove(0.04f, 0.04f, 0.05f);     // Black tactical gloves
	FLinearColor Skin(0.55f, 0.42f, 0.32f);       // Not visible (under suit)
	FLinearColor Accent(0.1f, 0.4f, 0.8f);         // Suit accent stripe

	// === RIGHT ARM (trigger hand, closer to camera) ===

	// Right forearm — suit sleeve
	AddPart(FVector(-12.f, 5.f, -6.f), FVector(0.14f, 0.04f, 0.035f), Suit,
		FRotator(0.f, -5.f, 0.f), CylinderMesh);
	// Right forearm armor plate
	AddPart(FVector(-10.f, 5.f, -4.f), FVector(0.1f, 0.045f, 0.01f), SuitDark);
	// Right wrist joint
	AddPart(FVector(-3.f, 5.f, -5.5f), FVector(0.025f, 0.03f, 0.03f), SuitDark, FRotator::ZeroRotator, SphereMesh);
	// Right hand (glove)
	AddPart(FVector(0.f, 4.f, -4.f), FVector(0.04f, 0.035f, 0.02f), Glove);
	// Right thumb
	AddPart(FVector(0.f, 2.5f, -3.5f), FVector(0.015f, 0.015f, 0.01f), Glove);
	// Right fingers (wrapped around grip area)
	AddPart(FVector(1.f, 4.f, -5.f), FVector(0.025f, 0.03f, 0.012f), Glove);
	AddPart(FVector(2.5f, 4.f, -5.2f), FVector(0.015f, 0.025f, 0.01f), Glove);
	// Right arm accent stripe
	AddPart(FVector(-8.f, 5.f, -3.5f), FVector(0.08f, 0.003f, 0.005f), Accent);

	// === LEFT ARM (support hand, further away) ===

	// Left forearm — suit sleeve
	AddPart(FVector(15.f, -5.f, -5.f), FVector(0.12f, 0.04f, 0.035f), Suit,
		FRotator(0.f, 10.f, 0.f), CylinderMesh);
	// Left forearm armor plate
	AddPart(FVector(17.f, -5.f, -3.f), FVector(0.09f, 0.045f, 0.01f), SuitDark);
	// Left wrist joint
	AddPart(FVector(22.f, -4.f, -4.5f), FVector(0.025f, 0.03f, 0.03f), SuitDark, FRotator::ZeroRotator, SphereMesh);
	// Left hand (glove, gripping forward)
	AddPart(FVector(24.f, -3.f, -3.5f), FVector(0.035f, 0.03f, 0.02f), Glove);
	// Left fingers (wrapped under barrel)
	AddPart(FVector(25.f, -3.f, -4.5f), FVector(0.025f, 0.025f, 0.012f), Glove);
	AddPart(FVector(26.5f, -3.f, -4.3f), FVector(0.015f, 0.02f, 0.01f), Glove);
	// Left arm accent stripe
	AddPart(FVector(18.f, -5.f, -2.5f), FVector(0.07f, 0.003f, 0.005f), Accent);

	// Wrist data display (small glowing screen on left forearm)
	AddPart(FVector(19.f, -5.5f, -3.5f), FVector(0.03f, 0.02f, 0.005f),
		FLinearColor(0.05f, 0.3f, 0.5f));
}
