#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoTracer.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Traveling beam tracer for hitscan weapons.
 * Flies from muzzle to impact point with a bright head, core beam, and outer glow.
 */
UCLASS()
class AExoTracer : public AActor
{
	GENERATED_BODY()

public:
	AExoTracer();

	/** Set up the tracer between two points. Call immediately after spawn. */
	void InitTracer(const FVector& Start, const FVector& End, bool bIsHit);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* BeamCore;

	UPROPERTY()
	UStaticMeshComponent* BeamGlow;

	UPROPERTY()
	UStaticMeshComponent* HeadMesh;

	UPROPERTY()
	UPointLightComponent* HeadLight;

	UPROPERTY()
	UMaterialInstanceDynamic* CoreMat;

	UPROPERTY()
	UMaterialInstanceDynamic* GlowMat;

	FVector StartPos;
	FVector EndPos;
	FVector Direction;
	float TotalDistance = 0.f;
	float TraveledDist = 0.f;

	float TravelSpeed = 100000.f;
	float BeamLength = 800.f;

	float FadeAge = 0.f;
	float FadeTime = 0.08f;
	bool bReachedEnd = false;

	FLinearColor CoreColor;
	FLinearColor GlowColor;
};
