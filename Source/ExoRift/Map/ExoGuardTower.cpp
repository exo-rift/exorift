// ExoGuardTower.cpp — Climbable vantage point structure
#include "Map/ExoGuardTower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoGuardTower::AExoGuardTower()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubF.Succeeded()) CubeMesh = CubF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphF.Succeeded()) SphereMesh = SphF.Object;

	// Materials created at runtime via FExoMaterialFactory

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

UStaticMeshComponent* AExoGuardTower::AddPart(
	const FVector& Pos, const FVector& Scale, const FRotator& Rot,
	UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	Part->SetupAttachment(RootComponent);
	Part->SetStaticMesh(Mesh);
	Part->SetRelativeLocation(Pos);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rot);
	Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Part->SetCollisionResponseToAllChannels(ECR_Block);
	Part->CastShadow = true;
	Part->RegisterComponent();

	float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
	if (Lum > 0.15f)
	{
		UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();
		if (EmMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmMat, this);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 2.f, Color.G * 2.f, Color.B * 2.f));
			Part->SetMaterial(0, Mat);
		}
	}
	else
	{
		UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
		if (LitMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.88f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.28f);
			Part->SetMaterial(0, Mat);
		}
	}
	Parts.Add(Part);
	return Part;
}

void AExoGuardTower::BuildTower()
{
	FLinearColor Steel(0.06f, 0.06f, 0.08f);
	FLinearColor DarkSteel(0.04f, 0.04f, 0.05f);
	FLinearColor Grate(0.07f, 0.07f, 0.08f);
	FLinearColor Accent(0.1f, 0.08f, 0.06f);

	float DeckHeight = 800.f;

	// === 4 CORNER LEGS — structural support pillars ===
	float LegSpread = 200.f;
	float LegInset = 150.f; // Legs taper inward at top
	for (int32 i = 0; i < 4; i++)
	{
		float X = (i < 2) ? -LegSpread : LegSpread;
		float Y = (i % 2 == 0) ? -LegSpread : LegSpread;
		float TopX = (i < 2) ? -LegInset : LegInset;
		float TopY = (i % 2 == 0) ? -LegInset : LegInset;
		FVector Bot(X, Y, 0.f);
		FVector Top(TopX, TopY, DeckHeight);
		FVector Mid = (Bot + Top) * 0.5f;
		FVector Dir = Top - Bot;
		FRotator Rot = Dir.Rotation();
		float Len = Dir.Size();
		AddPart(Mid, FVector(0.3f, 0.3f, Len / 100.f),
			FRotator(Rot.Pitch + 90.f, Rot.Yaw, 0.f), CylinderMesh, Steel);
	}

	// === CROSS BRACING — X-braces between legs ===
	auto AddBrace = [&](const FVector& From, const FVector& To)
	{
		FVector Mid = (From + To) * 0.5f;
		FVector Dir = To - From;
		float Len = Dir.Size();
		FRotator Rot = Dir.Rotation();
		AddPart(Mid, FVector(0.1f, 0.1f, Len / 100.f),
			FRotator(Rot.Pitch + 90.f, Rot.Yaw, 0.f), CylinderMesh, DarkSteel);
	};
	float BraceH = DeckHeight * 0.5f;
	AddBrace(FVector(-LegSpread, -LegSpread, 0.f), FVector(LegSpread, -LegSpread, BraceH));
	AddBrace(FVector(LegSpread, -LegSpread, 0.f), FVector(-LegSpread, -LegSpread, BraceH));
	AddBrace(FVector(-LegSpread, LegSpread, 0.f), FVector(LegSpread, LegSpread, BraceH));
	AddBrace(FVector(LegSpread, LegSpread, 0.f), FVector(-LegSpread, LegSpread, BraceH));

	// === OBSERVATION DECK — walkable platform ===
	AddPart(FVector(0.f, 0.f, DeckHeight), FVector(4.f, 4.f, 0.1f),
		FRotator::ZeroRotator, CubeMesh, Grate);

	// === RAILING — chest-high walls around deck ===
	float RailH = 100.f;
	float RailTop = DeckHeight + RailH * 0.5f + 5.f;
	AddPart(FVector(0.f, -LegInset, RailTop), FVector(3.2f, 0.05f, RailH / 100.f),
		FRotator::ZeroRotator, CubeMesh, Steel);
	AddPart(FVector(0.f, LegInset, RailTop), FVector(3.2f, 0.05f, RailH / 100.f),
		FRotator::ZeroRotator, CubeMesh, Steel);
	AddPart(FVector(-LegInset, 0.f, RailTop), FVector(0.05f, 3.2f, RailH / 100.f),
		FRotator::ZeroRotator, CubeMesh, Steel);
	// Front railing gap (opening for stairs)
	AddPart(FVector(LegInset, -80.f, RailTop), FVector(0.05f, 0.8f, RailH / 100.f),
		FRotator::ZeroRotator, CubeMesh, Steel);
	AddPart(FVector(LegInset, 80.f, RailTop), FVector(0.05f, 0.8f, RailH / 100.f),
		FRotator::ZeroRotator, CubeMesh, Steel);

	// === STAIRWAY — ramp leading up to deck ===
	int32 Steps = 8;
	for (int32 i = 0; i < Steps; i++)
	{
		float T = (float)i / Steps;
		float StepH = T * DeckHeight;
		float StepX = LegInset + 50.f + (1.f - T) * 400.f;
		AddPart(FVector(StepX, 0.f, StepH + 15.f),
			FVector(0.8f, 1.2f, 0.15f), FRotator::ZeroRotator, CubeMesh, Grate);
	}
	// Stair rail posts
	AddPart(FVector(LegInset + 450.f, -60.f, 30.f), FVector(0.1f, 0.1f, 0.6f),
		FRotator::ZeroRotator, CylinderMesh, Steel);
	AddPart(FVector(LegInset + 450.f, 60.f, 30.f), FVector(0.1f, 0.1f, 0.6f),
		FRotator::ZeroRotator, CylinderMesh, Steel);

	// === ROOF CANOPY — partial cover overhead ===
	AddPart(FVector(-30.f, 0.f, DeckHeight + 250.f), FVector(3.f, 3.5f, 0.08f),
		FRotator(3.f, 0.f, 0.f), CubeMesh, DarkSteel);
	// Roof support posts
	AddPart(FVector(-LegInset + 20.f, -LegInset + 20.f, DeckHeight + 125.f),
		FVector(0.15f, 0.15f, 2.5f), FRotator::ZeroRotator, CylinderMesh, Steel);
	AddPart(FVector(-LegInset + 20.f, LegInset - 20.f, DeckHeight + 125.f),
		FVector(0.15f, 0.15f, 2.5f), FRotator::ZeroRotator, CylinderMesh, Steel);

	// === SEARCHLIGHT — mounted on roof edge ===
	USpotLightComponent* Spot = NewObject<USpotLightComponent>(this);
	Spot->SetupAttachment(RootComponent);
	Spot->SetRelativeLocation(FVector(LegInset, 0.f, DeckHeight + 260.f));
	Spot->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	Spot->SetIntensity(40000.f);
	Spot->SetAttenuationRadius(3000.f);
	Spot->SetOuterConeAngle(30.f);
	Spot->SetInnerConeAngle(18.f);
	Spot->SetLightColor(FLinearColor(0.8f, 0.85f, 1.f));
	Spot->CastShadows = false;
	Spot->RegisterComponent();

	// Deck ambient light
	SearchLight = NewObject<UPointLightComponent>(this);
	SearchLight->SetupAttachment(RootComponent);
	SearchLight->SetRelativeLocation(FVector(0.f, 0.f, DeckHeight + 50.f));
	SearchLight->SetIntensity(3000.f);
	SearchLight->SetAttenuationRadius(600.f);
	SearchLight->SetLightColor(FLinearColor(0.7f, 0.75f, 0.9f));
	SearchLight->CastShadows = false;
	SearchLight->RegisterComponent();

	// Warning stripe on deck edge
	AddPart(FVector(LegInset, 0.f, DeckHeight + 2.f),
		FVector(0.08f, 3.f, 0.02f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.6f, 0.4f, 0.05f));
}

void AExoGuardTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Subtle light pulse
	if (SearchLight)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Pulse = 0.85f + 0.15f * FMath::Sin(Time * 1.5f);
		SearchLight->SetIntensity(3000.f * Pulse);
	}
}
