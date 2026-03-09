#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoShieldComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldChanged, float, NewShield, float, MaxShield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldFullyRecharged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoShieldComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// Returns remaining damage that passed through shield
	float AbsorbDamage(float IncomingDamage);

	float GetCurrentShield() const { return CurrentShield; }
	float GetMaxShield() const { return MaxShield; }
	float GetShieldPercent() const { return MaxShield > 0.f ? CurrentShield / MaxShield : 0.f; }
	bool HasShield() const { return CurrentShield > 0.f; }

	void SetShield(float Amount);
	void AddShield(float Amount);

	UPROPERTY(BlueprintAssignable)
	FOnShieldChanged OnShieldChanged;

	UPROPERTY(BlueprintAssignable)
	FOnShieldBroken OnShieldBroken;

	UPROPERTY(BlueprintAssignable)
	FOnShieldFullyRecharged OnShieldFullyRecharged;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	float MaxShield = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	float RechargeRate = 10.f; // Per second

	UPROPERTY(EditDefaultsOnly, Category = "Shield")
	float RechargeDelay = 5.f; // Seconds after damage before recharge starts

	UPROPERTY(Replicated)
	float CurrentShield = 0.f;

	float TimeSinceLastDamage = 0.f;
	bool bWasBroken = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
