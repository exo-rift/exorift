#include "Weapons/ExoWeaponSMG.h"

AExoWeaponSMG::AExoWeaponSMG()
{
	WeaponName = TEXT("Storm Needle");
	WeaponType = EWeaponType::SMG;
	bIsAutomatic = true;

	// Very high fire rate
	FireRate = 15.f;
	Damage = 11.f; // 15 RPS × 11 = 165 DPS (rewards close-range aggression)

	// Heat system — normalised 0-1 (4/100 per shot)
	HeatPerShot = 0.04f;
	CooldownRate = 0.30f;
	OverheatCooldownRate = 0.10f;
	OverheatRecoveryThreshold = 0.3f;

	// Energy (ammo)
	MaxEnergy = 300.f;
	CurrentEnergy = 300.f;
	EnergyPerShot = 1.f;

	// Short-medium range falloff
	MaxRange = 30000.f;
	FalloffStartRange = 5000.f;   // 50 m
	FalloffEndRange = 15000.f;    // 150 m
	MinDamageMultiplier = 0.3f;

	// Tight base spread, blooms fast
	BaseSpread = 0.4f;
	MaxSpread = 5.f;
	SpreadPerShot = 0.35f;
	SpreadRecoveryRate = 5.f;

	// Light recoil
	RecoilPitch = -0.15f;
	RecoilYawRange = 0.2f;
	HeadshotMultiplier = 1.8f;

	// SMG ADS: minimal zoom, small spread reduction (run-and-gun weapon)
	ADSFOV = 75.f;
	ADSSpreadMultiplier = 0.7f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SMGMesh(
		TEXT("/Game/Weapons/SMG/SKM_SMG"));
	if (SMGMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(SMGMesh.Object);
	}
}

void AExoWeaponSMG::FireShot()
{
	// Inject per-shot random deviation by temporarily adding to CurrentSpread.
	// The base DoLineTrace applies CurrentSpread as a cone; we widen it for this shot.
	float SavedSpread = CurrentSpread;
	CurrentSpread = FMath::Min(CurrentSpread + ShotDeviation, MaxSpread);

	Super::FireShot();

	// Restore spread so only the normal SpreadPerShot accumulation persists.
	// Super::FireShot already added SpreadPerShot, so keep that part.
	float AccumulatedSpread = SavedSpread + SpreadPerShot;
	CurrentSpread = FMath::Min(AccumulatedSpread, MaxSpread);
}
