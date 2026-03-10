#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoFootstepDust.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Multi-particle dust burst at character feet.
 * Scales with movement mode and surface type.
 * Self-destructs after lifetime expires.
 */
UCLASS()
class AExoFootstepDust : public AActor
{
	GENERATED_BODY()

public:
	AExoFootstepDust();

	virtual void Tick(float DeltaTime) override;

	/** Configure dust style. bLanding = larger burst for hard landings. */
	void InitDust(bool bLanding, float Intensity = 1.f, const FLinearColor& SurfaceColor = FLinearColor(0.35f, 0.3f, 0.25f));

	/** Utility: spawn footstep dust — now works for walk, sprint, and crouch. */
	static void SpawnFootstepDust(UWorld* World, const FVector& Location, bool bSprinting);

	/** Utility: spawn a landing dust cloud. FallSpeed scales intensity. */
	static void SpawnLandingDust(UWorld* World, const FVector& Location, float FallSpeed);

private:
	static constexpr int32 NUM_PUFFS = 4;

	UPROPERTY()
	UStaticMeshComponent* Puffs[NUM_PUFFS];

	UPROPERTY()
	UPointLightComponent* DustLight;

	FVector PuffVelocities[NUM_PUFFS];

	float Age = 0.f;
	float Lifetime = 0.4f;
	float MaxScale = 0.3f;
	float LightIntensity = 0.f;
};
