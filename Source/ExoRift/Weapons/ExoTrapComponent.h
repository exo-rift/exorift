#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoTrapComponent.generated.h"

class AExoProximityMine;
class AExoCharacter;

/**
 * Component attached to ExoCharacter for managing deployable traps.
 * Tracks ammo count and spawns proximity mines.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoTrapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoTrapComponent();

	/** Deploy a proximity mine at the given location. Returns true if placed. */
	bool PlaceTrap(FVector Location, FRotator Rotation);

	/** Add mines to the inventory (e.g. from a pickup). */
	void AddTraps(int32 Count);

	int32 GetTrapCount() const { return TrapCount; }
	int32 GetMaxTraps() const { return MaxTraps; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	int32 MaxTraps = 2;

private:
	int32 TrapCount = 0;
};
