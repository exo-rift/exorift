#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoHoverVehicle.generated.h"

class UCameraComponent;
class USphereComponent;
class USpringArmComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;
class AExoCharacter;

UCLASS()
class EXORIFT_API AExoHoverVehicle : public APawn, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoHoverVehicle();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

	/** Driver enters this vehicle. */
	void EnterVehicle(AExoCharacter* Driver);

	/** Driver exits this vehicle. */
	void ExitVehicle();

	bool IsOccupied() const { return bIsOccupied; }
	bool IsBoosting() const { return bIsBoosting && CurrentBoostEnergy > 0.f; }
	float GetBoostEnergy() const { return CurrentBoostEnergy; }
	float GetMaxBoostEnergy() const { return BoostEnergy; }
	float GetCurrentSpeed() const { return CurrentSpeed; }

protected:
	// --- Components ---

	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	USpringArmComponent* CameraArm;

	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	UCameraComponent* DriverCamera;

	UPROPERTY(VisibleAnywhere, Category = "Vehicle")
	USphereComponent* InteractionSphere;

	// --- Tuning ---

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Movement")
	float MaxSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Movement")
	float Acceleration = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Movement")
	float TurnRate = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Hover")
	float HoverHeight = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Hover")
	float HoverStiffness = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Hover")
	float HoverDamping = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Boost")
	float BoostMultiplier = 1.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Boost")
	float BoostEnergy = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Boost")
	float BoostDrainRate = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Vehicle|Boost")
	float BoostRechargeRate = 15.f;

private:
	// State
	bool bIsOccupied = false;
	float CurrentSpeed = 0.f;
	float CurrentBoostEnergy = 100.f;
	bool bIsBoosting = false;
	float ThrottleInput = 0.f;
	float SteerInput = 0.f;
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY()
	TWeakObjectPtr<AExoCharacter> DriverCharacter;

	UPROPERTY()
	AController* StoredDriverController = nullptr;

	// Input handlers
	void HandleThrottle(float Value);
	void HandleSteer(float Value);
	void HandleBoostPressed();
	void HandleBoostReleased();
	void HandleExitVehicle();

	// Physics
	void ApplyHoverForce(float DeltaTime);
	void ApplyMovement(float DeltaTime);
	void UpdateVFX(float DeltaTime);

	// VFX
	UPROPERTY()
	UPointLightComponent* EngineGlowL = nullptr;
	UPROPERTY()
	UPointLightComponent* EngineGlowR = nullptr;
	UPROPERTY()
	UStaticMeshComponent* ThrusterL = nullptr;
	UPROPERTY()
	UStaticMeshComponent* ThrusterR = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* ThrusterMat = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* BodyMat = nullptr;

	float CurrentLeanAngle = 0.f;
};
