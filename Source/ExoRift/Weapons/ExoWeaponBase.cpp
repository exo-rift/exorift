// ExoWeaponBase.cpp — Constructor, Tick, heat/energy, sway, rarity helpers
// FireShot & DoLineTrace are in ExoWeaponCombat.cpp
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Visual/ExoWeaponViewModel.h"
#include "Core/ExoAudioManager.h"
#include "Core/ExoPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "ExoRift.h"

AExoWeaponBase::AExoWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void AExoWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	ViewModel = NewObject<UExoWeaponViewModel>(this);
	ViewModel->SetupAttachment(RootComponent);
	ViewModel->SetRelativeLocation(FVector(20.f, 10.f, -8.f));
	ViewModel->RegisterComponent();
	ViewModel->BuildModel(WeaponType, GetRarityColor(Rarity));
}

float AExoWeaponBase::GetRarityDamageMultiplier() const
{
	switch (Rarity)
	{
	case EWeaponRarity::Common:    return 1.0f;
	case EWeaponRarity::Rare:      return 1.15f;
	case EWeaponRarity::Epic:      return 1.3f;
	case EWeaponRarity::Legendary: return 1.5f;
	default:                       return 1.0f;
	}
}

float AExoWeaponBase::GetRarityHeatMultiplier() const
{
	switch (Rarity)
	{
	case EWeaponRarity::Common:    return 1.0f;
	case EWeaponRarity::Rare:      return 0.95f;
	case EWeaponRarity::Epic:      return 0.85f;
	case EWeaponRarity::Legendary: return 0.75f;
	default:                       return 1.0f;
	}
}

FLinearColor AExoWeaponBase::GetRarityColor(EWeaponRarity InRarity)
{
	switch (InRarity)
	{
	case EWeaponRarity::Common:    return FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
	case EWeaponRarity::Rare:      return FLinearColor(0.2f, 0.5f, 1.0f, 1.f);
	case EWeaponRarity::Epic:      return FLinearColor(0.7f, 0.2f, 1.0f, 1.f);
	case EWeaponRarity::Legendary: return FLinearColor(1.0f, 0.85f, 0.2f, 1.f);
	default:                       return FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
	}
}

void AExoWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastShot += DeltaTime;
	TickCooldown(DeltaTime);

	// Spread recovery
	if (!bWantsToFire || bIsOverheated)
	{
		CurrentSpread = FMath::Max(BaseSpread, CurrentSpread - SpreadRecoveryRate * DeltaTime);
	}

	TickWeaponSway(DeltaTime);

	// Auto-fire
	if (bWantsToFire && !bIsOverheated && CurrentEnergy >= EnergyPerShot)
	{
		float FireInterval = 1.f / FireRate;
		if (TimeSinceLastShot >= FireInterval)
		{
			FireShot();
			TimeSinceLastShot = 0.f;

			if (!bIsAutomatic)
			{
				bWantsToFire = false;
			}
		}
	}
}

void AExoWeaponBase::StartFire()
{
	bWantsToFire = true;
}

void AExoWeaponBase::StopFire()
{
	bWantsToFire = false;
}

void AExoWeaponBase::AddHeat(float Amount)
{
	Amount *= GetRarityHeatMultiplier();
	float OldHeat = CurrentHeat;
	CurrentHeat = FMath::Clamp(CurrentHeat + Amount, 0.f, 1.f);

	if (CurrentHeat >= 1.f && !bIsOverheated)
	{
		bIsOverheated = true;
		bWantsToFire = false;
		OnOverheated.Broadcast();
		UE_LOG(LogExoRift, Verbose, TEXT("%s overheated!"), *WeaponName);
	}

	if (CurrentHeat != OldHeat)
	{
		OnHeatChanged.Broadcast(CurrentHeat);
	}
}

void AExoWeaponBase::TickCooldown(float DeltaTime)
{
	if (CurrentHeat <= 0.f) return;
	if (bWantsToFire && !bIsOverheated) return;

	float Rate = bIsOverheated ? OverheatCooldownRate : CooldownRate;
	float OldHeat = CurrentHeat;
	CurrentHeat = FMath::Max(CurrentHeat - Rate * DeltaTime, 0.f);

	if (bIsOverheated && CurrentHeat <= OverheatRecoveryThreshold)
	{
		bIsOverheated = false;
		OnCooledDown.Broadcast();
	}

	if (CurrentHeat != OldHeat)
	{
		OnHeatChanged.Broadcast(CurrentHeat);
	}
}

void AExoWeaponBase::AddEnergy(float Amount)
{
	float OldEnergy = CurrentEnergy;
	CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.f, MaxEnergy);
	if (CurrentEnergy != OldEnergy) OnEnergyChanged.Broadcast(CurrentEnergy);
}

void AExoWeaponBase::TickWeaponSway(float DeltaTime)
{
	if (!ViewModel) return;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	AController* PC = OwnerPawn->GetController();
	if (!PC) return;

	FRotator CurrentRot = PC->GetControlRotation();
	FRotator DeltaRot = CurrentRot - PrevControlRotation;
	DeltaRot.Normalize();
	PrevControlRotation = CurrentRot;

	SwayOffset.Y += DeltaRot.Yaw * SwayAmount;
	SwayOffset.Z += DeltaRot.Pitch * SwayAmount;

	SwayOffset.Y = FMath::Clamp(SwayOffset.Y, -MaxSwayOffset, MaxSwayOffset);
	SwayOffset.Z = FMath::Clamp(SwayOffset.Z, -MaxSwayOffset, MaxSwayOffset);

	SwayOffset = FMath::VInterpTo(SwayOffset, FVector::ZeroVector, DeltaTime, SwayReturnSpeed);

	float Time = GetWorld()->GetTimeSeconds();
	float IdleY = FMath::Sin(Time * 1.2f) * 0.15f;
	float IdleZ = FMath::Sin(Time * 0.8f + 0.7f) * 0.1f;

	FVector BasePos(20.f, 10.f, -8.f);
	ViewModel->SetRelativeLocation(BasePos + SwayOffset + FVector(0.f, IdleY, IdleZ));
}
