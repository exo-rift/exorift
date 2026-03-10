#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoImpactEffect.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/** Dramatic impact burst at bullet hit locations with sparks, shockwave ring, and dust. */
UCLASS()
class AExoImpactEffect : public AActor
{
	GENERATED_BODY()

public:
	AExoImpactEffect();

	/** Configure the effect at spawn time with weapon-specific color. */
	void InitEffect(const FVector& HitNormal, bool bHitCharacter,
		const FLinearColor& WeaponColor = FLinearColor(0.3f, 0.7f, 1.f));

	virtual void Tick(float DeltaTime) override;

private:
	static constexpr int32 IMPACT_NUM_SPARKS = 10;

	UPROPERTY()
	UStaticMeshComponent* CoreMesh;

	UPROPERTY()
	UStaticMeshComponent* DustPuff;

	UPROPERTY()
	UStaticMeshComponent* ShockwaveRing;

	UPROPERTY()
	UPointLightComponent* FlashLight;

	UPROPERTY()
	TArray<UStaticMeshComponent*> SparkMeshes;

	TArray<FVector> SparkVelocities;

	FVector HitNorm = FVector::UpVector;
	float Age = 0.f;
	float Lifetime = 0.5f;
	float BaseIntensity = 0.f;
};
