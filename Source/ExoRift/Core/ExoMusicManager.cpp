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

	if (FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.01f)) { CurrentVolume = TargetVolume; return; }

	CurrentVolume = FMath::FInterpConstantTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
	if (ActiveComp) ActiveComp->SetVolumeMultiplier(CurrentVolume);

	if (FMath::IsNearlyZero(CurrentVolume) && ActiveComp)
	{
		ActiveComp->Stop(); ActiveComp->DestroyComponent(); ActiveComp = nullptr;
	}
}

void UExoMusicManager::SetMusicState(EMusicState NewState)
{
	if (NewState == CurrentState && ActiveComp && ActiveComp->IsPlaying()) return;
	CurrentState = NewState;

	USoundBase* Sound = GetSoundForState(NewState);
	if (!Sound) { TargetVolume = 0.f; return; }

	float Volume = 1.f;
	switch (NewState)
	{
	case EMusicState::Combat:    Volume = 1.0f; break;
	case EMusicState::LowPlayers: Volume = 0.9f; break;
	case EMusicState::Menu:      Volume = 0.6f; break;
	case EMusicState::Warmup:    Volume = 0.5f; break;
	case EMusicState::Victory:   Volume = 0.8f; break;
	case EMusicState::Defeat:    Volume = 0.7f; break;
	}

	StopCurrentTrack();
	StartTrack(Sound, Volume);
}

void UExoMusicManager::StartTrack(USoundBase* Sound, float Volume)
{
	if (!Sound || !GetWorld()) return;
	ActiveComp = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, 0.f);
	if (ActiveComp) { CurrentVolume = 0.f; TargetVolume = Volume; }
}

void UExoMusicManager::StopCurrentTrack()
{
	if (ActiveComp) { ActiveComp->Stop(); ActiveComp->DestroyComponent(); ActiveComp = nullptr; }
	CurrentVolume = 0.f;
	TargetVolume = 0.f;
}

USoundBase* UExoMusicManager::GetSoundForState(EMusicState State) const
{
	switch (State)
	{
	case EMusicState::Menu:       return MenuMusic.IsValid() ? MenuMusic.Get() : nullptr;
	case EMusicState::Warmup:     return WarmupMusic.IsValid() ? WarmupMusic.Get() : nullptr;
	case EMusicState::Combat:     return CombatMusic.IsValid() ? CombatMusic.Get() : nullptr;
	case EMusicState::LowPlayers: return LowPlayersMusic.IsValid() ? LowPlayersMusic.Get() : nullptr;
	case EMusicState::Victory:    return VictoryStinger.IsValid() ? VictoryStinger.Get() : nullptr;
	case EMusicState::Defeat:     return DefeatStinger.IsValid() ? DefeatStinger.Get() : nullptr;
	default: return nullptr;
	}
}

UExoMusicManager* UExoMusicManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	return World->GetSubsystem<UExoMusicManager>();
}
