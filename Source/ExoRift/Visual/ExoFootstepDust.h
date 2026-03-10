#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoFootstepDust.generated.h"

class UStaticMeshComponent;

/**
 * Small dust puff at character feet when running, and larger cloud on landing.
 * Self-destructs after lifetime expires.
 */
UCLASS()
class AExoFootstepDust : public AActor
{
	GENERATED_BODY()

public:
	AExoFootstepDust();

	virtual void Tick(float DeltaTime) override;

	/** Configure puff scale. bLanding = larger burst for hard landings. */
	void InitDust(bool bLanding = false);

	/** Utility: spawn a footstep puff at location. */
	static void SpawnFootstepDust(UWorld* World, const FVector& Location, bool bSprinting);

	/** Utility: spawn a landing dust cloud at location. FallSpeed for intensity scaling. */
	static void SpawnLandingDust(UWorld* World, const FVector& Location, float FallSpeed);

private:
	UPROPERTY()
	UStaticMeshComponent* PuffMesh;

	float Age = 0.f;
	float Lifetime = 0.4f;
	float MaxScale = 0.3f;
	float RiseSpeed = 30.f;
};
