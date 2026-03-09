#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoWeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatChanged, float, NewHeat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOverheated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooledDown);

UCLASS(Abstract)
class EXORIFT_API AExoWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AExoWeaponBase();

	virtual void Tick(float DeltaTime) override;

	virtual void StartFire();
	virtual void StopFire();
	virtual FString GetWeaponName() const { return WeaponName; }

	// Heat accessors
	float GetCurrentHeat() const { return CurrentHeat; }
	bool IsOverheated() const { return bIsOverheated; }
	EWeaponType GetWeaponType() const { return WeaponType; }

	UPROPERTY(BlueprintAssignable)
	FOnHeatChanged OnHeatChanged;

	UPROPERTY(BlueprintAssignable)
	FOnOverheated OnOverheated;

	UPROPERTY(BlueprintAssignable)
	FOnCooledDown OnCooledDown;

protected:
	virtual void FireShot();
	void AddHeat(float Amount);
	void TickCooldown(float DeltaTime);
	FHitResult DoLineTrace(float Range) const;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FString WeaponName = TEXT("Weapon");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Rifle;

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

	// Headshot
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float HeadshotMultiplier = 2.f;

	float CurrentSpread = 0.f;
	float CurrentHeat = 0.f;
	bool bIsOverheated = false;
	bool bWantsToFire = false;
	float TimeSinceLastShot = 0.f;
};
