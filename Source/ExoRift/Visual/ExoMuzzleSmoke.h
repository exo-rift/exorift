#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoMuzzleSmoke.generated.h"

/**
 * Brief smoke wisp that lingers at the muzzle after firing.
 * Drifts upward and expands before fading.
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
	UPROPERTY()
	UStaticMeshComponent* SmokePuff;

	UPROPERTY()
	class UMaterialInstanceDynamic* SmokeMat;

	FVector DriftVelocity;
	float Age = 0.f;
	float Lifetime = 0.6f;
	float BaseScale = 0.15f;
};
