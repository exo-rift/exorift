#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Weapons/ExoWeaponGrenadeLauncher.h"
#include "Weapons/ExoWeaponSniper.h"
#include "Weapons/ExoWeaponShotgun.h"
#include "Weapons/ExoWeaponSMG.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
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

void AExoWeaponPickup::BuildPickupModel()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	UStaticMesh* CubeMesh = CubeFind.Succeeded() ? CubeFind.Object : nullptr;
	UStaticMesh* CylMesh = CylFind.Succeeded() ? CylFind.Object : nullptr;
	UMaterialInterface* BaseMat = MatFind.Succeeded() ? MatFind.Object : nullptr;

	if (!CubeMesh || !BaseMat) return;

	FLinearColor RarityColor = AExoWeaponBase::GetRarityColor(Rarity);

	auto MakePart = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->RegisterComponent();
		if (BaseMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			C->SetMaterial(0, Mat);
		}
	};

	FLinearColor BodyColor(0.1f, 0.1f, 0.12f);

	// Build a simplified weapon silhouette per type
	switch (WeaponType)
	{
	case EWeaponType::Rifle:
	case EWeaponType::SMG:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.6f, 0.12f, 0.1f), BodyColor);
		MakePart(CubeMesh, FVector(25.f, 0.f, 0.f), FVector(0.35f, 0.06f, 0.06f), BodyColor); // Barrel
		MakePart(CubeMesh, FVector(-10.f, 0.f, -8.f), FVector(0.08f, 0.06f, 0.12f), BodyColor); // Mag
		MakePart(CubeMesh, FVector(0.f, 0.f, 2.f), FVector(0.4f, 0.02f, 0.02f), RarityColor); // Accent
		break;

	case EWeaponType::Pistol:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.3f, 0.1f, 0.1f), BodyColor);
		MakePart(CubeMesh, FVector(15.f, 0.f, 0.f), FVector(0.2f, 0.05f, 0.05f), BodyColor);
		MakePart(CubeMesh, FVector(-5.f, 0.f, -8.f), FVector(0.06f, 0.06f, 0.1f), BodyColor); // Grip
		MakePart(CubeMesh, FVector(0.f, 0.f, 3.f), FVector(0.25f, 0.02f, 0.02f), RarityColor);
		break;

	case EWeaponType::Shotgun:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.5f, 0.14f, 0.12f), BodyColor);
		MakePart(CubeMesh, FVector(20.f, 0.f, 0.f), FVector(0.25f, 0.08f, 0.08f), BodyColor);
		MakePart(CubeMesh, FVector(20.f, 0.f, 3.f), FVector(0.25f, 0.08f, 0.08f), BodyColor); // Double barrel
		MakePart(CubeMesh, FVector(0.f, 0.f, 3.f), FVector(0.35f, 0.02f, 0.02f), RarityColor);
		break;

	case EWeaponType::Sniper:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.7f, 0.1f, 0.08f), BodyColor);
		MakePart(CubeMesh, FVector(30.f, 0.f, 0.f), FVector(0.4f, 0.04f, 0.04f), BodyColor); // Long barrel
		MakePart(CubeMesh, FVector(10.f, 0.f, 6.f), FVector(0.1f, 0.05f, 0.05f), BodyColor); // Scope
		MakePart(CubeMesh, FVector(0.f, 0.f, 2.f), FVector(0.5f, 0.02f, 0.02f), RarityColor);
		break;

	case EWeaponType::GrenadeLauncher:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.45f, 0.14f, 0.14f), BodyColor);
		if (CylMesh) MakePart(CylMesh, FVector(18.f, 0.f, 0.f), FVector(0.12f, 0.12f, 0.15f), BodyColor,
			FRotator(0.f, 0.f, 90.f)); // Drum
		MakePart(CubeMesh, FVector(0.f, 0.f, 3.f), FVector(0.3f, 0.02f, 0.02f), RarityColor);
		break;

	default:
		MakePart(CubeMesh, FVector(0.f, 0.f, 0.f), FVector(0.4f, 0.1f, 0.1f), BodyColor);
		MakePart(CubeMesh, FVector(0.f, 0.f, 2.f), FVector(0.3f, 0.02f, 0.02f), RarityColor);
		break;
	}

	// Rarity glow light
	UPointLightComponent* Glow = NewObject<UPointLightComponent>(this);
	Glow->SetupAttachment(RootComponent);
	Glow->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	Glow->SetLightColor(RarityColor);
	Glow->CastShadows = false;
	Glow->RegisterComponent();

	// Intensity based on rarity
	float GlowIntensity;
	switch (Rarity)
	{
	case EWeaponRarity::Common:    GlowIntensity = 1000.f; break;
	case EWeaponRarity::Rare:      GlowIntensity = 2000.f; break;
	case EWeaponRarity::Epic:      GlowIntensity = 4000.f; break;
	case EWeaponRarity::Legendary: GlowIntensity = 6000.f; break;
	default:                       GlowIntensity = 1000.f; break;
	}
	Glow->SetIntensity(GlowIntensity);
	Glow->SetAttenuationRadius(300.f);
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
