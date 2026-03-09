#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ExoAmbientSoundManager.generated.h"

class UAudioComponent;
class USoundBase;
enum class EExoWeatherState : uint8;

/**
 * Manages ambient background audio loops. Tick-driven, cross-fades
 * between ambient layers based on the current weather state.
 */
UCLASS()
class EXORIFT_API UExoAmbientSoundManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UExoAmbientSoundManager, STATGROUP_Tickables); }

	/** Start playing an ambient loop. Fades in over FadeDuration. */
	void PlayAmbientLoop(USoundBase* Sound, float Volume = 1.f);

	/** Stop the current ambient loop, fading out over FadeDuration. */
	void StopAmbientLoop();

	static UExoAmbientSoundManager* Get(UWorld* World);

	// --- Soft asset paths (set in editor or loaded at init) ---

	UPROPERTY(EditDefaultsOnly, Category = "Ambient|Sounds")
	TSoftObjectPtr<USoundBase> WindLoop;

	UPROPERTY(EditDefaultsOnly, Category = "Ambient|Sounds")
	TSoftObjectPtr<USoundBase> RainLoop;

	UPROPERTY(EditDefaultsOnly, Category = "Ambient|Sounds")
	TSoftObjectPtr<USoundBase> ThunderStinger;

	UPROPERTY(EditDefaultsOnly, Category = "Ambient|Sounds")
	TSoftObjectPtr<USoundBase> ClearAmbienceLoop;

	UPROPERTY(EditDefaultsOnly, Category = "Ambient")
	float FadeDuration = 3.f;

protected:
	/** Checks weather and picks the right ambient loop. */
	void UpdateAmbience();

	/** Resolves a soft path, returning nullptr if not yet loaded. */
	USoundBase* ResolveSoft(const TSoftObjectPtr<USoundBase>& Soft) const;

private:
	UPROPERTY()
	UAudioComponent* ActiveAudioComp = nullptr;

	UPROPERTY()
	USoundBase* PendingSound = nullptr;

	float CurrentVolume = 0.f;
	float TargetVolume = 0.f;
	float FadeSpeed = 0.f;

	bool bFadingOut = false;

	EExoWeatherState LastWeatherState;
};
