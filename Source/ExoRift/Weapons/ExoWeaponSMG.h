#pragma once

#include "CoreMinimal.h"
#include "Weapons/ExoWeaponBase.h"
#include "ExoWeaponSMG.generated.h"

UCLASS()
class EXORIFT_API AExoWeaponSMG : public AExoWeaponBase
{
	GENERATED_BODY()

public:
	AExoWeaponSMG();

protected:
	virtual void FireShot() override;

	/** Per-shot random deviation in degrees. */
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float ShotDeviation = 1.5f;
};
