#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoImpactEffect.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

static constexpr int32 NUM_SPARKS = 5;

/** Short-lived impact burst at bullet hit locations with sparks and dust. */
UCLASS()
class AExoImpactEffect : public AActor
{
	GENERATED_BODY()

public:
	AExoImpactEffect();

	/** Configure the effect at spawn time. */
	void InitEffect(const FVector& HitNormal, bool bHitCharacter);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* CoreMesh;

	UPROPERTY()
	UStaticMeshComponent* DustPuff;

	UPROPERTY()
	UPointLightComponent* FlashLight;

	UPROPERTY()
	TArray<UStaticMeshComponent*> SparkMeshes;

	TArray<FVector> SparkVelocities;

	float Age = 0.f;
	float Lifetime = 0.2f;
	float BaseIntensity = 0.f;
};
