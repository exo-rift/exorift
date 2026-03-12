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
	UStaticMeshComponent* Corona; // Outer soft halo

	UPROPERTY()
	UStaticMeshComponent* HeadMesh;

	UPROPERTY()
	UPointLightComponent* HeadLight;

	UPROPERTY()
	UPointLightComponent* TailLight;

	// Trailing sparks — glowing energy fragments that scatter behind the bolt
	static constexpr int32 NUM_SPARKS = 8;
	UPROPERTY()
	TArray<UStaticMeshComponent*> SparkMeshes;
	TArray<FVector> SparkOffsets;
	TArray<FVector> SparkVelocities;

	// Helix orbiters — small spheres spiraling around the beam
	static constexpr int32 NUM_HELIX = 4;
	UPROPERTY()
	TArray<UStaticMeshComponent*> HelixOrbs;
	float HelixAngle = 0.f;
	float HelixSpeed = 28.f;  // Revolutions per second (radians)
	float HelixRadius = 0.f;  // Set per-weapon

	// Head shockwave ring — expanding ring at the bolt front
	UPROPERTY()
	UStaticMeshComponent* HeadRing;

	UPROPERTY()
	UMaterialInstanceDynamic* CoreMat;

	UPROPERTY()
	UMaterialInstanceDynamic* GlowMat;

	UPROPERTY()
	UMaterialInstanceDynamic* TrailMat;

	UPROPERTY()
	UMaterialInstanceDynamic* CoronaMat;

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
	float CoreRadius = 0.03f;
	float GlowRadius = 0.09f;
	float TrailRadius = 0.015f;
	float CoronaRadius = 0.18f;
	float HeadScale = 0.30f;
	float LightIntensity = 55000.f;

	float FadeAge = 0.f;
	float FadeTime = 0.15f;
	bool bReachedEnd = false;

	FLinearColor CoreColor;
	FLinearColor GlowColor;
	FLinearColor BaseWeaponColor;

	UPROPERTY()
	UStaticMesh* CylinderMesh = nullptr;
	UPROPERTY()
	UStaticMesh* SphereMesh = nullptr;
	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
};
