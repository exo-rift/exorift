#include "Player/ExoDecoyActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
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

	UE_LOG(LogExoRift, Log, TEXT("Holographic decoy deployed at %s"),
		*GetActorLocation().ToString());
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

	// Holographic flicker — rapid small opacity changes
	float Flicker = 1.f;
	float FastNoise = FMath::Sin(T * 47.f) * FMath::Sin(T * 31.f);
	if (FastNoise > 0.85f)
		Flicker = 0.3f; // Brief dim
	else if (FastNoise > 0.7f)
		Flicker = 0.6f; // Slight dim

	// Scan line effect — periodic brightness stripe
	float ScanLine = FMath::Fmod(T * 3.f, 1.f);
	float ScanBoost = (ScanLine > 0.9f) ? 1.5f : 1.f;

	// Pulse emissive
	float Pulse = 1.f + 0.3f * FMath::Sin(T * 4.f);
	float EmScale = Flicker * ScanBoost * Pulse;

	if (HoloMat)
	{
		FLinearColor EmCol(0.2f * EmScale, 1.5f * EmScale, 4.f * EmScale);
		HoloMat->SetVectorParameterValue(TEXT("EmissiveColor"), EmCol);
	}

	// Base disk rotation ring pulse
	if (BaseMat)
	{
		float BasePulse = 0.8f + 0.4f * FMath::Sin(T * 6.f);
		FLinearColor BaseEm(0.1f * BasePulse, 0.8f * BasePulse, 2.f * BasePulse);
		BaseMat->SetVectorParameterValue(TEXT("EmissiveColor"), BaseEm);
	}

	// Light intensity flicker
	if (HoloLight)
	{
		float LightPulse = 6000.f + 4000.f * Flicker * FMath::Sin(T * 5.f);
		HoloLight->SetIntensity(LightPulse);
	}

	// Fade out near end of life (last 2 seconds)
	float LifeRemaining = GetLifeSpan();
	if (LifeRemaining > 0.f && LifeRemaining < 2.f)
	{
		float FadeAlpha = LifeRemaining / 2.f;
		FVector FadeScale = FVector(1.f) * FadeAlpha;
		BodyMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.9f) * FadeAlpha);
		HeadMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f) * FadeAlpha);
		if (HoloLight) HoloLight->SetIntensity(8000.f * FadeAlpha);
	}
}
