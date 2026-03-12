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
	Exploration,
	Combat,
	ZoneShrinking,
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

	// --- Combat detection ---

	/** Call when the local player takes or deals damage. Temporarily switches to Combat state. */
	void NotifyCombatEvent();

	/** Call when zone starts/stops shrinking. */
	void SetZoneShrinking(bool bShrinking);

	/** Call when the alive player count changes, to detect low-player tension. */
	void NotifyAliveCountChanged(int32 AliveCount, int32 TotalPlayers);

	// --- Music cues per state (assign in editor or load at runtime) ---

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> MenuMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> WarmupMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> ExplorationMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> CombatMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> ZoneShrinkingMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> LowPlayersMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> VictoryStinger;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	TSoftObjectPtr<USoundBase> DefeatStinger;

	UPROPERTY(EditDefaultsOnly, Category = "Music")
	float FadeSpeed = 0.5f;

	/** How long combat music persists after the last combat event (seconds). */
	UPROPERTY(EditDefaultsOnly, Category = "Music")
	float CombatCooldown = 8.f;

private:
	void StartTrack(USoundBase* Sound, float Volume);
	void StopCurrentTrack();
	USoundBase* GetSoundForState(EMusicState State) const;

	/** Determine the best music state based on gameplay conditions and apply it. */
	void EvaluateMusicState();

	/** Apply a specific music state (cross-fade to it). */
	void ApplyState(EMusicState NewState);

	UPROPERTY()
	UAudioComponent* ActiveComp = nullptr;

	/** The state that was explicitly set (by phase transitions). */
	EMusicState BaseState = EMusicState::Menu;

	/** The currently playing state (may be overridden by combat/zone). */
	EMusicState CurrentState = EMusicState::Menu;

	float CurrentVolume = 0.f;
	float TargetVolume = 0.f;

	// Combat tracking
	float CombatTimer = 0.f;
	bool bInCombat = false;

	// Zone tracking
	bool bZoneShrinking = false;

	// Player count tracking
	bool bLowPlayers = false;
};
