#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ExoCharacter.generated.h"

class UCameraComponent;
class AExoWeaponBase;

UCLASS()
class EXORIFT_API AExoCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AExoCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// Weapon interface
	void StartFire();
	void StopFire();
	void SwapWeapon();
	void EquipWeapon(AExoWeaponBase* Weapon);
	AExoWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	// Health
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	bool IsAlive() const { return Health > 0.f; }

	// Camera
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	// Sprint
	void StartSprint();
	void StopSprint();

protected:
	void Die(AController* Killer, const FString& WeaponName);
	void SpawnDefaultWeapons();

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* FPArms;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(Replicated)
	AExoWeaponBase* CurrentWeapon = nullptr;

	UPROPERTY()
	TArray<AExoWeaponBase*> WeaponInventory;

	int32 CurrentWeaponIndex = 0;
	bool bIsSprinting = false;
	bool bIsDead = false;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeedMultiplier = 1.5f;

	float DefaultWalkSpeed = 600.f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
