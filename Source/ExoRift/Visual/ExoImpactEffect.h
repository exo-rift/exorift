#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoImpactEffect.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/** Short-lived impact burst at bullet hit locations. */
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
	UStaticMeshComponent* SparkMesh1;

	UPROPERTY()
	UStaticMeshComponent* SparkMesh2;

	UPROPERTY()
	UStaticMeshComponent* SparkMesh3;

	UPROPERTY()
	UPointLightComponent* FlashLight;

	float Age = 0.f;
	float Lifetime = 0.15f;
	float BaseIntensity = 0.f;
	FVector SparkVel1;
	FVector SparkVel2;
	FVector SparkVel3;
};
