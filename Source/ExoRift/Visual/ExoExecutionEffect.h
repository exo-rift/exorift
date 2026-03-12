#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoExecutionEffect.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/** Dramatic slow-motion execution finisher VFX — distinct from normal death. */
UCLASS()
class AExoExecutionEffect : public AActor
{
	GENERATED_BODY()

public:
	AExoExecutionEffect();

	void Init(const FLinearColor& AccentColor, const FVector& ExecutorPos);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* CoreVortex;

	UPROPERTY()
	UStaticMeshComponent* InnerRing;

	UPROPERTY()
	UStaticMeshComponent* OuterRing;

	UPROPERTY()
	UStaticMeshComponent* EnergyColumn;

	UPROPERTY()
	UStaticMeshComponent* GroundBurn;

	UPROPERTY()
	UPointLightComponent* FlashLight;

	UPROPERTY()
	UPointLightComponent* ColumnLight;

	// Orbiting energy shards — spiraling fragments around the vortex
	static constexpr int32 NumShards = 12;
	UPROPERTY()
	TArray<UStaticMeshComponent*> Shards;
	TArray<float> ShardAngles;
	TArray<float> ShardHeights;

	// Converging beams from executor toward target
	static constexpr int32 NumBeams = 6;
	UPROPERTY()
	TArray<UStaticMeshComponent*> Beams;

	UPROPERTY()
	UMaterialInstanceDynamic* VortexMat;

	UPROPERTY()
	UMaterialInstanceDynamic* RingMat;

	FLinearColor Accent;
	FVector ExecutorDirection;
	float Age = 0.f;
	static constexpr float Lifetime = 1.8f;
};
