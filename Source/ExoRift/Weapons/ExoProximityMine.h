#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoProximityMine.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AExoCharacter;

UENUM(BlueprintType)
enum class EProximityMineState : uint8
{
	Deploying,
	Armed,
	Triggered,
	Detonated
};

/**
 * Player-deployable proximity mine.
 * Arms after 2s delay, triggers on enemy proximity, detonates after 0.5s warning.
 */
UCLASS()
class EXORIFT_API AExoProximityMine : public AActor
{
	GENERATED_BODY()

public:
	AExoProximityMine();

	/** The player who placed this mine (ignored by trigger). */
	UPROPERTY()
	AExoCharacter* OwnerCharacter = nullptr;

	EProximityMineState GetMineState() const { return MineState; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	void Arm();
	void TriggerMine();
	void Detonate();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MineMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* TriggerSphere;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float ArmDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float TriggerDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float ExplosionDamage = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float ExplosionRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float TriggerRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mine")
	float MaxLifetime = 60.f;

	EProximityMineState MineState = EProximityMineState::Deploying;
	float StateTimer = 0.f;
	float LifetimeTimer = 0.f;
	bool bPulseOn = false;
	float PulseTimer = 0.f;
};
