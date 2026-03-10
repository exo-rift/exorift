// ExoWeaponBase.cpp — Constructor, Tick, heat/energy, sway, rarity helpers
// FireShot & DoLineTrace are in ExoWeaponCombat.cpp
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Visual/ExoWeaponViewModel.h"
#include "Core/ExoAudioManager.h"
#include "Core/ExoPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
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

	// Movement bob — weapon rocks when walking/sprinting
	float MoveSpeed = OwnerPawn->GetVelocity().Size2D();
	if (MoveSpeed > 50.f)
	{
		AExoCharacter* ExoChar = Cast<AExoCharacter>(OwnerPawn);
		bool bSprint = ExoChar && ExoChar->IsSprinting();
		float BobFreq = bSprint ? 10.f : 7.f;
		float BobAmpY = bSprint ? 0.8f : 0.35f;
		float BobAmpZ = bSprint ? 0.5f : 0.25f;
		float SpeedFactor = FMath::Clamp(MoveSpeed / 600.f, 0.f, 1.f);
		MoveBobTimer += DeltaTime * BobFreq;
		FVector TargetBob(0.f,
			FMath::Sin(MoveBobTimer) * BobAmpY * SpeedFactor,
			FMath::Abs(FMath::Sin(MoveBobTimer)) * BobAmpZ * SpeedFactor);
		MoveBobOffset = FMath::VInterpTo(MoveBobOffset, TargetBob, DeltaTime, 12.f);
	}
	else
	{
		MoveBobOffset = FMath::VInterpTo(MoveBobOffset, FVector::ZeroVector, DeltaTime, 8.f);
	}

	// ADS position: weapon centers and moves forward
	FVector HipPos(20.f, 10.f, -8.f);
	FVector ADSPos(30.f, 0.f, -2.f); // Centered, closer to eye
	FVector BasePos = FMath::Lerp(HipPos, ADSPos, ADSBlend);

	// Reduce sway/idle/bob while ADS
	float SwayScale = FMath::Lerp(1.f, 0.15f, ADSBlend);
	float IdleScale = FMath::Lerp(1.f, 0.1f, ADSBlend);
	float BobScale = FMath::Lerp(1.f, 0.05f, ADSBlend);

	// Draw animation offset — weapon rises from below when equipped
	FVector DrawOffset = FVector(0.f, 0.f, -30.f * (1.f - DrawBlend));
	float DrawRotOffset = -25.f * (1.f - DrawBlend);

	// Inspect animation — weapon tilts toward camera for examination
	FVector InspectOffset = FVector::ZeroVector;
	FRotator InspectRotOffset = FRotator::ZeroRotator;
	if (InspectBlend > 0.01f)
	{
		// Bring weapon to center, tilt it
		InspectOffset = FVector(8.f, -8.f, 3.f) * InspectBlend;
		float RotAngle = FMath::Sin(InspectPhase * 0.8f) * 25.f;
		float TiltAngle = FMath::Sin(InspectPhase * 0.5f + 1.f) * 10.f;
		InspectRotOffset = FRotator(TiltAngle, RotAngle, 15.f) * InspectBlend;
	}

	ViewModel->SetRelativeLocation(BasePos
		+ SwayOffset * SwayScale
		+ RecoilOffset
		+ MoveBobOffset * BobScale
		+ FVector(0.f, IdleY * IdleScale, IdleZ * IdleScale)
		+ DrawOffset
		+ InspectOffset);
	ViewModel->SetRelativeRotation(
		FRotator(RecoilRotation + DrawRotOffset, 0.f, 0.f) + InspectRotOffset);
}

void AExoWeaponBase::StartInspect()
{
	if (bWantsToFire || bIsADS) return;
	bIsInspecting = true;
}

void AExoWeaponBase::StopInspect()
{
	bIsInspecting = false;
}

void AExoWeaponBase::TickHeatGlow()
{
	if (!ViewModel || !ViewModel->IsRegistered()) return;

	// Update barrel material heat glow
	ViewModel->UpdateHeatGlow(CurrentHeat);

	// Update muzzle ready light based on heat — cool accent when ready, hot orange when hot
	UPointLightComponent* Light = nullptr;
	TArray<USceneComponent*> VMChildren;
	ViewModel->GetChildrenComponents(false, VMChildren);
	for (auto* Child : VMChildren)
	{
		if (UPointLightComponent* PL = Cast<UPointLightComponent>(Child))
		{
			Light = PL;
			break;
		}
	}
	if (!Light) return;

	if (CurrentHeat > 0.2f)
	{
		float HeatAlpha = FMath::Clamp((CurrentHeat - 0.2f) / 0.8f, 0.f, 1.f);
		FLinearColor HotColor(1.f, 0.3f, 0.05f);
		FLinearColor CoolColor = GetRarityColor(Rarity);
		Light->SetLightColor(FMath::Lerp(CoolColor, HotColor, HeatAlpha));
		Light->SetIntensity(FMath::Lerp(800.f, 5000.f, HeatAlpha));
		Light->SetAttenuationRadius(FMath::Lerp(120.f, 300.f, HeatAlpha));
	}
	else
	{
		Light->SetLightColor(GetRarityColor(Rarity));
		Light->SetIntensity(800.f);
		Light->SetAttenuationRadius(120.f);
	}
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
