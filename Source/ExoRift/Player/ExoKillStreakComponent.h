#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoKillStreakComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStreakChanged, int32, NewStreak, const FString&, StreakName);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EXORIFT_API UExoKillStreakComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoKillStreakComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Call when the owning player scores a kill. */
	void RegisterKill();

	int32 GetCurrentStreak() const { return CurrentStreak; }
	float GetDamageMultiplier() const { return DamageMultiplier; }
	float GetSpeedMultiplier() const { return SpeedMultiplier; }
	FString GetStreakName() const;

	UPROPERTY(BlueprintAssignable, Category = "KillStreak")
	FOnStreakChanged OnStreakChanged;

protected:
	void UpdateStreakBonuses();
	void ResetStreak();
	void ApplySpeedBonus();
	void RevertSpeedBonus();

	/** Time window between kills to maintain a streak. */
	UPROPERTY(EditDefaultsOnly, Category = "KillStreak")
	float StreakTimeWindow = 10.f;

private:
	int32 CurrentStreak = 0;
	float StreakTimer = 0.f;
	bool bStreakActive = false;

	float DamageMultiplier = 1.f;
	float SpeedMultiplier = 1.f;

	/** Cached so we can revert speed changes. */
	float BaseWalkSpeed = 0.f;
	bool bSpeedApplied = false;
};
