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

void AExoWeaponBase::StartADS()
{
	bIsADS = true;
}

void AExoWeaponBase::StopADS()
{
	bIsADS = false;
}

void AExoWeaponBase::ToggleFireMode()
{
	// Base weapons don't have fire mode toggle — override in subclass
}

void AExoWeaponBase::PlayDrawAnimation()
{
	DrawBlend = 0.f;
	bDrawAnimActive = true;
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

	// Smooth ADS blend
	ADSBlend = FMath::FInterpTo(ADSBlend, bIsADS ? 1.f : 0.f, DeltaTime, ADSBlendSpeed);

	FRotator CurrentRot = PC->GetControlRotation();
	FRotator DeltaRot = CurrentRot - PrevControlRotation;
	DeltaRot.Normalize();
	PrevControlRotation = CurrentRot;

	SwayOffset.Y += DeltaRot.Yaw * SwayAmount;
	SwayOffset.Z += DeltaRot.Pitch * SwayAmount;

	SwayOffset.Y = FMath::Clamp(SwayOffset.Y, -MaxSwayOffset, MaxSwayOffset);
	SwayOffset.Z = FMath::Clamp(SwayOffset.Z, -MaxSwayOffset, MaxSwayOffset);

	SwayOffset = FMath::VInterpTo(SwayOffset, FVector::ZeroVector, DeltaTime, SwayReturnSpeed);

	// Recoil kick recovery
	RecoilOffset = FMath::VInterpTo(RecoilOffset, FVector::ZeroVector, DeltaTime, RecoilRecoverySpeed);
	RecoilRotation = FMath::FInterpTo(RecoilRotation, 0.f, DeltaTime, RecoilRecoverySpeed);

	float Time = GetWorld()->GetTimeSeconds();
	float IdleY = FMath::Sin(Time * 1.2f) * 0.15f;
	float IdleZ = FMath::Sin(Time * 0.8f + 0.7f) * 0.1f;

	// ADS position: weapon centers and moves forward
	FVector HipPos(20.f, 10.f, -8.f);
	FVector ADSPos(30.f, 0.f, -2.f); // Centered, closer to eye
	FVector BasePos = FMath::Lerp(HipPos, ADSPos, ADSBlend);

	// Reduce sway/idle while ADS
	float SwayScale = FMath::Lerp(1.f, 0.15f, ADSBlend);
	float IdleScale = FMath::Lerp(1.f, 0.1f, ADSBlend);

	// Draw animation offset — weapon rises from below when equipped
	FVector DrawOffset = FVector(0.f, 0.f, -30.f * (1.f - DrawBlend));
	float DrawRotOffset = -25.f * (1.f - DrawBlend);

	ViewModel->SetRelativeLocation(BasePos
		+ SwayOffset * SwayScale
		+ RecoilOffset
		+ FVector(0.f, IdleY * IdleScale, IdleZ * IdleScale)
		+ DrawOffset);
	ViewModel->SetRelativeRotation(FRotator(RecoilRotation + DrawRotOffset, 0.f, 0.f));
}

void AExoWeaponBase::ApplyRecoilKick()
{
	// Per-weapon recoil kick strength
	float KickBack = 0.f;
	float KickUp = 0.f;
	float KickRot = 0.f;
	switch (WeaponType)
	{
	case EWeaponType::Rifle:          KickBack = -3.f; KickUp = 0.8f; KickRot = -4.f; break;
	case EWeaponType::SMG:            KickBack = -1.5f; KickUp = 0.4f; KickRot = -2.f; break;
	case EWeaponType::Pistol:         KickBack = -2.5f; KickUp = 1.0f; KickRot = -5.f; break;
	case EWeaponType::Shotgun:        KickBack = -6.f; KickUp = 2.0f; KickRot = -8.f; break;
	case EWeaponType::Sniper:         KickBack = -8.f; KickUp = 2.5f; KickRot = -10.f; break;
	case EWeaponType::GrenadeLauncher: KickBack = -5.f; KickUp = 1.5f; KickRot = -7.f; break;
	default: KickBack = -2.f; KickUp = 0.5f; KickRot = -3.f; break;
	}
	RecoilOffset += FVector(KickBack, 0.f, KickUp);
	RecoilRotation += KickRot;
}
