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

	/** Dash ability — brief FOV punch + radial blur + desaturation. */
	void TriggerDashEffect();

	/** Grapple ability — tunnel vision + green tint while active. 0=off, 1=full. */
	void ApplyGrappleEffect(float Alpha);

	/** Area scan pulse — blue tint outward wave + slight aberration. */
	void TriggerScanPulse();

	/** Shield bubble — brief cyan flash overlay. */
	void TriggerShieldFlash();

	/** Brief bloom + exposure spike when firing weapon. Intensity 0-1 per weapon. */
	void TriggerWeaponFireFlash(float Intensity);

	/** Headshot hit pause — brief timescale dip for satisfying feedback. */
	void TriggerHeadshotPause();

	/** Enhanced headshot kill — stronger chromatic + desaturation + golden flash. */
	void TriggerHeadshotKillEffect();

	/** Energy pickup — brief white-blue bloom flash + slight tint. */
	void TriggerEnergyPickupFlash();

	/** Death cinematic — heavy desat, vignette, chromatic aberration, cold tint. */
	void TriggerDeathEffect();

	/** Cinematic endgame effect — desaturation, vignette, bloom ramp. */
	void TriggerEndgameCinematic(bool bIsVictory);

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

	// Dash effect state (quick burst)
	float DashEffectIntensity = 0.f;

	// Grapple effect state (sustained while grappling)
	float GrappleEffectAlpha = 0.f;
	float TargetGrappleAlpha = 0.f;

	// Scan pulse state
	float ScanPulseIntensity = 0.f;

	// Shield flash state
	float ShieldFlashIntensity = 0.f;

	// Weapon fire bloom kick
	float WeaponFireFlash = 0.f;

	// Headshot hit pause
	float HitPauseRemaining = 0.f;
	float HitPauseOriginalDilation = 1.f;

	// Headshot kill
	float HeadshotKillIntensity = 0.f;

	// Energy pickup flash (white-blue bloom burst)
	float EnergyPickupFlash = 0.f;

	// Death cinematic effect (heavy desat + cold tint + vignette)
	float DeathEffectIntensity = 0.f;

	// Endgame cinematic
	float EndgameBlend = 0.f;
	float EndgameTarget = 0.f;
	bool bEndgameVictory = false;

	UPROPERTY()
	UMaterialInstanceDynamic* VignetteMID;
};
