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

	static AExoPostProcess* Get(UWorld* World);

protected:
	UPROPERTY(VisibleAnywhere)
	UPostProcessComponent* PostProcessComp;

	// Damage flash (red vignette pulse)
	float DamageFlashIntensity = 0.f;
	float DamageFlashDecayRate = 3.f;

	// Low health (desaturation + vignette)
	float LowHealthBlend = 0.f;
	float TargetLowHealthBlend = 0.f;

	// Kill confirm (brief chromatic aberration pulse)
	float KillEffectIntensity = 0.f;
	float KillEffectDecayRate = 4.f;

	UPROPERTY()
	UMaterialInstanceDynamic* VignetteMID;
};
