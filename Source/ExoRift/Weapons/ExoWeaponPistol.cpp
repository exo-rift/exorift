#include "Weapons/ExoWeaponPistol.h"

AExoWeaponPistol::AExoWeaponPistol()
{
	WeaponName = TEXT("Arc Pistol");
	WeaponType = EWeaponType::Pistol;
	bIsAutomatic = false;
	FireRate = 4.f;
	Damage = 25.f;
	MaxRange = 30000.f;
	HeatPerShot = 0.08f;
	CooldownRate = 0.2f;
	OverheatCooldownRate = 0.1f;
	BaseSpread = 0.1f;
	MaxSpread = 2.f;
	SpreadPerShot = 0.5f;
	SpreadRecoveryRate = 5.f;
	RecoilPitch = -0.5f;
	RecoilYawRange = 0.2f;
	HeadshotMultiplier = 2.5f;
	FalloffStartRange = 8000.f;
	FalloffEndRange = 20000.f;
	MinDamageMultiplier = 0.3f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PistolMesh(
		TEXT("/Game/Weapons/Pistol/SKM_Pistol"));
	if (PistolMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(PistolMesh.Object);
	}
}
