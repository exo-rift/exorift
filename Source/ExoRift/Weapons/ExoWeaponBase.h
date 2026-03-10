#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoWeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatChanged, float, NewHeat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOverheated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooledDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnergyChanged, float, NewEnergy);

class UExoWeaponViewModel;

UCLASS(Abstract)
class EXORIFT_API AExoWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AExoWeaponBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();
	virtual void StopFire();
	virtual FString GetWeaponName() const { return WeaponName; }

	// ADS (Aim Down Sights)
	virtual void StartADS();
	virtual void StopADS();
	virtual void ToggleFireMode();
	void PlayDrawAnimation();
	void StartInspect();
	void StopInspect();
	bool IsInspecting() const { return bIsInspecting; }
	bool IsADS() const { return bIsADS; }
	float GetADSFOV() const { return ADSFOV; }
	float GetADSSpreadMultiplier() const { return ADSSpreadMultiplier; }

	// Rarity system
	UPROPERTY(EditDefaultsOnly, Category = "Rarity")
	EWeaponRarity Rarity = EWeaponRarity::Common;

	float GetRarityDamageMultiplier() const;
	float GetRarityHeatMultiplier() const;
	static FLinearColor GetRarityColor(EWeaponRarity InRarity);

	// Heat accessors
	float GetCurrentHeat() const { return CurrentHeat; }
	bool IsOverheated() const { return bIsOverheated; }
	void ResetHeat();
	EWeaponType GetWeaponType() const { return WeaponType; }

	// Energy accessors
	float GetCurrentEnergy() const { return CurrentEnergy; }
	float GetMaxEnergy() const { return MaxEnergy; }
	float GetEnergyPercent() const { return MaxEnergy > 0.f ? CurrentEnergy / MaxEnergy : 0.f; }
	void AddEnergy(float Amount);

	UPROPERTY(BlueprintAssignable)
	FOnHeatChanged OnHeatChanged;

	UPROPERTY(BlueprintAssignable)
	FOnOverheated OnOverheated;

	UPROPERTY(BlueprintAssignable)
	FOnCooledDown OnCooledDown;

	UPROPERTY(BlueprintAssignable)
	FOnEnergyChanged OnEnergyChanged;

protected:
	virtual void FireShot();
	void AddHeat(float Amount);
	void TickCooldown(float DeltaTime);
	FHitResult DoLineTrace(float Range) const;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY()
	UExoWeaponViewModel* ViewModel;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString WeaponName = TEXT("Weapon");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Rifle;

	// Energy cell system
	UPROPERTY(EditDefaultsOnly, Category = "Energy")
	float MaxEnergy = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Energy")
	float EnergyPerShot = 1.f;

	float CurrentEnergy = 100.f;

	// Overheat system
	UPROPERTY(EditDefaultsOnly, Category = "Heat")
	float HeatPerShot = 0.04f;

	UPROPERTY(EditDefaultsOnly, Category = "Heat")
	float CooldownRate = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Heat")
	float OverheatCooldownRate = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Heat")
	float OverheatRecoveryThreshold = 0.3f;

	// Fire mechanics
	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float FireRate = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float Damage = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	float MaxRange = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Fire")
	bool bIsAutomatic = true;

	// Spread & recoil (degrees)
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float BaseSpread = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float MaxSpread = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadPerShot = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadRecoveryRate = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoilPitch = -0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoilYawRange = 0.15f;

	// Distance-based damage falloff
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float FalloffStartRange = 20000.f; // Full damage up to this distance

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float FalloffEndRange = 50000.f; // Minimum damage beyond this distance

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float MinDamageMultiplier = 0.3f; // Multiplier at max falloff range

	// Headshot
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float HeadshotMultiplier = 2.f;

	// ADS state
	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSFOV = 65.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSSpreadMultiplier = 0.5f;

	bool bIsADS = false;

	float CurrentSpread = 0.f;
	float CurrentHeat = 0.f;
	bool bIsOverheated = false;
	bool bWantsToFire = false;
	float TimeSinceLastShot = 0.f;

	// Weapon sway
	void TickWeaponSway(float DeltaTime);
	FRotator PrevControlRotation;
	FVector SwayOffset = FVector::ZeroVector;
	float SwayReturnSpeed = 8.f;
	float SwayAmount = 0.4f;
	float MaxSwayOffset = 3.f;

	// Equip/draw animation
	float DrawBlend = 0.f;
	float DrawSpeed = 6.f;
	bool bDrawAnimActive = false;

	// ADS blend
	float ADSBlend = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "ADS")
	float ADSBlendSpeed = 10.f;

	// Visual recoil kick
	void ApplyRecoilKick();
	FVector RecoilOffset = FVector::ZeroVector;
	float RecoilRotation = 0.f;
	float RecoilRecoverySpeed = 15.f;

	// Movement bob (weapon bobs when walking/sprinting)
	float MoveBobTimer = 0.f;
	FVector MoveBobOffset = FVector::ZeroVector;

	// Weapon inspect animation
	bool bIsInspecting = false;
	float InspectBlend = 0.f;
	float InspectPhase = 0.f;

	// Heat glow on barrel
	void TickHeatGlow();
};
