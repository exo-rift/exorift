#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/ExoTypes.h"
#include "ExoGrenadeComponent.generated.h"

class AExoGrenade;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXORIFT_API UExoGrenadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoGrenadeComponent();

	/** Spawn and throw a grenade from Origin in the given Direction. */
	void ThrowGrenade(FVector Origin, FRotator Direction);

	/** Add grenades (e.g. from a pickup). */
	void AddGrenades(int32 Count);

	int32 GetCurrentGrenades() const { return CurrentGrenades; }
	int32 GetMaxGrenades() const { return MaxGrenades; }
	EGrenadeType GetGrenadeType() const { return GrenadeType; }

	/** Set which grenade type this component currently holds. */
	void SetGrenadeType(EGrenadeType NewType) { GrenadeType = NewType; }

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	EGrenadeType GrenadeType = EGrenadeType::Frag;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	int32 MaxGrenades = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ThrowCooldown = 1.f;

protected:
	int32 CurrentGrenades = 2;

private:
	float LastThrowTime = -10.f;
};
