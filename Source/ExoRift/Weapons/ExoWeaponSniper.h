#pragma once

#include "CoreMinimal.h"
#include "Weapons/ExoWeaponBase.h"
#include "ExoWeaponSniper.generated.h"

class UPointLightComponent;

UCLASS()
class EXORIFT_API AExoWeaponSniper : public AExoWeaponBase
{
	GENERATED_BODY()

public:
	AExoWeaponSniper();

	virtual void Tick(float DeltaTime) override;
	virtual void StartADS() override;
	virtual void StopADS() override;
	virtual float GetBreathHoldFactor() const override;

	bool IsScoped() const { return bIsScoped; }
	float GetScopeHoldProgress() const;

protected:
	bool bIsScoped = false;

	// Scope glint — visible flash to other players when scoped
	UPROPERTY()
	UStaticMeshComponent* GlintMesh;

	UPROPERTY()
	UPointLightComponent* GlintLight;

	float GlintBlend = 0.f;

	// Hold-breath mechanic: reduces sway to zero for a limited time
	float HoldBreathTimer = 0.f;
	float MaxHoldBreath = 4.f;
	float HoldBreathRecovery = 2.f; // Seconds to fully recover
	bool bIsHoldingBreath = true;
};
