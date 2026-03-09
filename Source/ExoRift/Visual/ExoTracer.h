#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoTracer.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Traveling energy tracer for hitscan weapons.
 * Multi-layer beam with weapon-specific color and sizing.
 */
UCLASS()
class AExoTracer : public AActor
{
	GENERATED_BODY()

public:
	AExoTracer();

	void InitTracer(const FVector& Start, const FVector& End, bool bIsHit,
		const FLinearColor& WeaponColor, EWeaponType WeaponType);

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
	UPointLightComponent* TailLight;

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

	// Per-weapon sizing
	float TravelSpeed = 120000.f;
	float BeamLength = 1200.f;
	float CoreRadius = 0.012f;
	float GlowRadius = 0.035f;
	float TrailRadius = 0.006f;
	float HeadScale = 0.15f;
	float LightIntensity = 20000.f;

	float FadeAge = 0.f;
	float FadeTime = 0.15f;
	bool bReachedEnd = false;

	FLinearColor CoreColor;
	FLinearColor GlowColor;

	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
};
