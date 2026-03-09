#pragma once

#include "CoreMinimal.h"
#include "Weapons/ExoWeaponBase.h"
#include "ExoWeaponShotgun.generated.h"

UCLASS()
class EXORIFT_API AExoWeaponShotgun : public AExoWeaponBase
{
	GENERATED_BODY()

public:
	AExoWeaponShotgun();

protected:
	virtual void FireShot() override;

	/** Number of pellets per shot. */
	UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
	int32 NumPellets = 8;

	/** Half-angle cone spread in degrees for pellet deviation. */
	UPROPERTY(EditDefaultsOnly, Category = "Shotgun")
	float PelletSpreadAngle = 5.f;

private:
	/** Fire a single pellet trace with random cone deviation and apply damage. */
	void FirePellet(const FVector& Start, const FRotator& AimDir, APawn* OwnerPawn);
};
