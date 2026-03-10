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
class AExoCharacter;
class AExoWeaponPickup;
class AExoDecoyActor;

/**
 * Enhanced AI controller with difficulty scaling, strafing, cover-seeking,
 * and smart target prioritization.
 */
UCLASS()
class EXORIFT_API AExoBotController : public AAIController
{
	GENERATED_BODY()

public:
	AExoBotController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	void SetDifficulty(EBotDifficulty NewDifficulty);

protected:
	// LOD tiers
	void UpdateLODLevel();
	void TickFullAI(float DeltaTime);
	void TickSimplifiedAI(float DeltaTime);
	void TickBasicAI(float DeltaTime);

	// Combat
	void FindTarget();
	void AimAtTarget(float DeltaTime);
	void TryFire();
	void StopFiring();
	void UpdateStrafeDirection(float DeltaTime);
	void ApplyStrafe(float DeltaTime);
	bool HasLineOfSight(AActor* Target) const;
	bool ShouldSeekCover() const;
	void SeekCover();

	// Navigation
	void MoveTowardZone();
	void WanderRandomly();
	void MoveToTarget();
	void LookForLoot();

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// Components
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	UAISenseConfig_Hearing* HearingConfig;

	// State
	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	UPROPERTY()
	AExoZoneSystem* ZoneSystem = nullptr;

	UPROPERTY()
	AExoWeaponPickup* LootTarget = nullptr;

	EAILODLevel CurrentLOD = EAILODLevel::Full;
	EBotDifficulty Difficulty = EBotDifficulty::Medium;

	// Timers
	float LODUpdateTimer = 0.f;
	float TargetSearchTimer = 0.f;
	float WanderTimer = 0.f;
	float StrafeTimer = 0.f;
	float LootSearchTimer = 0.f;
	float ReactionTimer = 0.f; // Delay before first shot on new target
	bool bReactionPending = false;

	// Strafe state
	float StrafeDirection = 1.f; // +1 right, -1 left
	float StrafeChangeInterval = 1.5f;

	// Cover seeking
	FVector CoverLocation = FVector::ZeroVector;
	bool bSeekingCover = false;

	// Grenade usage
	float GrenadeTimer = 0.f;
	float GrenadeCooldown = 15.f;
	void TryThrowGrenade();

	// DBNO execution
	void TryExecuteDBNO();
	float ExecutionSearchTimer = 0.f;

	// Ability usage
	void TryUseAbilities(float DeltaTime);
	float AbilityCheckTimer = 0.f;
	float DashCooldownBot = 0.f;
	float ScanCooldownBot = 0.f;
	float ShieldCooldownBot = 0.f;
	float DecoyCooldownBot = 0.f;

	// Difficulty-derived stats (set by SetDifficulty)
	float AimAccuracy = 0.7f;
	float ReactionTime = 0.3f;
	float StrafeChance = 0.5f;
	float CoverHealthThreshold = 40.f;
	float BurstDuration = 0.f;   // How long to fire before pausing
	float BurstPause = 0.f;      // Pause between bursts
	float BurstTimer = 0.f;
	bool bInBurst = true;

	// Configuration
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float EngageRange = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SightRadius = 8000.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float LODUpdateInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float TargetSearchInterval = 1.f;

	FVector WanderTarget = FVector::ZeroVector;
};
