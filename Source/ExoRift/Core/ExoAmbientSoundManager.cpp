#include "Core/ExoAmbientSoundManager.h"
#include "Visual/ExoWeatherSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

void UExoAmbientSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LastWeatherState = EExoWeatherState::Clear;
	UE_LOG(LogExoRift, Log, TEXT("AmbientSoundManager initialized"));
}

void UExoAmbientSoundManager::Deinitialize()
{
	StopAmbientLoop();
	Super::Deinitialize();
}

void UExoAmbientSoundManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FadeSpeed > 0.f)
	{
		CurrentVolume = FMath::FInterpConstantTo(CurrentVolume, TargetVolume, DeltaTime, FadeSpeed);
		if (ActiveAudioComp) ActiveAudioComp->SetVolumeMultiplier(CurrentVolume);

		// When fade-out completes, swap to the pending sound
		if (bFadingOut && FMath::IsNearlyZero(CurrentVolume))
		{
			if (ActiveAudioComp) { ActiveAudioComp->Stop(); ActiveAudioComp->DestroyComponent(); ActiveAudioComp = nullptr; }
			bFadingOut = false;
			if (PendingSound) { PlayAmbientLoop(PendingSound, 1.f); PendingSound = nullptr; }
		}

		if (FMath::IsNearlyEqual(CurrentVolume, TargetVolume, 0.01f)) { CurrentVolume = TargetVolume; FadeSpeed = 0.f; }
	}

	UpdateAmbience();
}

void UExoAmbientSoundManager::PlayAmbientLoop(USoundBase* Sound, float Volume)
{
	if (!Sound || !GetWorld()) return;

	// If already playing something, cross-fade via fade-out first
	if (ActiveAudioComp && ActiveAudioComp->IsPlaying())
	{
		PendingSound = Sound;
		bFadingOut = true;
		TargetVolume = 0.f;
		FadeSpeed = (FadeDuration > 0.f) ? (1.f / FadeDuration) : 100.f;
		return;
	}

	// Spawn a new audio component attached to nothing (2D ambient)
	ActiveAudioComp = UGameplayStatics::SpawnSound2D(GetWorld(), Sound, 0.f);
	if (ActiveAudioComp)
	{
		CurrentVolume = 0.f;
		TargetVolume = Volume;
		FadeSpeed = (FadeDuration > 0.f) ? (Volume / FadeDuration) : 100.f;
	}
}

void UExoAmbientSoundManager::StopAmbientLoop()
{
	PendingSound = nullptr;
	bFadingOut = true;
	TargetVolume = 0.f;
	FadeSpeed = (FadeDuration > 0.f) ? (1.f / FadeDuration) : 100.f;
}

void UExoAmbientSoundManager::UpdateAmbience()
{
	AExoWeatherSystem* Weather = AExoWeatherSystem::Get(GetWorld());
	if (!Weather) return;

	EExoWeatherState Current = Weather->GetCurrentWeather();
	if (Current == LastWeatherState) return;
	LastWeatherState = Current;

	USoundBase* DesiredLoop = nullptr;
	switch (Current)
	{
	case EExoWeatherState::Rain:     DesiredLoop = ResolveSoft(RainLoop);          break;
	case EExoWeatherState::Storm:    DesiredLoop = ResolveSoft(RainLoop);          break;
	case EExoWeatherState::Overcast: DesiredLoop = ResolveSoft(WindLoop);          break;
	case EExoWeatherState::Fog:      DesiredLoop = ResolveSoft(WindLoop);          break;
	case EExoWeatherState::Clear:
	default:                         DesiredLoop = ResolveSoft(ClearAmbienceLoop); break;
	}

	if (DesiredLoop)
	{
		PlayAmbientLoop(DesiredLoop, 1.f);
	}
}

USoundBase* UExoAmbientSoundManager::ResolveSoft(const TSoftObjectPtr<USoundBase>& Soft) const
{
	return Soft.IsValid() ? Soft.Get() : nullptr;
}

UExoAmbientSoundManager* UExoAmbientSoundManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	return World->GetSubsystem<UExoAmbientSoundManager>();
}
