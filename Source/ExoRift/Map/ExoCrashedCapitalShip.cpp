// ExoCrashedCapitalShip.cpp — Massive crashed ship landmark
#include "Map/ExoCrashedCapitalShip.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoCrashedCapitalShip::AExoCrashedCapitalShip()
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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatF.Succeeded()) BaseMaterial = MatF.Object;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

UStaticMeshComponent* AExoCrashedCapitalShip::AddHullSection(
	const FVector& Pos, const FVector& Scale, const FRotator& Rot,
	const FLinearColor& Color)
{
	if (!CubeMesh || !BaseMaterial) return nullptr;

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	Part->SetupAttachment(RootComponent);
	Part->SetStaticMesh(CubeMesh);
	Part->SetRelativeLocation(Pos);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rot);
	Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Part->SetCollisionResponseToAllChannels(ECR_Block);
	Part->CastShadow = true;
	Part->RegisterComponent();

	UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Part->SetMaterial(0, Mat);
	HullParts.Add(Part);
	return Part;
}

UStaticMeshComponent* AExoCrashedCapitalShip::AddDamageGlow(
	const FVector& Pos, const FVector& Scale, const FLinearColor& Color)
{
	if (!SphereMesh) return nullptr;

	UStaticMeshComponent* Glow = NewObject<UStaticMeshComponent>(this);
	Glow->SetupAttachment(RootComponent);
	Glow->SetStaticMesh(SphereMesh);
	Glow->SetRelativeLocation(Pos);
	Glow->SetRelativeScale3D(Scale);
	Glow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Glow->CastShadow = false;
	Glow->RegisterComponent();

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	Glow->SetMaterial(0, Mat);
	DamageGlows.Add(Glow);
	DamageMats.Add(Mat);
	DamageBaseColors.Add(Color);

	// Light at damage point
	UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
	Light->SetupAttachment(RootComponent);
	Light->SetRelativeLocation(Pos);
	Light->SetIntensity(8000.f);
	Light->SetAttenuationRadius(2000.f);
	Light->SetLightColor(FLinearColor(Color.R, Color.G, Color.B));
	Light->CastShadows = false;
	Light->RegisterComponent();
	DamageLights.Add(Light);

	return Glow;
}

void AExoCrashedCapitalShip::BuildShip()
{
	FLinearColor HullDark(0.04f, 0.04f, 0.06f);
	FLinearColor HullMid(0.06f, 0.06f, 0.08f);
	FLinearColor HullLight(0.08f, 0.08f, 0.1f);
	FLinearColor ArmorPlate(0.05f, 0.055f, 0.07f);

	// === MAIN HULL — massive elongated box, tilted from impact ===
	// Forward section (nose, partially buried)
	AddHullSection(FVector(0.f, 0.f, 500.f),
		FVector(80.f, 30.f, 15.f), FRotator(-8.f, 0.f, 3.f), HullDark);

	// Mid section (widest, main deck)
	AddHullSection(FVector(-4000.f, 0.f, 1200.f),
		FVector(60.f, 35.f, 18.f), FRotator(-5.f, 2.f, 5.f), HullMid);

	// Rear section (engine block, elevated from tilt)
	AddHullSection(FVector(-9000.f, 0.f, 2000.f),
		FVector(50.f, 28.f, 20.f), FRotator(-3.f, -1.f, 7.f), HullDark);

	// === BRIDGE TOWER — on top of mid section ===
	AddHullSection(FVector(-3000.f, 0.f, 3200.f),
		FVector(15.f, 12.f, 10.f), FRotator(-5.f, 2.f, 4.f), HullLight);

	// Bridge viewport (thin dark strip)
	AddHullSection(FVector(-2500.f, 0.f, 3800.f),
		FVector(10.f, 13.f, 1.f), FRotator(-10.f, 2.f, 4.f),
		FLinearColor(0.02f, 0.02f, 0.03f));

	// === WING SECTIONS — broken, angled from crash ===
	// Port wing (intact but bent)
	AddHullSection(FVector(-2000.f, 4500.f, 800.f),
		FVector(40.f, 60.f, 3.f), FRotator(-3.f, 5.f, -15.f), ArmorPlate);

	// Starboard wing (broken, sticking up)
	AddHullSection(FVector(-2000.f, -3500.f, 1500.f),
		FVector(35.f, 45.f, 3.f), FRotator(-3.f, -8.f, 25.f), ArmorPlate);

	// === ENGINE NACELLES — cylinders at rear ===
	if (CylinderMesh)
	{
		auto AddEngine = [&](const FVector& Pos, const FRotator& Rot)
		{
			UStaticMeshComponent* Eng = NewObject<UStaticMeshComponent>(this);
			Eng->SetupAttachment(RootComponent);
			Eng->SetStaticMesh(CylinderMesh);
			Eng->SetRelativeLocation(Pos);
			Eng->SetRelativeScale3D(FVector(12.f, 12.f, 25.f));
			Eng->SetRelativeRotation(Rot + FRotator(0.f, 0.f, 90.f));
			Eng->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Eng->SetCollisionResponseToAllChannels(ECR_Block);
			Eng->CastShadow = true;
			Eng->RegisterComponent();
			if (BaseMaterial)
			{
				UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(BaseMaterial, this);
				M->SetVectorParameterValue(TEXT("BaseColor"), HullDark);
				Eng->SetMaterial(0, M);
			}
			HullParts.Add(Eng);
		};
		AddEngine(FVector(-11000.f, 2000.f, 2200.f), FRotator(-5.f, 0.f, 8.f));
		AddEngine(FVector(-11000.f, -2000.f, 2200.f), FRotator(-5.f, 0.f, 8.f));
	}

	// === DAMAGE GLOW POINTS — smoldering impact damage ===
	FLinearColor FireGlow(8.f, 3.f, 0.5f);
	FLinearColor PlasmaGlow(2.f, 5.f, 10.f);

	// Impact crater at nose (buried in ground)
	AddDamageGlow(FVector(3000.f, 0.f, 200.f), FVector(15.f, 10.f, 5.f), FireGlow);
	// Hull breach midship (port side)
	AddDamageGlow(FVector(-3500.f, 1800.f, 1500.f), FVector(6.f, 4.f, 4.f), FireGlow);
	// Engine damage (plasma leak)
	AddDamageGlow(FVector(-11500.f, 2000.f, 2200.f), FVector(8.f, 8.f, 5.f), PlasmaGlow);
	// Starboard wing break point
	AddDamageGlow(FVector(-1500.f, -2500.f, 1000.f), FVector(5.f, 5.f, 3.f), FireGlow);
	// Bridge damage
	AddDamageGlow(FVector(-2800.f, 500.f, 3400.f), FVector(3.f, 3.f, 3.f), PlasmaGlow);

	// === SCATTERED DEBRIS around crash site ===
	FVector DebrisPositions[] = {
		{6000.f, 2000.f, 100.f}, {5000.f, -3000.f, 100.f},
		{-1000.f, 5000.f, 100.f}, {-6000.f, -4000.f, 200.f},
		{-12000.f, 3000.f, 300.f}, {7000.f, -1000.f, 50.f},
		{3000.f, 4000.f, 80.f}, {-8000.f, 5000.f, 150.f},
	};
	for (int32 i = 0; i < 8; i++)
	{
		float S = 3.f + (i % 4) * 2.f;
		FRotator Rot(FMath::RandRange(-20.f, 20.f),
			FMath::RandRange(0.f, 360.f),
			FMath::RandRange(-15.f, 15.f));
		AddHullSection(DebrisPositions[i],
			FVector(S, S * 0.6f, S * 0.3f), Rot, ArmorPlate);
	}

	// === GROUND SCORCH — dark impact trail ===
	AddHullSection(FVector(2000.f, 0.f, 2.f),
		FVector(120.f, 25.f, 0.1f), FRotator(0.f, 0.f, 0.f),
		FLinearColor(0.02f, 0.02f, 0.025f));
}

void AExoCrashedCapitalShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Animate damage glow points — flickering fire/plasma
	for (int32 i = 0; i < DamageMats.Num(); i++)
	{
		if (!DamageMats[i]) continue;
		float Phase = Time * (2.f + i * 0.7f);
		float Flicker = 0.6f + 0.4f * FMath::Abs(FMath::Sin(Phase))
			+ 0.2f * FMath::Sin(Phase * 3.7f);

		FLinearColor Base = DamageBaseColors.IsValidIndex(i) ? DamageBaseColors[i] : FLinearColor::White;
		DamageMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Base.R * Flicker, Base.G * Flicker, Base.B * Flicker));

		if (DamageLights.IsValidIndex(i) && DamageLights[i])
			DamageLights[i]->SetIntensity(8000.f * Flicker);
	}
}
