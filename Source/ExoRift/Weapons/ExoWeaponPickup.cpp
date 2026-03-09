#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Weapons/ExoWeaponGrenadeLauncher.h"
#include "Weapons/ExoWeaponSniper.h"
#include "Weapons/ExoWeaponShotgun.h"
#include "Weapons/ExoWeaponSMG.h"
#include "Player/ExoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ExoRift.h"

AExoWeaponPickup::AExoWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision sphere kept for spatial queries (interaction trace hits it)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(150.f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = CollisionSphere;

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(CollisionSphere);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetRelativeScale3D(FVector(0.5f));

	// No overlap delegate — pickup is now interaction-based

	// Random bob phase so pickups don't all bob in sync
	BobPhase = FMath::RandRange(0.f, 2.f * PI);
}

void AExoWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
}

void AExoWeaponPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		// Floating bob + rotation
		BobPhase += DeltaTime * 2.f;
		float Bob = FMath::Sin(BobPhase) * 20.f;
		SetActorLocation(BaseLocation + FVector(0.f, 0.f, Bob));

		FRotator Rot = GetActorRotation();
		Rot.Yaw += DeltaTime * 45.f;
		SetActorRotation(Rot);

		// Tint display mesh with rarity color
		if (DisplayMesh && DisplayMesh->GetMaterial(0))
		{
			UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(DisplayMesh->GetMaterial(0));
			if (!DynMat)
			{
				DynMat = DisplayMesh->CreateAndSetMaterialInstanceDynamic(0);
			}
			if (DynMat)
			{
				FLinearColor RarityColor = AExoWeaponBase::GetRarityColor(Rarity);
				DynMat->SetVectorParameterValue(TEXT("BaseColor"), RarityColor);
			}
		}
	}
	else if (bRespawns)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f)
		{
			SetPickupActive(true);
		}
	}
}

// ---------------------------------------------------------------------------
// IExoInteractable implementation
// ---------------------------------------------------------------------------

void AExoWeaponPickup::Interact(AExoCharacter* Interactor)
{
	if (!bIsActive || !Interactor || !Interactor->IsAlive()) return;

	SpawnWeaponForPlayer(Interactor);
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

FString AExoWeaponPickup::GetInteractionPrompt()
{
	return FString::Printf(TEXT("Press E to pick up %s"), *GetWeaponDisplayName());
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

FString AExoWeaponPickup::GetWeaponDisplayName() const
{
	switch (WeaponType)
	{
	case EWeaponType::Rifle:           return TEXT("Rifle");
	case EWeaponType::Pistol:          return TEXT("Pistol");
	case EWeaponType::GrenadeLauncher: return TEXT("Grenade Launcher");
	case EWeaponType::Sniper:          return TEXT("Sniper");
	case EWeaponType::Shotgun:         return TEXT("Shotgun");
	case EWeaponType::SMG:             return TEXT("SMG");
	case EWeaponType::Melee:           return TEXT("Plasma Blade");
	default:                           return TEXT("Weapon");
	}
}

void AExoWeaponPickup::SpawnWeaponForPlayer(AExoCharacter* Character)
{
	if (!Character) return;

	TSubclassOf<AExoWeaponBase> WeaponClass;
	switch (WeaponType)
	{
	case EWeaponType::Rifle:           WeaponClass = AExoWeaponRifle::StaticClass(); break;
	case EWeaponType::Pistol:          WeaponClass = AExoWeaponPistol::StaticClass(); break;
	case EWeaponType::GrenadeLauncher: WeaponClass = AExoWeaponGrenadeLauncher::StaticClass(); break;
	case EWeaponType::Sniper:          WeaponClass = AExoWeaponSniper::StaticClass(); break;
	case EWeaponType::Shotgun:         WeaponClass = AExoWeaponShotgun::StaticClass(); break;
	case EWeaponType::SMG:             WeaponClass = AExoWeaponSMG::StaticClass(); break;
	default: break;
	}

	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;

		AExoWeaponBase* Weapon = GetWorld()->SpawnActor<AExoWeaponBase>(
			WeaponClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Weapon)
		{
			Weapon->Rarity = Rarity;
			Character->EquipWeapon(Weapon);
			UE_LOG(LogExoRift, Log, TEXT("%s picked up %s (Rarity: %d)"),
				*Character->GetName(), *Weapon->GetWeaponName(), static_cast<uint8>(Rarity));
		}
	}
}

void AExoWeaponPickup::SetPickupActive(bool bActive)
{
	bIsActive = bActive;
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
}
