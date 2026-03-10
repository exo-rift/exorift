#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDropPod.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class AExoDropPodManager;

UENUM()
enum class EDropPodPhase : uint8
{
	FreeFall,       // High altitude, fast descent, player can steer
	Braking,        // Thrusters fire, decelerating
	Landing,        // Final approach
	Landed          // On ground, deploying player
};

UCLASS()
class EXORIFT_API AExoDropPod : public AActor
{
	GENERATED_BODY()

public:
	AExoDropPod();

	virtual void Tick(float DeltaTime) override;

	void InitPod(AController* InPassenger, AExoDropPodManager* InManager);

	/** Call from player input to steer during freefall. X=left/right, Y=forward/back. */
	void ApplySteerInput(FVector2D Input);

protected:
	void OnLanded();
	void BuildPodMesh();
	void UpdateThrusterVFX(float DeltaTime, float BrakeAlpha);
	void UpdateLandedSequence(float DeltaTime);
	void SpawnLandingDust();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PodMesh;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PodCamera;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* ThrusterLight;

	// Pod hull parts (built from basic shapes)
	UPROPERTY()
	UStaticMeshComponent* HullBody;
	UPROPERTY()
	UStaticMeshComponent* HullNose;
	UPROPERTY()
	UStaticMeshComponent* FinLeft;
	UPROPERTY()
	UStaticMeshComponent* FinRight;
	UPROPERTY()
	UStaticMeshComponent* ThrusterCone;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float MaxDescentSpeed = 12000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float BrakeAltitude = 8000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float LandingSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float SteerSpeed = 4000.f;

	UPROPERTY()
	AController* Passenger = nullptr;

	UPROPERTY()
	AExoDropPodManager* Manager = nullptr;

	EDropPodPhase Phase = EDropPodPhase::FreeFall;
	float GroundZ = 0.f;
	bool bGroundDetected = false;
	FVector2D SteerInput = FVector2D::ZeroVector;
	float PodTilt = 0.f;
	float LandedTimer = 0.f;

	// Camera shake state
	float ShakeIntensity = 0.f;
	float ShakeDecay = 0.f;
	FVector CameraBaseOffset;

	// VFX components built at runtime
	UPROPERTY()
	UStaticMeshComponent* ThrusterFlame = nullptr;
	UPROPERTY()
	UStaticMeshComponent* HeatShield = nullptr;
	UPROPERTY()
	UPointLightComponent* HeatLight = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* FlameMat = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* HeatMat = nullptr;

	// Landing dust ring
	UPROPERTY()
	UStaticMeshComponent* DustRing = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* DustMat = nullptr;
	UPROPERTY()
	UPointLightComponent* ImpactFlash = nullptr;
	float DustAge = 0.f;

	// Cached meshes
	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* ConeMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
};
