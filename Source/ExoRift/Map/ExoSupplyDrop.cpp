#include "Map/ExoSupplyDrop.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoEnergyCellPickup.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "ExoRift.h"

AExoSupplyDrop::AExoSupplyDrop()
{
	PrimaryActorTick.bCanEverTick = true;

	CrateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
	RootComponent = CrateMesh;
	CrateMesh->SetCollisionProfileName(TEXT("BlockAll"));

	BeaconLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeaconLight"));
	BeaconLight->SetupAttachment(RootComponent);
	BeaconLight->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	BeaconLight->SetIntensity(0.f); // Off until landed
	BeaconLight->SetAttenuationRadius(BeaconRange);
	BeaconLight->SetLightColor(FColor(255, 200, 50)); // Yellow beacon
}

void AExoSupplyDrop::BeginPlay()
{
	Super::BeginPlay();
	TransitionToState(ESupplyDropState::Falling);
}

void AExoSupplyDrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ESupplyDropState::Falling)
	{
		TickFalling(DeltaTime);
	}
}

void AExoSupplyDrop::TickFalling(float DeltaTime)
{
	FVector Location = GetActorLocation();
	FVector NewLocation = Location - FVector(0.f, 0.f, DropSpeed * DeltaTime);

	// Trace downward to detect ground
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHitGround = GetWorld()->LineTraceSingleByChannel(
		Hit, Location, NewLocation - FVector(0.f, 0.f, 50.f),
		ECC_WorldStatic, Params);

	if (bHitGround)
	{
		// Snap to ground
		SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, 5.f));
		TransitionToState(ESupplyDropState::Landed);
	}
	else
	{
		SetActorLocation(NewLocation);
	}
}

void AExoSupplyDrop::TransitionToState(ESupplyDropState NewState)
{
	CurrentState = NewState;

	switch (NewState)
	{
	case ESupplyDropState::Falling:
		BeaconLight->SetIntensity(0.f);
		break;

	case ESupplyDropState::Landed:
		BeaconLight->SetIntensity(BeaconIntensity);
		BeaconLight->SetAttenuationRadius(BeaconRange);
		UE_LOG(LogExoRift, Log, TEXT("Supply drop landed at %s"), *GetActorLocation().ToString());
		break;

	case ESupplyDropState::Opened:
		SpawnLoot();
		BeaconLight->SetIntensity(BeaconIntensity * 0.1f);
		break;

	case ESupplyDropState::Depleted:
		BeaconLight->SetIntensity(0.f);
		break;
	}
}

void AExoSupplyDrop::Interact(AExoCharacter* Interactor)
{
	if (CurrentState != ESupplyDropState::Landed || !Interactor) return;
	TransitionToState(ESupplyDropState::Opened);

	// Grant shield directly to the interactor
	UExoShieldComponent* Shield = Interactor->GetShieldComponent();
	if (Shield)
	{
		Shield->AddShield(25.f);
	}

	TransitionToState(ESupplyDropState::Depleted);
}

FString AExoSupplyDrop::GetInteractionPrompt()
{
	if (CurrentState == ESupplyDropState::Landed)
	{
		return TEXT("Press E to open supply drop");
	}
	return FString();
}

void AExoSupplyDrop::SpawnLoot()
{
	FVector Origin = GetActorLocation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn 2 weapons at Epic or Legendary rarity
	const EWeaponType WeaponTypes[] = { EWeaponType::Rifle, EWeaponType::Pistol, EWeaponType::GrenadeLauncher };
	const EWeaponRarity HighRarities[] = { EWeaponRarity::Epic, EWeaponRarity::Legendary };

	for (int32 i = 0; i < 2; ++i)
	{
		float Angle = i * PI; // Opposite sides
		FVector Offset(FMath::Cos(Angle) * 120.f, FMath::Sin(Angle) * 120.f, 30.f);
		FVector SpawnLoc = Origin + Offset;

		AExoWeaponPickup* WeaponPickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
			AExoWeaponPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (WeaponPickup)
		{
			WeaponPickup->WeaponType = WeaponTypes[FMath::RandRange(0, 2)];
			WeaponPickup->Rarity = HighRarities[FMath::RandRange(0, 1)];
		}
	}

	// Spawn 2 energy cells
	for (int32 i = 0; i < 2; ++i)
	{
		float Angle = (i * PI) + PI * 0.5f; // 90/270 degrees from weapons
		FVector Offset(FMath::Cos(Angle) * 100.f, FMath::Sin(Angle) * 100.f, 20.f);
		FVector SpawnLoc = Origin + Offset;

		AExoEnergyCellPickup* CellPickup = GetWorld()->SpawnActor<AExoEnergyCellPickup>(
			AExoEnergyCellPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (CellPickup)
		{
			CellPickup->EnergyAmount = 100.f; // High-tier cells
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Supply drop spawned loot at %s"), *Origin.ToString());
}
