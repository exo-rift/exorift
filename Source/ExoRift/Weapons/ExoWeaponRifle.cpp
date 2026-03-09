#include "Weapons/ExoWeaponRifle.h"

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

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> RifleMesh(
		TEXT("/Game/Weapons/Rifle/SKM_Rifle"));
	if (RifleMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(RifleMesh.Object);
	}
}
