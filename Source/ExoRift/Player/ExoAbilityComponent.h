#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoAbilityComponent.generated.h"

class AExoDecoyActor;
class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EExoAbilityType : uint8
{
	Dash,
	AreaScan,
	ShieldBubble,
	GrappleHook,
	Decoy
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

	// Grapple state queries
	bool IsGrappling() const { return bIsGrappling; }

	UPROPERTY(BlueprintAssignable)
	FOnAbilityUsed OnAbilityUsed;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityCooldownComplete OnAbilityCooldownComplete;

protected:
	void ExecuteDash();
	void ExecuteAreaScan();
	void ExecuteShieldBubble();
	void ExecuteGrapple();
	void ExecuteDecoy();
	void TickAreaScan(float DeltaTime);
	void TickGrapple(float DeltaTime);

	UPROPERTY()
	TArray<FExoAbility> Abilities;

	UPROPERTY()
	TArray<AActor*> ScannedEnemies;

	float ScanTimeRemaining = 0.f;

	// Grapple state
	bool bIsGrappling = false;
	FVector GrappleTarget = FVector::ZeroVector;
	FVector GrappleStartLocation = FVector::ZeroVector;
	float GrappleTimer = 0.f;

	// Grapple beam VFX
	UPROPERTY()
	UStaticMeshComponent* GrappleBeam = nullptr;
	UPROPERTY()
	UMaterialInstanceDynamic* GrappleBeamMat = nullptr;
	UPROPERTY()
	UPointLightComponent* GrappleLight = nullptr;
	void CreateGrappleBeam();
	void UpdateGrappleBeam();
	void DestroyGrappleBeam();

	static constexpr float DashImpulse = 2000.f;
	static constexpr float ScanRadius = 5000.f;
	static constexpr float ScanDuration = 3.f;
	static constexpr float ShieldRestoreAmount = 50.f;
	static constexpr float GrappleRange = 3000.f;
	static constexpr float GrappleDuration = 0.5f;
	static constexpr float DecoyLifetime = 10.f;
};
