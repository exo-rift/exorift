#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoHeatShimmer.generated.h"

class UStaticMeshComponent;

/**
 * Heat shimmer rising from weapon vents — spawned when weapon heat > 50%.
 * Self-destructing actor with rising, expanding, fading smoke wisps.
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
	static constexpr int32 NUM_WISPS = 3;

	UPROPERTY()
	UStaticMeshComponent* Wisps[NUM_WISPS];

	float Age = 0.f;
	float Lifetime = 0.6f;
	float IntensityScale = 1.f;
	FVector Velocities[NUM_WISPS];
};
