#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Core/ExoTypes.h"
#include "ExoBotController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class AExoBotCharacter;
class AExoZoneSystem;

UCLASS()
class EXORIFT_API AExoBotController : public AAIController
{
	GENERATED_BODY()

public:
	AExoBotController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	void UpdateLODLevel();
	void TickFullAI(float DeltaTime);
	void TickSimplifiedAI(float DeltaTime);
	void TickBasicAI(float DeltaTime);

	// Combat
	void FindTarget();
	void AimAtTarget(float DeltaTime);
	void TryFire();
	void StopFiring();

	// Navigation
	void MoveTowardZone();
	void WanderRandomly();
	void MoveToTarget();

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	UPROPERTY()
	AExoZoneSystem* ZoneSystem = nullptr;

	EAILODLevel CurrentLOD = EAILODLevel::Full;

	// LOD update timer
	float LODUpdateInterval = 0.5f;
	float LODUpdateTimer = 0.f;

	// Combat timers
	float TargetSearchInterval = 1.f;
	float TargetSearchTimer = 0.f;

	// Wander
	FVector WanderTarget = FVector::ZeroVector;
	float WanderTimer = 0.f;

	// AI tuning
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float AimAccuracy = 0.7f; // 0-1, affects aim jitter

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float ReactionTime = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float EngageRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SightRadius = 8000.f;
};
