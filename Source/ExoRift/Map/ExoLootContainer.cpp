#include "Map/ExoLootContainer.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoArmorPickup.h"
#include "Weapons/ExoEnergyCellPickup.h"
#include "Player/ExoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Core/ExoTypes.h"
#include "ExoRift.h"

AExoLootContainer::AExoLootContainer()
{
	PrimaryActorTick.bCanEverTick = true;

	ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
	ContainerMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ContainerMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = ContainerMesh;
}

void AExoLootContainer::BeginPlay() { Super::BeginPlay(); }

void AExoLootContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (State != ELootContainerState::Opening) return;

	OpenTimer += DeltaTime;
	if (OpenTimer >= OpenDuration) FinishOpening();
}

void AExoLootContainer::Interact(AExoCharacter* Interactor)
{
	if (State != ELootContainerState::Closed || !Interactor) return;
	StartOpening(Interactor);
}

FString AExoLootContainer::GetInteractionPrompt()
{
	return (State == ELootContainerState::Closed) ? TEXT("Open Container") : FString();
}

void AExoLootContainer::StartOpening(AExoCharacter* Interactor)
{
	State = ELootContainerState::Opening;
	OpenTimer = 0.f;
	PendingInteractor = Interactor;
	UE_LOG(LogExoRift, Log, TEXT("LootContainer %s: opening"), *GetName());
}

void AExoLootContainer::FinishOpening()
{
	State = ELootContainerState::Open;
	SpawnLoot();
	State = ELootContainerState::Empty;
	PendingInteractor = nullptr;
	UE_LOG(LogExoRift, Log, TEXT("LootContainer %s: opened and emptied"), *GetName());
}

void AExoLootContainer::SpawnLoot()
{
	const int32 ItemCount = FMath::RandRange(MinItems, MaxItems);
	const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, 80.f);
	FActorSpawnParameters Params;

	for (int32 i = 0; i < ItemCount; ++i)
	{
		const float TypeRoll = FMath::FRand();

		if (TypeRoll < 0.5f)
		{
			auto* Pickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
				AExoWeaponPickup::StaticClass(), Origin, FRotator::ZeroRotator, Params);
			if (Pickup)
			{
				Pickup->Rarity = RollRarity();
				Pickup->WeaponType = static_cast<EWeaponType>(FMath::RandRange(0, 5));
			}
		}
		else if (TypeRoll < 0.7f)
		{
			GetWorld()->SpawnActor<AExoEnergyCellPickup>(
				AExoEnergyCellPickup::StaticClass(), Origin, FRotator::ZeroRotator, Params);
		}
		else if (TypeRoll < 0.85f)
		{
			GetWorld()->SpawnActor<AExoArmorPickup>(
				AExoArmorPickup::StaticClass(), Origin, FRotator::ZeroRotator, Params);
		}
		else
		{
			GetWorld()->SpawnActor<AExoEnergyCellPickup>(
				AExoEnergyCellPickup::StaticClass(), Origin, FRotator::ZeroRotator, Params);
		}
	}
}

FVector AExoLootContainer::ComputeEjectVelocity() const
{
	float Angle = FMath::FRandRange(0.f, 360.f);
	float Speed = FMath::FRandRange(200.f, 400.f);
	FVector Dir(FMath::Cos(FMath::DegreesToRadians(Angle)),
		FMath::Sin(FMath::DegreesToRadians(Angle)), 0.6f);
	return Dir.GetSafeNormal() * Speed;
}

EWeaponRarity AExoLootContainer::RollRarity() const
{
	const float Roll = FMath::FRand();
	if (Roll < 0.60f) return EWeaponRarity::Common;
	if (Roll < 0.85f) return EWeaponRarity::Rare;
	if (Roll < 0.95f) return EWeaponRarity::Epic;
	return EWeaponRarity::Legendary;
}
