#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoMuzzleSmoke.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Multi-puff muzzle smoke cloud that lingers after firing.
 * Drifts upward, turbulates, and expands before fading.
 */
UCLASS()
class AExoMuzzleSmoke : public AActor
{
	GENERATED_BODY()

public:
	AExoMuzzleSmoke();

	void InitSmoke(const FVector& DriftDir);
	virtual void Tick(float DeltaTime) override;

	static void SpawnSmoke(UWorld* World, const FVector& MuzzlePos,
		const FRotator& MuzzleRot);

private:
	static constexpr int32 NUM_PUFFS = 3;

	UPROPERTY()
	UStaticMeshComponent* SmokePuffs[NUM_PUFFS];

	UPROPERTY()
	UMaterialInstanceDynamic* SmokeMats[NUM_PUFFS];

	FVector DriftVelocities[NUM_PUFFS];
	float Age = 0.f;
	float Lifetime = 0.7f;
	float BaseScale = 0.15f;
};
