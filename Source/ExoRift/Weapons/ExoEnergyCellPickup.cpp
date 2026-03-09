#include "Weapons/ExoEnergyCellPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "ExoRift.h"

AExoEnergyCellPickup::AExoEnergyCellPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(120.f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = CollisionSphere;

	// Battery body — cylinder core
	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(CollisionSphere);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.4f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFinder.Succeeded())
	{
		CachedCylinder = CylFinder.Object;
		DisplayMesh->SetStaticMesh(CachedCylinder);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphFinder.Succeeded())
	{
		CachedSphere = SphFinder.Object;
	}

	BobPhase = FMath::RandRange(0.f, 2.f * PI);
}

void AExoEnergyCellPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
	BuildCellModel();
}

void AExoEnergyCellPickup::BuildCellModel()
{
	// Teal energy material on the main body
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	UMaterialInterface* BaseMat = MatFinder.Succeeded() ? MatFinder.Object : nullptr;

	if (BaseMat && DisplayMesh)
	{
		UMaterialInstanceDynamic* BodyMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.05f, 0.15f, 0.12f));
		DisplayMesh->SetMaterial(0, BodyMat);
	}

	// Top cap — flattened sphere
	if (CachedSphere && BaseMat)
	{
		UStaticMeshComponent* TopCap = NewObject<UStaticMeshComponent>(this);
		TopCap->SetupAttachment(DisplayMesh);
		TopCap->SetStaticMesh(CachedSphere);
		TopCap->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
		TopCap->SetRelativeScale3D(FVector(1.1f, 1.1f, 0.3f));
		TopCap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UMaterialInstanceDynamic* CapMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		CapMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.08f, 0.25f, 0.2f));
		TopCap->SetMaterial(0, CapMat);
		TopCap->RegisterComponent();

		// Bottom cap
		UStaticMeshComponent* BotCap = NewObject<UStaticMeshComponent>(this);
		BotCap->SetupAttachment(DisplayMesh);
		BotCap->SetStaticMesh(CachedSphere);
		BotCap->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
		BotCap->SetRelativeScale3D(FVector(1.1f, 1.1f, 0.3f));
		BotCap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BotCap->SetMaterial(0, CapMat);
		BotCap->RegisterComponent();
	}

	// Energy ring — thin cylinder around the middle
	if (CachedCylinder && BaseMat)
	{
		UStaticMeshComponent* Ring = NewObject<UStaticMeshComponent>(this);
		Ring->SetupAttachment(DisplayMesh);
		Ring->SetStaticMesh(CachedCylinder);
		Ring->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		Ring->SetRelativeScale3D(FVector(1.3f, 1.3f, 0.08f));
		Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		RingMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.9f, 0.6f));
		Ring->SetMaterial(0, RingMat);
		Ring->RegisterComponent();
	}

	// Glow light — teal energy pulse
	GlowLight = NewObject<UPointLightComponent>(this);
	GlowLight->SetupAttachment(DisplayMesh);
	GlowLight->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	GlowLight->SetIntensity(3000.f);
	GlowLight->SetAttenuationRadius(500.f);
	GlowLight->SetLightColor(FLinearColor(0.1f, 0.9f, 0.5f));
	GlowLight->CastShadows = false;
	GlowLight->RegisterComponent();
}

void AExoEnergyCellPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		// Floating bob + rotation
		BobPhase += DeltaTime * 2.5f;
		float Bob = FMath::Sin(BobPhase) * 15.f;
		SetActorLocation(BaseLocation + FVector(0.f, 0.f, Bob));

		FRotator Rot = GetActorRotation();
		Rot.Yaw += DeltaTime * 60.f;
		SetActorRotation(Rot);

		// Pulsing glow light
		if (GlowLight)
		{
			float Pulse = 0.7f + 0.3f * FMath::Sin(BobPhase * 1.5f);
			GlowLight->SetIntensity(2000.f + 2000.f * Pulse);
		}
	}
	else if (bRespawns)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f) SetPickupActive(true);
	}
}

void AExoEnergyCellPickup::Interact(AExoCharacter* Interactor)
{
	if (!bIsActive || !Interactor || !Interactor->IsAlive()) return;

	AExoWeaponBase* Weapon = Interactor->GetCurrentWeapon();
	if (!Weapon) return;

	Weapon->AddEnergy(EnergyAmount);
	UE_LOG(LogExoRift, Log, TEXT("%s picked up energy cell (+%.0f)"),
		*Interactor->GetName(), EnergyAmount);

	// Pickup audio feedback
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayImpactSound(GetActorLocation(), false);
	}

	SetPickupActive(false);
	if (bRespawns)
	{
		RespawnTimer = RespawnTime;
	}
	else
	{
		SetLifeSpan(0.1f);
	}
}

FString AExoEnergyCellPickup::GetInteractionPrompt()
{
	return FString::Printf(TEXT("[E] Energy Cell (+%.0f)"), EnergyAmount);
}

void AExoEnergyCellPickup::SetPickupActive(bool bActive)
{
	bIsActive = bActive;
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
	if (GlowLight) GlowLight->SetVisibility(bActive);
}
