#include "Weapons/ExoWeaponGrenadeLauncher.h"
#include "Weapons/ExoProjectile.h"

AExoWeaponGrenadeLauncher::AExoWeaponGrenadeLauncher()
{
	WeaponName = TEXT("Nova Launcher");
	WeaponType = EWeaponType::GrenadeLauncher;
	bIsAutomatic = false;
	FireRate = 1.f;
	Damage = 80.f;
	MaxRange = 20000.f;
	HeatPerShot = 0.35f;
	CooldownRate = 0.12f;
	OverheatCooldownRate = 0.06f;
	MaxEnergy = 30.f;
	CurrentEnergy = 30.f;
	EnergyPerShot = 10.f;
	ProjectileClass = AExoProjectile::StaticClass();

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> LauncherMesh(
		TEXT("/Game/Weapons/GrenadeLauncher/SKM_GrenadeLauncher"));
	if (LauncherMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(LauncherMesh.Object);
	}
}

void AExoWeaponGrenadeLauncher::FireShot()
{
	if (CurrentEnergy < EnergyPerShot) return;
	CurrentEnergy = FMath::Max(CurrentEnergy - EnergyPerShot, 0.f);
	OnEnergyChanged.Broadcast(CurrentEnergy);

	AddHeat(HeatPerShot);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !ProjectileClass) return;

	AController* PC = OwnerPawn->GetController();
	if (!PC) return;

	FVector SpawnLoc;
	FRotator SpawnRot;
	PC->GetPlayerViewPoint(SpawnLoc, SpawnRot);
	SpawnLoc += SpawnRot.Vector() * 100.f; // Offset forward from camera

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = OwnerPawn;

	AExoProjectile* Projectile = GetWorld()->SpawnActor<AExoProjectile>(
		ProjectileClass, SpawnLoc, SpawnRot, SpawnParams);
	if (Projectile)
	{
		Projectile->InitProjectile(SpawnRot.Vector() * ProjectileSpeed, Damage, PC);
	}
}
