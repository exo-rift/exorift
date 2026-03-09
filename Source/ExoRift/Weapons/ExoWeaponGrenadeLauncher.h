#pragma once

#include "CoreMinimal.h"
#include "Weapons/ExoWeaponBase.h"
#include "ExoWeaponGrenadeLauncher.generated.h"

class AExoProjectile;

UCLASS()
class EXORIFT_API AExoWeaponGrenadeLauncher : public AExoWeaponBase
{
	GENERATED_BODY()

public:
	AExoWeaponGrenadeLauncher();

protected:
	virtual void FireShot() override;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AExoProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ProjectileSpeed = 3000.f;
};
