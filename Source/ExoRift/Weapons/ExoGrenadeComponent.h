#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	int32 MaxGrenades = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ThrowCooldown = 1.f;

protected:
	int32 CurrentGrenades = 2;

private:
	float LastThrowTime = -10.f;
};
