#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ExoMusicManager.generated.h"

class UAudioComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EMusicState : uint8
{
	Menu,
	Warmup,
	Combat,
	LowPlayers,
	Victory,
	Defeat
};

/**
 * Adaptive music system. Cross-fades between music layers
 * based on match phase and gameplay intensity.
 */
UCLASS()
class EXORIFT_API UExoMusicManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UExoMusicManager, STATGROUP_Tickables); }

	/** Transition to a new music state with cross-fade. */
	void SetMusicState(EMusicState NewState);

	EMusicState GetMusicState() const { return CurrentState; }

	static UExoMusicManager* Get(UWorld* World);

	// --- Music cues per state (assign in editor or load at runtime) ---

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> MenuMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> WarmupMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> CombatMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> LowPlayersMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> VictoryStinger;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> DefeatStinger;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	float FadeSpeed = 0.5f;

private:
	void StartTrack(USoundBase* Sound, float Volume);
	void StopCurrentTrack();
	USoundBase* GetSoundForState(EMusicState State) const;

	UPROPERTY()
	UAudioComponent* ActiveComp = nullptr;

	EMusicState CurrentState = EMusicState::Menu;
	float CurrentVolume = 0.f;
	float TargetVolume = 0.f;
};
