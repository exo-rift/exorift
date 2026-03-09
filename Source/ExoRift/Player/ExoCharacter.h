#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ExoCharacter.generated.h"

class UCameraComponent;
class AExoWeaponBase;
class UExoShieldComponent;
class UExoInteractionComponent;
class UExoAbilityComponent;
class UExoKillStreakComponent;
class UExoInventoryComponent;

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
	AExoWeaponBase* GetCurrentWeapon() const;

	// Inventory
	UExoInventoryComponent* GetInventoryComponent() const { return InventoryComp; }

	// Health & Shield
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	bool IsAlive() const { return Health > 0.f || bIsDBNO; }
	UExoShieldComponent* GetShieldComponent() const { return ShieldComp; }

	// DBNO (Down But Not Out)
	bool IsDBNO() const { return bIsDBNO; }
	float GetDBNOHealth() const { return DBNOHealthRemaining; }

	// Camera
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	// Interaction
	UExoInteractionComponent* GetInteractionComponent() const { return InteractionComp; }

	// Abilities
	UExoAbilityComponent* GetAbilityComponent() const { return AbilityComp; }

	// Kill Streak
	UExoKillStreakComponent* GetKillStreakComponent() const { return KillStreakComp; }

	// Sprint
	void StartSprint();
	void StopSprint();

	// Sliding
	void StartSlide();
	void StopSlide();
	bool IsSliding() const { return bIsSliding; }

	// Mantling
	void TryMantle();
	bool IsMantling() const { return bIsMantling; }

	// State queries for input gating
	bool IsSprinting() const { return bIsSprinting; }

protected:
	void EnterDBNO();
	void TickDBNO(float DeltaTime);
	void Die(AController* Killer, const FString& WeaponName);

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* FPArms;

	UPROPERTY(VisibleAnywhere, Category = "Shield")
	UExoShieldComponent* ShieldComp;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	UExoInteractionComponent* InteractionComp;

	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	UExoAbilityComponent* AbilityComp;

	UPROPERTY(VisibleAnywhere, Category = "KillStreak")
	UExoKillStreakComponent* KillStreakComp;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UExoInventoryComponent* InventoryComp;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	bool bIsSprinting = false;
	bool bIsDead = false;

	// DBNO state
	bool bIsDBNO = false;

	UPROPERTY(EditDefaultsOnly, Category = "DBNO")
	float DBNOHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "DBNO")
	float DBNOBleedRate = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "DBNO")
	float ReviveTime = 5.f;

	float DBNOHealthRemaining = 0.f;
	float DBNOWalkSpeed = 100.f;

	// Tracks last damage dealer for DBNO bleed-out attribution
	UPROPERTY()
	AController* LastDamageInstigator = nullptr;
	FString LastDamageWeaponName;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeedMultiplier = 1.5f;

	float DefaultWalkSpeed = 600.f;

	// Sliding state
	bool bIsSliding = false;
	float SlideTimer = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlideDuration = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlideStartSpeed = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlideEndSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlideCameraOffset = -40.f;

	float DefaultCapsuleHalfHeight = 96.f;
	float DefaultCameraZ = 64.f;

	void TickSlide(float DeltaTime);

	// Mantling state
	bool bIsMantling = false;
	float MantleTimer = 0.f;
	FVector MantleTarget = FVector::ZeroVector;
	FVector MantleStartLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Mantle")
	float MantleDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Mantle")
	float MantleReach = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Mantle")
	float MantleForwardTrace = 80.f;

	void TickMantle(float DeltaTime);

	// Footstep audio
	float FootstepTimer = 0.f;
	float FootstepInterval = 0.5f;
	void TickFootsteps(float DeltaTime);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
