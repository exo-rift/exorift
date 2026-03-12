#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShieldShatter.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Dramatic shield break VFX — expanding hexagonal fragments scatter outward
 * with blue energy burst when a player's shield is depleted.
 */
UCLASS()
class AExoShieldShatter : public AActor
{
	GENERATED_BODY()

public:
	AExoShieldShatter();
	virtual void Tick(float DeltaTime) override;

	void InitShatter(const FVector& HitDirection);

	static void SpawnShatter(UWorld* World, const FVector& Location,
		const FVector& HitDirection);

private:
	static constexpr int32 NUM_FRAGMENTS = 12;
	static constexpr int32 NUM_SPARKS = 8;

	UPROPERTY()
	UStaticMeshComponent* CoreBurst;

	UPROPERTY()
	UStaticMeshComponent* ShockwaveRing;

	UPROPERTY()
	UPointLightComponent* BurstLight;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Fragments;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Sparks;

	TArray<FVector> FragVelocities;
	TArray<FVector> SparkVelocities;
	TArray<float> FragRotSpeeds;

	float Age = 0.f;
	float Lifetime = 0.8f;
};
