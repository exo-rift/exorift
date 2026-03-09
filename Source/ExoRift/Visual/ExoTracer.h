#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoTracer.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Traveling energy tracer for hitscan weapons.
 * Three-layer beam: bright core cylinder, outer glow cylinder, and emissive head.
 * Leaves a fading trail behind the main bolt.
 */
UCLASS()
class AExoTracer : public AActor
{
	GENERATED_BODY()

public:
	AExoTracer();

	void InitTracer(const FVector& Start, const FVector& End, bool bIsHit);

	virtual void Tick(float DeltaTime) override;

private:
	void UpdateTraveling(float DeltaTime);
	void UpdateFading(float DeltaTime);

	UPROPERTY()
	UStaticMeshComponent* BeamCore;

	UPROPERTY()
	UStaticMeshComponent* BeamGlow;

	UPROPERTY()
	UStaticMeshComponent* BeamTrail;

	UPROPERTY()
	UStaticMeshComponent* HeadMesh;

	UPROPERTY()
	UPointLightComponent* HeadLight;

	UPROPERTY()
	UMaterialInstanceDynamic* CoreMat;

	UPROPERTY()
	UMaterialInstanceDynamic* GlowMat;

	UPROPERTY()
	UMaterialInstanceDynamic* TrailMat;

	UPROPERTY()
	UMaterialInstanceDynamic* HeadMat;

	FVector StartPos;
	FVector EndPos;
	FVector Direction;
	float TotalDistance = 0.f;
	float TraveledDist = 0.f;

	float TravelSpeed = 120000.f;
	float BeamLength = 1200.f;

	float FadeAge = 0.f;
	float FadeTime = 0.12f;
	bool bReachedEnd = false;

	FLinearColor CoreColor;
	FLinearColor GlowColor;

	// Cached meshes
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
};
