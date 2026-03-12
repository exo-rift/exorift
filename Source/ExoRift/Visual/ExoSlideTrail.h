#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoSlideTrail.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Short-lived energy streak on the ground spawned at the player's feet during a slide.
 * Flattened cube with additive emissive material, cyan-blue sci-fi glow.
 * Expands slightly and fades over its brief lifetime.
 */
UCLASS()
class AExoSlideTrail : public AActor
{
	GENERATED_BODY()

public:
	AExoSlideTrail();
	virtual void Tick(float DeltaTime) override;

	/** Spawn a slide trail mark at the given world location. */
	static void SpawnMark(UWorld* World, const FVector& Location);

private:
	UPROPERTY()
	UStaticMeshComponent* Streak;

	UPROPERTY()
	UPointLightComponent* GroundLight;

	UPROPERTY()
	UMaterialInstanceDynamic* StreakMat = nullptr;

	float Age = 0.f;

	static constexpr float Lifetime = 0.6f;

	/** Initial streak dimensions (X = length along slide, Y = width, Z = flat). */
	FVector BaseScale = FVector(0.6f, 0.15f, 0.008f);
	float BaseLightIntensity = 8000.f;
	FLinearColor EmissiveColor;
};
