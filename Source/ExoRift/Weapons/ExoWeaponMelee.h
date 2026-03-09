#pragma once

#include "CoreMinimal.h"
#include "Weapons/ExoWeaponBase.h"
#include "ExoWeaponMelee.generated.h"

/**
 * Plasma Blade — melee weapon using a sphere sweep instead of a line trace.
 * Always available as a fallback; does not occupy an inventory slot.
 */
UCLASS()
class EXORIFT_API AExoWeaponMelee : public AExoWeaponBase
{
	GENERATED_BODY()

public:
	AExoWeaponMelee();

protected:
	virtual void FireShot() override;

private:
	/** Sphere sweep radius (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float SweepRadius = 50.f;

	/** Forward reach of the swing (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float SweepRange = 200.f;

	/** Forward lunge impulse applied to the owner on each swing. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float LungeDistance = 200.f;

	/** Apply a forward lunge to the owning pawn. */
	void ApplyLunge();
};
