// ExoWeaponBase.cpp — Constructor, BeginPlay, Tick, fire/stop, heat, cooldown, energy, rarity
// Animation & visual logic split to ExoWeaponBaseAnim.cpp
// FireShot & DoLineTrace are in ExoWeaponCombat.cpp
#include "Weapons/ExoWeaponBase.h"
#include "Visual/ExoWeaponViewModel.h"
#include "Visual/ExoWeaponAura.h"
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

	// Rarity aura for Epic/Legendary weapons
	if (Rarity == EWeaponRarity::Epic || Rarity == EWeaponRarity::Legendary)
	{
		UExoWeaponAura* Aura = NewObject<UExoWeaponAura>(this);
		Aura->SetupAttachment(ViewModel);
		Aura->RegisterComponent();
		Aura->InitAura(Rarity, GetRarityColor(Rarity));
	}
}

void AExoWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastShot += DeltaTime;
	TickCooldown(DeltaTime);

	// Draw/equip animation
	if (bDrawAnimActive)
	{
		DrawBlend = FMath::FInterpTo(DrawBlend, 1.f, DeltaTime, DrawSpeed);
		if (DrawBlend > 0.98f) { DrawBlend = 1.f; bDrawAnimActive = false; }
	}

	// Spread recovery (faster when ADS)
	if (!bWantsToFire || bIsOverheated)
	{
		float RecovRate = bIsADS ? SpreadRecoveryRate * 2.f : SpreadRecoveryRate;
		float MinSpread = bIsADS ? BaseSpread * ADSSpreadMultiplier : BaseSpread;
		CurrentSpread = FMath::Max(MinSpread, CurrentSpread - RecovRate * DeltaTime);
	}

	TickWeaponSway(DeltaTime);
	TickHeatGlow();

	// Inspect animation
	if (bIsInspecting)
	{
		InspectBlend = FMath::FInterpTo(InspectBlend, 1.f, DeltaTime, 4.f);
		InspectPhase += DeltaTime;
	}
	else
	{
		InspectBlend = FMath::FInterpTo(InspectBlend, 0.f, DeltaTime, 6.f);
		if (InspectBlend < 0.01f) InspectPhase = 0.f;
	}

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
	if (bIsInspecting) StopInspect();
}

void AExoWeaponBase::StopFire()
{
	bWantsToFire = false;
}

void AExoWeaponBase::StartADS()
{
	bIsADS = true;
	if (bIsInspecting) StopInspect();
}

void AExoWeaponBase::StopADS()
{
	bIsADS = false;
}

void AExoWeaponBase::ToggleFireMode()
{
	// Base weapons don't have fire mode toggle — override in subclass
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

void AExoWeaponBase::ResetHeat()
{
	CurrentHeat = 0.f;
	if (bIsOverheated)
	{
		bIsOverheated = false;
		OnCooledDown.Broadcast();
	}
	OnHeatChanged.Broadcast(0.f);
}

void AExoWeaponBase::AddEnergy(float Amount)
{
	float OldEnergy = CurrentEnergy;
	CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.f, MaxEnergy);
	if (CurrentEnergy != OldEnergy)
	{
		OnEnergyChanged.Broadcast(CurrentEnergy);
		// Reset low energy warning when energy climbs back above 25%
		float EnergyPct = (MaxEnergy > 0.f) ? CurrentEnergy / MaxEnergy : 0.f;
		if (EnergyPct >= 0.25f)
			bLowEnergyWarningPlayed = false;
	}
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
