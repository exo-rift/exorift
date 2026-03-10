#include "Weapons/ExoWeaponRifle.h"
#include "ExoRift.h"

AExoWeaponRifle::AExoWeaponRifle()
{
	WeaponName = TEXT("Pulse Rifle");
	WeaponType = EWeaponType::Rifle;
	bIsAutomatic = true;
	FireRate = 10.f;
	Damage = 15.f;
	MaxRange = 50000.f;
	HeatPerShot = 0.04f;
	CooldownRate = 0.15f;
	OverheatCooldownRate = 0.08f;
	BaseSpread = 0.3f;
	MaxSpread = 3.5f;
	SpreadPerShot = 0.2f;
	SpreadRecoveryRate = 4.f;
	RecoilPitch = -0.2f;
	RecoilYawRange = 0.1f;
	HeadshotMultiplier = 2.f;
	FalloffStartRange = 15000.f;
	FalloffEndRange = 40000.f;
	MinDamageMultiplier = 0.4f;
	MaxEnergy = 200.f;
	CurrentEnergy = 200.f;
	EnergyPerShot = 1.f;

	// ADS: moderate zoom, tighter spread
	ADSFOV = 65.f;
	ADSSpreadMultiplier = 0.4f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(
		TEXT("/Game/Weapons/Rifle/SKM_Rifle"));
	if (RifleMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(RifleMesh.Object);
	}
}

void AExoWeaponRifle::StartFire()
{
	Super::StartFire();
	if (FireMode == ERifleFireMode::Burst && !bInBurstCooldown && BurstShotsRemaining <= 0)
	{
		BurstShotsRemaining = 3;
		BurstTimer = 0.f; // Fire immediately
	}
}

void AExoWeaponRifle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FireMode != ERifleFireMode::Burst) return;

	// Handle burst fire timing
	if (bInBurstCooldown)
	{
		BurstTimer -= DeltaTime;
		if (BurstTimer <= 0.f)
		{
			bInBurstCooldown = false;
			// Start new burst if still holding fire
			if (bWantsToFire && BurstShotsRemaining <= 0)
				BurstShotsRemaining = 3;
		}
		return;
	}

	if (BurstShotsRemaining > 0 && !bIsOverheated && CurrentEnergy >= EnergyPerShot)
	{
		BurstTimer -= DeltaTime;
		if (BurstTimer <= 0.f)
		{
			FireShot();
			BurstShotsRemaining--;
			BurstTimer = BurstFireInterval;

			if (BurstShotsRemaining <= 0)
			{
				bInBurstCooldown = true;
				BurstTimer = BurstCooldown;
			}
		}
	}
}

void AExoWeaponRifle::ToggleFireMode()
{
	if (FireMode == ERifleFireMode::FullAuto)
	{
		FireMode = ERifleFireMode::Burst;
		bIsAutomatic = false;
		BurstShotsRemaining = 0;
		bInBurstCooldown = false;
		UE_LOG(LogExoRift, Log, TEXT("Pulse Rifle: Burst mode"));
	}
	else
	{
		FireMode = ERifleFireMode::FullAuto;
		bIsAutomatic = true;
		UE_LOG(LogExoRift, Log, TEXT("Pulse Rifle: Full-auto mode"));
	}
}
