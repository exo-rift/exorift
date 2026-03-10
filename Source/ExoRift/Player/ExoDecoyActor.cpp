#include "Player/ExoDecoyActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "UObject/ConstructorHelpers.h"
#include "ExoRift.h"

AExoDecoyActor::AExoDecoyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	// Body — tall cylinder for torso
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMesh;
	if (CylFinder.Succeeded()) BodyMesh->SetStaticMesh(CylFinder.Object);
	BodyMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.9f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->CastShadow = false;
	BodyMesh->SetTranslucentSortPriority(100);

	// Head — sphere on top
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(BodyMesh);
	if (SphereFinder.Succeeded()) HeadMesh->SetStaticMesh(SphereFinder.Object);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HeadMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->CastShadow = false;

	// Base disk — projector pad on ground
	BaseDisk = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseDisk"));
	BaseDisk->SetupAttachment(BodyMesh);
	if (CylFinder.Succeeded()) BaseDisk->SetStaticMesh(CylFinder.Object);
	BaseDisk->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	BaseDisk->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.02f));
	BaseDisk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseDisk->CastShadow = false;

	// Holographic glow light
	HoloLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HoloLight"));
	HoloLight->SetupAttachment(BodyMesh);
	HoloLight->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	HoloLight->SetIntensity(8000.f);
	HoloLight->SetAttenuationRadius(500.f);
	HoloLight->SetLightColor(FLinearColor(0.1f, 0.6f, 1.f));
	HoloLight->CastShadows = false;

	// AI perception — makes bots detect via sight
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
		TEXT("StimuliSource"));
	StimuliSource->bAutoRegister = true;
	StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());

	FlickerSeed = FMath::FRand() * 100.f;
}

void AExoDecoyActor::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();

	// Create hologram material — bright cyan-blue emissive
	UMaterialInterface* Base = BodyMesh->GetMaterial(0);
	if (Base)
	{
		HoloMat = UMaterialInstanceDynamic::Create(Base, this);
		FLinearColor HoloCol(0.1f, 0.5f, 1.f);
		HoloMat->SetVectorParameterValue(TEXT("BaseColor"), HoloCol);
		HoloMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.2f, 1.5f, 4.f));
		BodyMesh->SetMaterial(0, HoloMat);
		HeadMesh->SetMaterial(0, HoloMat);

		// Base disk — dimmer accent
		BaseMat = UMaterialInstanceDynamic::Create(Base, this);
		BaseMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.05f, 0.2f, 0.4f));
		BaseMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.1f, 0.8f, 2.f));
		BaseDisk->SetMaterial(0, BaseMat);
	}

	BuildDetailParts();

	UE_LOG(LogExoRift, Log, TEXT("Holographic decoy deployed at %s"),
		*GetActorLocation().ToString());
}

void AExoDecoyActor::BuildDetailParts()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	UStaticMesh* CubeMesh = CubeF.Succeeded() ? CubeF.Object : nullptr;
	UStaticMesh* CylMesh = CylF.Succeeded() ? CylF.Object : nullptr;
	UMaterialInterface* BaseMaterial = MatF.Succeeded() ? MatF.Object : nullptr;

	if (!CubeMesh || !BaseMaterial || !HoloMat) return;

	auto AddHoloPart = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot = FRotator::ZeroRotator)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetMaterial(0, HoloMat);
		C->RegisterComponent();
		DetailParts.Add(C);
	};

	// Shoulder pads
	AddHoloPart(CubeMesh, FVector(0.f, 22.f, 80.f), FVector(0.15f, 0.12f, 0.08f));
	AddHoloPart(CubeMesh, FVector(0.f, -22.f, 80.f), FVector(0.15f, 0.12f, 0.08f));

	// Upper arms
	if (CylMesh)
	{
		AddHoloPart(CylMesh, FVector(0.f, 26.f, 40.f), FVector(0.08f, 0.06f, 0.2f));
		AddHoloPart(CylMesh, FVector(0.f, -26.f, 40.f), FVector(0.08f, 0.06f, 0.2f));
	}

	// Forearms (slightly forward like holding a weapon)
	if (CylMesh)
	{
		AddHoloPart(CylMesh, FVector(5.f, 24.f, 5.f), FVector(0.06f, 0.05f, 0.18f));
		AddHoloPart(CylMesh, FVector(5.f, -24.f, 5.f), FVector(0.06f, 0.05f, 0.18f));
	}

	// Weapon silhouette (held across front)
	AddHoloPart(CubeMesh, FVector(20.f, 0.f, 10.f), FVector(0.4f, 0.06f, 0.04f));

	// Visor accent (brighter bar across face)
	AddHoloPart(CubeMesh, FVector(8.f, 0.f, 120.f), FVector(0.02f, 0.12f, 0.03f));

	// Leg outlines
	if (CylMesh)
	{
		AddHoloPart(CylMesh, FVector(0.f, 8.f, -30.f), FVector(0.1f, 0.08f, 0.3f));
		AddHoloPart(CylMesh, FVector(0.f, -8.f, -30.f), FVector(0.1f, 0.08f, 0.3f));
	}

	// Vertical scan line — thin bright bar that sweeps up through the hologram
	ScanLineMesh = NewObject<UStaticMeshComponent>(this);
	ScanLineMesh->SetupAttachment(RootComponent);
	ScanLineMesh->SetStaticMesh(CubeMesh);
	ScanLineMesh->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
	ScanLineMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.008f));
	ScanLineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScanLineMesh->CastShadow = false;
	ScanLineMesh->RegisterComponent();
	ScanMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	ScanMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.7f, 1.f));
	ScanMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.5f, 3.f, 8.f));
	ScanLineMesh->SetMaterial(0, ScanMat);

	// Scan line light that follows the bar
	ScanLight = NewObject<UPointLightComponent>(this);
	ScanLight->SetupAttachment(RootComponent);
	ScanLight->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ScanLight->SetIntensity(3000.f);
	ScanLight->SetAttenuationRadius(200.f);
	ScanLight->SetLightColor(FLinearColor(0.2f, 0.7f, 1.f));
	ScanLight->CastShadows = false;
	ScanLight->RegisterComponent();

	// Outer ring around base disk (hologram projector ring)
	if (CylMesh)
	{
		UStaticMeshComponent* Ring = NewObject<UStaticMeshComponent>(this);
		Ring->SetupAttachment(RootComponent);
		Ring->SetStaticMesh(CylMesh);
		Ring->SetRelativeLocation(FVector(0.f, 0.f, -48.f));
		Ring->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.01f));
		Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Ring->CastShadow = false;
		Ring->SetMaterial(0, BaseMat);
		Ring->RegisterComponent();
		DetailParts.Add(Ring);
	}
}

void AExoDecoyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;
	UpdateHologram(DeltaTime);
}

void AExoDecoyActor::UpdateHologram(float DeltaTime)
{
	float T = Age + FlickerSeed;

	// Gentle hover bob
	float BobZ = 8.f * FMath::Sin(T * 2.5f);
	float RotYaw = GetActorRotation().Yaw + 15.f * DeltaTime;
	SetActorLocation(SpawnLocation + FVector(0.f, 0.f, BobZ));
	SetActorRotation(FRotator(0.f, RotYaw, 0.f));

	// Multi-frequency holographic flicker (organic instability)
	float F1 = FMath::Sin(T * 47.f) * 0.15f;
	float F2 = FMath::Sin(T * 31.f) * 0.10f;
	float F3 = FMath::Sin(T * 89.f) * 0.06f;
	float Flicker = FMath::Clamp(1.f + F1 + F2 + F3, 0.25f, 1.5f);

	// Occasional hard glitch (brief near-invisible)
	float GlitchNoise = FMath::Sin(T * 47.f) * FMath::Sin(T * 31.f);
	if (GlitchNoise > 0.88f)
		Flicker *= 0.15f;

	// Slow pulse
	float Pulse = 1.f + 0.2f * FMath::Sin(T * 4.f);
	float EmScale = Flicker * Pulse;

	if (HoloMat)
	{
		FLinearColor EmCol(0.2f * EmScale, 1.5f * EmScale, 4.f * EmScale);
		HoloMat->SetVectorParameterValue(TEXT("EmissiveColor"), EmCol);
	}

	// Base disk pulse
	if (BaseMat)
	{
		float BasePulse = 0.8f + 0.4f * FMath::Sin(T * 6.f);
		FLinearColor BaseEm(0.1f * BasePulse, 0.8f * BasePulse, 2.f * BasePulse);
		BaseMat->SetVectorParameterValue(TEXT("EmissiveColor"), BaseEm);
	}

	// Light intensity with multi-frequency flicker
	if (HoloLight)
	{
		float LF = FMath::Sin(T * 35.f) * 0.12f + FMath::Sin(T * 73.f) * 0.08f;
		float LightPulse = (6000.f + 4000.f * Flicker) * (1.f + LF);
		HoloLight->SetIntensity(LightPulse);
	}

	// Scan line sweeps upward through the hologram
	if (ScanLineMesh)
	{
		float ScanCycle = FMath::Fmod(T * 1.8f, 1.f); // Cycles every ~0.55s
		float ScanZ = FMath::Lerp(-60.f, 140.f, ScanCycle);
		ScanLineMesh->SetRelativeLocation(FVector(0.f, 0.f, ScanZ));

		// Fade near edges of the sweep
		float ScanAlpha = FMath::Sin(ScanCycle * PI);
		if (ScanMat)
		{
			ScanMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.5f * ScanAlpha, 3.f * ScanAlpha, 8.f * ScanAlpha));
		}

		if (ScanLight)
		{
			ScanLight->SetRelativeLocation(FVector(0.f, 0.f, ScanZ));
			ScanLight->SetIntensity(3000.f * ScanAlpha);
		}
	}

	// Fade out near end of life (last 2 seconds)
	float LifeRemaining = GetLifeSpan();
	if (LifeRemaining > 0.f && LifeRemaining < 2.f)
	{
		float FadeAlpha = LifeRemaining / 2.f;
		BodyMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.9f) * FadeAlpha);
		HeadMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f) * FadeAlpha);
		if (HoloLight) HoloLight->SetIntensity(8000.f * FadeAlpha);

		// Fade detail parts too
		for (UStaticMeshComponent* P : DetailParts)
		{
			if (P)
			{
				FVector S = P->GetRelativeScale3D();
				P->SetRelativeScale3D(S * FMath::Max(FadeAlpha, 0.01f));
			}
		}
	}
}
