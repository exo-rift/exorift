#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoAbilityComponent.generated.h"

UENUM(BlueprintType)
enum class EExoAbilityType : uint8
{
	Dash,
	AreaScan,
	ShieldBubble
};

USTRUCT(BlueprintType)
struct FExoAbility
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EExoAbilityType Type = EExoAbilityType::Dash;

	UPROPERTY(EditAnywhere)
	float Cooldown = 10.f;

	UPROPERTY()
	float CooldownRemaining = 0.f;

	UPROPERTY(EditAnywhere)
	FString AbilityName;

	bool bIsReady() const { return CooldownRemaining <= 0.f; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityUsed, EExoAbilityType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityCooldownComplete, EExoAbilityType, Type);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoAbilityComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void UseAbility(EExoAbilityType Type);
	const TArray<FExoAbility>& GetAbilities() const { return Abilities; }

	// Scanned enemies from AreaScan
	const TArray<AActor*>& GetScannedEnemies() const { return ScannedEnemies; }

	UPROPERTY(BlueprintAssignable)
	FOnAbilityUsed OnAbilityUsed;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityCooldownComplete OnAbilityCooldownComplete;

protected:
	void ExecuteDash();
	void ExecuteAreaScan();
	void ExecuteShieldBubble();
	void TickAreaScan(float DeltaTime);

	UPROPERTY()
	TArray<FExoAbility> Abilities;

	UPROPERTY()
	TArray<AActor*> ScannedEnemies;

	float ScanTimeRemaining = 0.f;

	static constexpr float DashImpulse = 2000.f;
	static constexpr float ScanRadius = 5000.f;
	static constexpr float ScanDuration = 3.f;
	static constexpr float ShieldRestoreAmount = 50.f;
};
