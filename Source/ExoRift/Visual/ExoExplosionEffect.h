#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoExplosionEffect.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Expanding fireball + shockwave ring + debris for explosions. */
UCLASS()
class AExoExplosionEffect : public AActor
{
	GENERATED_BODY()

public:
	AExoExplosionEffect();

	/** Set up explosion visuals. Call immediately after spawn. */
	void InitExplosion(float Radius);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* FireballMesh;

	UPROPERTY()
	UStaticMeshComponent* InnerFlashMesh;

	UPROPERTY()
	UStaticMeshComponent* ShockwaveRing;

	UPROPERTY()
	UPointLightComponent* ExplosionLight;

	UPROPERTY()
	UPointLightComponent* FlashLight;

	/** Ground scorch mark left after explosion fades. */
	UPROPERTY()
	UStaticMeshComponent* ScorchMark;

	/** Secondary upward smoke column. */
	UPROPERTY()
	UStaticMeshComponent* SmokeColumn;

	// Debris chunks
	UPROPERTY()
	TArray<UStaticMeshComponent*> DebrisMeshes;

	TArray<FVector> DebrisVelocities;

	float Age = 0.f;
	float Lifetime = 0.8f;
	float ExpRadius = 500.f;
	float BaseIntensity = 0.f;
};
