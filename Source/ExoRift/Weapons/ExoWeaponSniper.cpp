#include "Weapons/ExoWeaponSniper.h"

AExoWeaponSniper::AExoWeaponSniper()
{
	WeaponName = TEXT("Void Lance");
	WeaponType = EWeaponType::Sniper;
	bIsAutomatic = false;
	FireRate = 1.f;
	Damage = 120.f;
	MaxRange = 80000.f;

	// Heat system: 35/100 normalized to 0-1 range
	HeatPerShot = 0.35f;
	CooldownRate = 0.2f;
	OverheatCooldownRate = 0.07f; // Slow recovery from overheat (penalty = 3.0s feel)
	OverheatRecoveryThreshold = 0.3f;

	// Energy cell
	MaxEnergy = 20.f;
	CurrentEnergy = 20.f;
	EnergyPerShot = 5.f;

	// Long-range precision: minimal spread
	BaseSpread = 0.f;
	MaxSpread = 1.f;
	SpreadPerShot = 0.8f;
	SpreadRecoveryRate = 2.f;

	// Heavy recoil for a high-caliber weapon
	RecoilPitch = -1.2f;
	RecoilYawRange = 0.3f;

	// Headshot
	HeadshotMultiplier = 3.f;

	// Damage falloff: full damage to 400m, falloff to 800m
	FalloffStartRange = 40000.f;
	FalloffEndRange = 80000.f;
	MinDamageMultiplier = 0.5f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SniperMesh(
		TEXT("/Game/Weapons/Sniper/SKM_Sniper"));
	if (SniperMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(SniperMesh.Object);
	}
}
