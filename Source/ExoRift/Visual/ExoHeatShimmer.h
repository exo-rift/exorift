#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoHeatShimmer.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Heat shimmer rising from weapon vents — spawned when weapon heat > 50%.
 * Self-destructing actor with rising, expanding, fading wisps + heat glow light.
 */
UCLASS()
class AExoHeatShimmer : public AActor
{
	GENERATED_BODY()

public:
	AExoHeatShimmer();

	void InitShimmer(float Intensity);

	virtual void Tick(float DeltaTime) override;

	static void SpawnShimmer(UWorld* World, const FVector& Pos, float Intensity);

private:
	static constexpr int32 NUM_WISPS = 6;

	UPROPERTY()
	UStaticMeshComponent* Wisps[NUM_WISPS];

	UPROPERTY()
	UStaticMeshComponent* HeatRing;

	UPROPERTY()
	UPointLightComponent* HeatLight;

	float Age = 0.f;
	float Lifetime = 0.6f;
	float IntensityScale = 1.f;
	FVector Velocities[NUM_WISPS];
};
