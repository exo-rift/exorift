#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoPostProcess.generated.h"

class UPostProcessComponent;

UCLASS()
class EXORIFT_API AExoPostProcess : public AActor
{
	GENERATED_BODY()

public:
	AExoPostProcess();

	virtual void Tick(float DeltaTime) override;

	// Trigger effects
	void TriggerDamageFlash(float Intensity = 1.f);
	void SetLowHealthEffect(float HealthPercent);
	void TriggerKillEffect();

	/** Red tint + chromatic aberration when taking zone damage. 0=none, 1=max. */
	void ApplyZoneDamageEffect(float Intensity);

	/** Heartbeat-like red pulse when below 25% health. Pass current health 0-1. */
	void ApplyLowHealthPulse(float HealthPercent);

	/** Motion blur + FOV increase during sprint or dash. Alpha 0=off, 1=full. */
	void ApplySpeedBoostEffect(float Alpha);

	static AExoPostProcess* Get(UWorld* World);

	/** Post-process component — public for weather system integration. */
	UPROPERTY(VisibleAnywhere)
	UPostProcessComponent* PostProcessComp;

protected:

	// Damage flash (red vignette pulse)
	float DamageFlashIntensity = 0.f;
	float DamageFlashDecayRate = 3.f;

	// Low health (desaturation + vignette)
	float LowHealthBlend = 0.f;
	float TargetLowHealthBlend = 0.f;

	// Kill confirm (brief chromatic aberration pulse)
	float KillEffectIntensity = 0.f;
	float KillEffectDecayRate = 4.f;

	// Zone damage effect state
	float ZoneDamageIntensity = 0.f;
	float TargetZoneDamageIntensity = 0.f;

	// Low health pulse state (heartbeat at <25%)
	float LowHealthPulseBlend = 0.f;
	float TargetLowHealthPulseBlend = 0.f;

	// Speed boost effect state
	float SpeedBoostAlpha = 0.f;
	float TargetSpeedBoostAlpha = 0.f;

	UPROPERTY()
	UMaterialInstanceDynamic* VignetteMID;
};
