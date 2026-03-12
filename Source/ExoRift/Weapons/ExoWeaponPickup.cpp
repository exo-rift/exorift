// ExoWeaponPickup.cpp — Tick, interaction, weapon spawning
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Weapons/ExoWeaponGrenadeLauncher.h"
#include "Weapons/ExoWeaponSniper.h"
#include "Weapons/ExoWeaponShotgun.h"
#include "Weapons/ExoWeaponSMG.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Visual/ExoPickupFlash.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ExoRift.h"

AExoWeaponPickup::AExoWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;

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

	BobPhase = FMath::RandRange(0.f, 2.f * PI);
}

void AExoWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
	BuildPickupModel();
}

void AExoWeaponPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		BobPhase += DeltaTime * 2.f;
		float Bob = FMath::Sin(BobPhase) * 20.f;
		SetActorLocation(BaseLocation + FVector(0.f, 0.f, Bob));

		FRotator Rot = GetActorRotation();
		Rot.Yaw += DeltaTime * 45.f;
		SetActorRotation(Rot);

		// Pulse glow for epic/legendary
		if (RarityGlow && (Rarity == EWeaponRarity::Epic || Rarity == EWeaponRarity::Legendary))
		{
			float Pulse = 0.7f + 0.3f * FMath::Sin(BobPhase * 1.5f);
			float Base = (Rarity == EWeaponRarity::Legendary) ? 16000.f : 10000.f;
			RarityGlow->SetIntensity(Base * Pulse);
		}

		// Pulse emissive accent strip — all rarities pulse now
		if (AccentMat)
		{
			FLinearColor RC = AExoWeaponBase::GetRarityColor(Rarity);
			float BaseEm;
			switch (Rarity)
			{
			case EWeaponRarity::Common:    BaseEm = 1.1f; break;
			case EWeaponRarity::Rare:      BaseEm = 2.8f; break;
			case EWeaponRarity::Epic:      BaseEm = 4.5f; break;
			case EWeaponRarity::Legendary: BaseEm = 9.0f; break;
			default:                       BaseEm = 1.1f; break;
			}
			float EmPulse = BaseEm * (0.7f + 0.3f * FMath::Sin(BobPhase * 1.5f));
			AccentMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(RC.R * EmPulse, RC.G * EmPulse, RC.B * EmPulse));
		}

		// Animate pedestal glow pulse
		if (PedestalMat)
		{
			FLinearColor RC = AExoWeaponBase::GetRarityColor(Rarity);
			float PedPulse = 0.45f + 0.35f * FMath::Sin(BobPhase * 2.f);
			PedestalMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(RC.R * PedPulse, RC.G * PedPulse, RC.B * PedPulse));
		}

		// Spin rarity ring independently
		if (RarityRing)
		{
			FRotator RingRot = RarityRing->GetRelativeRotation();
			RingRot.Yaw += DeltaTime * 90.f;
			RarityRing->SetRelativeRotation(RingRot);
			// Breathe ring scale
			float RingBreath = 0.65f + 0.05f * FMath::Sin(BobPhase * 1.2f);
			RarityRing->SetRelativeScale3D(FVector(RingBreath, RingBreath, 0.005f));
		}

		// Beacon beam shimmer
		if (BeaconBeam && BeaconMat)
		{
			FLinearColor RC = AExoWeaponBase::GetRarityColor(Rarity);
			float BaseEm;
			switch (Rarity)
			{
			case EWeaponRarity::Rare:      BaseEm = 4.0f; break;
			case EWeaponRarity::Epic:      BaseEm = 7.0f; break;
			case EWeaponRarity::Legendary: BaseEm = 14.0f; break;
			default:                       BaseEm = 2.2f; break;
			}
			float Shimmer = BaseEm * (0.6f + 0.4f * FMath::Sin(BobPhase * 2.5f));
			BeaconMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(RC.R * Shimmer, RC.G * Shimmer, RC.B * Shimmer));
		}

		// Proximity audio hum for nearby players
		ProximityHumTimer -= DeltaTime;
		if (ProximityHumTimer <= 0.f)
		{
			ProximityHumTimer = 2.5f;
			// Find nearest player pawn
			APawn* NearestPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
			if (NearestPawn)
			{
				float Dist = FVector::Dist(GetActorLocation(), NearestPawn->GetActorLocation());
				if (Dist < 800.f)
				{
					float Volume = FMath::Lerp(0.15f, 0.02f, Dist / 800.f);
					float Pitch = 1.5f + (int32)Rarity * 0.3f; // Higher for rarer weapons
					if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
						Audio->PlayWeaponFireSound(nullptr, GetActorLocation(), Volume);
				}
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

	// Collection flash VFX
	AExoPickupFlash::SpawnAt(GetWorld(), GetActorLocation(),
		AExoWeaponBase::GetRarityColor(Rarity));

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
	return FString::Printf(TEXT("[E] %s"), *GetWeaponDisplayName());
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

FString AExoWeaponPickup::GetWeaponDisplayName() const
{
	FString RarityPrefix;
	switch (Rarity)
	{
	case EWeaponRarity::Common:    RarityPrefix = TEXT(""); break;
	case EWeaponRarity::Rare:      RarityPrefix = TEXT("Rare "); break;
	case EWeaponRarity::Epic:      RarityPrefix = TEXT("Epic "); break;
	case EWeaponRarity::Legendary: RarityPrefix = TEXT("Legendary "); break;
	default: break;
	}

	FString TypeName;
	switch (WeaponType)
	{
	case EWeaponType::Rifle:           TypeName = TEXT("Rifle"); break;
	case EWeaponType::Pistol:          TypeName = TEXT("Pistol"); break;
	case EWeaponType::GrenadeLauncher: TypeName = TEXT("Grenade Launcher"); break;
	case EWeaponType::Sniper:          TypeName = TEXT("Sniper"); break;
	case EWeaponType::Shotgun:         TypeName = TEXT("Shotgun"); break;
	case EWeaponType::SMG:             TypeName = TEXT("SMG"); break;
	case EWeaponType::Melee:           TypeName = TEXT("Plasma Blade"); break;
	default:                           TypeName = TEXT("Weapon"); break;
	}

	return RarityPrefix + TypeName;
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
