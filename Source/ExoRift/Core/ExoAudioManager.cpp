#include "Core/ExoAudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "ExoRift.h"

void UExoAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load weapon fire sound from template content
	WeaponFireSound = LoadObject<USoundBase>(nullptr,
		TEXT("/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02"));

	UE_LOG(LogExoRift, Log, TEXT("Audio Manager initialized (WeaponFire: %s)"),
		WeaponFireSound ? TEXT("loaded") : TEXT("missing"));
}

void UExoAudioManager::PlayHitMarkerSound(bool bHeadshot)
{
	// TODO: Load proper hit marker sounds
	// For now, uses engine default UI sound at different pitches
	if (WeaponFireSound)
	{
		float Pitch = bHeadshot ? 1.5f : 1.2f;
		UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, Pitch);
	}
}

void UExoAudioManager::PlayKillSound()
{
	if (WeaponFireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 0.6f);
	}
}

void UExoAudioManager::PlayOverheatSound()
{
	// Placeholder — overheat sizzle
}

void UExoAudioManager::PlayShieldBreakSound()
{
	// Placeholder — electric crackle
}

void UExoAudioManager::PlayZoneWarningSound()
{
	// Placeholder — alarm beep
}

void UExoAudioManager::PlayMatchPhaseSound()
{
	// Placeholder — announcement stinger
}

void UExoAudioManager::PlayCountdownTick()
{
	// Placeholder — tick sound
}

void UExoAudioManager::PlayWeaponFireSound(USoundBase* Sound, const FVector& Location, float Volume)
{
	USoundBase* SoundToPlay = Sound ? Sound : WeaponFireSound;
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, Location, Volume);
	}
}

void UExoAudioManager::PlayImpactSound(const FVector& Location, bool bFlesh)
{
	// Placeholder — different sounds for flesh vs metal/stone
}

void UExoAudioManager::PlayExplosionSound(const FVector& Location)
{
	if (WeaponFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 2.f, 0.4f);
	}
}

UExoAudioManager* UExoAudioManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	return World->GetSubsystem<UExoAudioManager>();
}
