#include "Core/ExoMusicManager.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

void UExoMusicManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogExoRift, Log, TEXT("MusicManager initialized"));
}

void UExoMusicManager::Deinitialize()
{
	StopCurrentTrack();
	Super::Deinitialize();
}

void UExoMusicManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Decay combat timer
	if (bInCombat)
	{
		CombatTimer -= DeltaTime;
		if (CombatTimer <= 0.f)
		{
			bInCombat = false;
			CombatTimer = 0.f;
			EvaluateMusicState();
		}
	}

	// Cross-fade volume
	if (FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.01f))
	{
		CurrentVolume = TargetVolume;
		return;
	}

	CurrentVolume = FMath::FInterpConstantTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
	if (ActiveComp) ActiveComp->SetVolumeMultiplier(CurrentVolume);

	if (FMath::IsNearlyZero(CurrentVolume) && ActiveComp)
	{
		ActiveComp->Stop();
		ActiveComp->DestroyComponent();
		ActiveComp = nullptr;
	}
}

void UExoMusicManager::SetMusicState(EMusicState NewState)
{
	// For Victory/Defeat, force immediate transition (stingers override everything)
	if (NewState == EMusicState::Victory || NewState == EMusicState::Defeat)
	{
		BaseState = NewState;
		bInCombat = false;
		CombatTimer = 0.f;
		ApplyState(NewState);
		return;
	}

	BaseState = NewState;
	EvaluateMusicState();
}

void UExoMusicManager::NotifyCombatEvent()
{
	bool bWasInCombat = bInCombat;
	bInCombat = true;
	CombatTimer = CombatCooldown;

	if (!bWasInCombat) EvaluateMusicState();
}

void UExoMusicManager::SetZoneShrinking(bool bShrinking)
{
	if (bZoneShrinking == bShrinking) return;
	bZoneShrinking = bShrinking;
	EvaluateMusicState();
}

void UExoMusicManager::NotifyAliveCountChanged(int32 AliveCount, int32 TotalPlayers)
{
	// "Low players" when 25% or fewer remain (min 3 to avoid triggering in tiny matches)
	bool bNowLow = TotalPlayers >= 4 && AliveCount <= FMath::Max(TotalPlayers / 4, 3);
	if (bNowLow != bLowPlayers)
	{
		bLowPlayers = bNowLow;
		EvaluateMusicState();
	}
}

void UExoMusicManager::EvaluateMusicState()
{
	// Priority: Victory/Defeat > Combat > LowPlayers > ZoneShrinking > BaseState
	EMusicState DesiredState = BaseState;

	if (BaseState == EMusicState::Victory || BaseState == EMusicState::Defeat)
	{
		DesiredState = BaseState;
	}
	else if (bInCombat)
	{
		DesiredState = EMusicState::Combat;
	}
	else if (bLowPlayers)
	{
		DesiredState = EMusicState::LowPlayers;
	}
	else if (bZoneShrinking && BaseState != EMusicState::Menu && BaseState != EMusicState::Warmup)
	{
		DesiredState = EMusicState::ZoneShrinking;
	}

	ApplyState(DesiredState);
}

void UExoMusicManager::ApplyState(EMusicState NewState)
{
	if (NewState == CurrentState && ActiveComp && ActiveComp->IsPlaying()) return;
	CurrentState = NewState;

	USoundBase* Sound = GetSoundForState(NewState);
	if (!Sound) { TargetVolume = 0.f; return; }

	float Volume = 1.f;
	switch (NewState)
	{
	case EMusicState::Menu:          Volume = 0.6f; break;
	case EMusicState::Warmup:        Volume = 0.5f; break;
	case EMusicState::Exploration:   Volume = 0.4f; break;
	case EMusicState::Combat:        Volume = 1.0f; break;
	case EMusicState::ZoneShrinking: Volume = 0.85f; break;
	case EMusicState::LowPlayers:    Volume = 0.9f; break;
	case EMusicState::Victory:       Volume = 0.8f; break;
	case EMusicState::Defeat:        Volume = 0.7f; break;
	}

	StopCurrentTrack();
	StartTrack(Sound, Volume);

	UE_LOG(LogExoRift, Log, TEXT("Music state -> %d (vol %.2f)"),
		static_cast<int32>(NewState), Volume);
}

void UExoMusicManager::StartTrack(USoundBase* Sound, float Volume)
{
	if (!Sound || !GetWorld()) return;
	ActiveComp = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, 0.f);
	if (ActiveComp) { CurrentVolume = 0.f; TargetVolume = Volume; }
}

void UExoMusicManager::StopCurrentTrack()
{
	if (ActiveComp)
	{
		ActiveComp->Stop();
		ActiveComp->DestroyComponent();
		ActiveComp = nullptr;
	}
	CurrentVolume = 0.f;
	TargetVolume = 0.f;
}

USoundBase* UExoMusicManager::GetSoundForState(EMusicState State) const
{
	switch (State)
	{
	case EMusicState::Menu:          return MenuMusic.IsValid() ? MenuMusic.Get() : nullptr;
	case EMusicState::Warmup:        return WarmupMusic.IsValid() ? WarmupMusic.Get() : nullptr;
	case EMusicState::Exploration:   return ExplorationMusic.IsValid() ? ExplorationMusic.Get() : nullptr;
	case EMusicState::Combat:        return CombatMusic.IsValid() ? CombatMusic.Get() : nullptr;
	case EMusicState::ZoneShrinking: return ZoneShrinkingMusic.IsValid() ? ZoneShrinkingMusic.Get() : nullptr;
	case EMusicState::LowPlayers:    return LowPlayersMusic.IsValid() ? LowPlayersMusic.Get() : nullptr;
	case EMusicState::Victory:       return VictoryStinger.IsValid() ? VictoryStinger.Get() : nullptr;
	case EMusicState::Defeat:        return DefeatStinger.IsValid() ? DefeatStinger.Get() : nullptr;
	default: return nullptr;
	}
}

UExoMusicManager* UExoMusicManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	return World->GetSubsystem<UExoMusicManager>();
}
