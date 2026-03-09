#include "Weapons/ExoEnergyCellPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(CollisionSphere);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetRelativeScale3D(FVector(0.4f));

	BobPhase = FMath::RandRange(0.f, 2.f * PI);
}

void AExoEnergyCellPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
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

		// Glowing yellow-green tint
		if (DisplayMesh && DisplayMesh->GetMaterial(0))
		{
			UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(DisplayMesh->GetMaterial(0));
			if (!DynMat) DynMat = DisplayMesh->CreateAndSetMaterialInstanceDynamic(0);
			if (DynMat)
			{
				float Pulse = 0.8f + 0.2f * FMath::Sin(BobPhase * 1.5f);
				FLinearColor GlowColor(Pulse * 0.8f, Pulse * 1.f, Pulse * 0.2f, 1.f);
				DynMat->SetVectorParameterValue(TEXT("BaseColor"), GlowColor);
				DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 3.f);
			}
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
	UE_LOG(LogExoRift, Log, TEXT("%s picked up energy cell (+%.0f)"), *Interactor->GetName(), EnergyAmount);

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
	return FString::Printf(TEXT("Press E to pick up Energy Cell (+%.0f)"), EnergyAmount);
}

void AExoEnergyCellPickup::SetPickupActive(bool bActive)
{
	bIsActive = bActive;
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
}
