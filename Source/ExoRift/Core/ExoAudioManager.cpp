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

// ---------------------------------------------------------------------------
// UI / Feedback sounds (2D)
// ---------------------------------------------------------------------------

void UExoAudioManager::PlayHitMarkerSound(bool bHeadshot)
{
	if (!WeaponFireSound) return;

	if (bHeadshot)
	{
		// High metallic ping for headshot
		UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 1.8f);
		// Double-tap for emphasis
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.12f, 2.2f);
		}, 0.05f, false);
	}
	else
	{
		// Short tick sound
		UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.12f, 1.5f);
	}
}

void UExoAudioManager::PlayKillSound()
{
	if (!WeaponFireSound) return;

	// Deep satisfying thump + high confirm
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.3f, 0.5f);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, 1.6f);
	}, 0.08f, false);
}

void UExoAudioManager::PlayOverheatSound()
{
	if (!WeaponFireSound) return;

	// Rising pitch sizzle — play multiple rapid shots at increasing pitch
	for (int32 i = 0; i < 3; ++i)
	{
		float Delay = i * 0.06f;
		float Pitch = 0.4f + i * 0.15f;
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this, Pitch]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.08f, Pitch);
		}, Delay, false);
	}
}

void UExoAudioManager::PlayShieldBreakSound()
{
	if (!WeaponFireSound) return;

	// Descending crackle — high to low pitch
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.25f, 2.0f);
	FTimerHandle Handle1, Handle2;
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 1.4f);
	}, 0.04f, false);
	GetWorld()->GetTimerManager().SetTimer(Handle2, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, 0.8f);
	}, 0.1f, false);
}

void UExoAudioManager::PlayZoneWarningSound()
{
	if (!WeaponFireSound) return;

	// Three alert beeps
	for (int32 i = 0; i < 3; ++i)
	{
		float Delay = i * 0.3f;
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 1.8f);
		}, Delay, false);
	}
}

void UExoAudioManager::PlayMatchPhaseSound()
{
	if (!WeaponFireSound) return;

	// Two-tone announcement stinger
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.35f, 1.2f);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.35f, 1.5f);
	}, 0.15f, false);
}

void UExoAudioManager::PlayCountdownTick()
{
	if (!WeaponFireSound) return;
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, 2.0f);
}

void UExoAudioManager::PlayVictoryStinger()
{
	if (!WeaponFireSound) return;

	// Ascending triumph fanfare
	float Pitches[] = { 1.0f, 1.25f, 1.5f, 2.0f };
	for (int32 i = 0; i < 4; ++i)
	{
		float Delay = i * 0.12f;
		float Pitch = Pitches[i];
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this, Pitch]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.3f, Pitch);
		}, Delay, false);
	}
}

void UExoAudioManager::PlayDefeatStinger()
{
	if (!WeaponFireSound) return;

	// Descending somber tone
	float Pitches[] = { 1.0f, 0.8f, 0.6f, 0.4f };
	for (int32 i = 0; i < 4; ++i)
	{
		float Delay = i * 0.15f;
		float Pitch = Pitches[i];
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this, Pitch]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.25f, Pitch);
		}, Delay, false);
	}
}

// ---------------------------------------------------------------------------
// Spatial sounds
// ---------------------------------------------------------------------------

void UExoAudioManager::PlayFootstepSound(const FVector& Location, bool bIsSprinting)
{
	if (!WeaponFireSound) return;

	float Volume = bIsSprinting ? 0.25f : 0.12f;
	// Very low pitch for thuddy footstep feel
	float Pitch = bIsSprinting ? 0.3f : 0.35f;
	// Add slight random variation
	Pitch += FMath::RandRange(-0.03f, 0.03f);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, Volume, Pitch);
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
	if (!WeaponFireSound) return;

	if (bFlesh)
	{
		// Dull thud for flesh
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.3f, 0.5f);
	}
	else
	{
		// Metallic ping for surfaces
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.2f, 1.8f);
	}
}

void UExoAudioManager::PlayExplosionSound(const FVector& Location)
{
	if (!WeaponFireSound) return;

	// Multi-layered explosion: bass boom + mid crackle + high debris
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 2.0f, 0.3f);

	FTimerHandle Handle1, Handle2;
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this, Location]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 1.0f, 0.7f);
	}, 0.05f, false);
	GetWorld()->GetTimerManager().SetTimer(Handle2, [this, Location]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.4f, 1.5f);
	}, 0.12f, false);
}

UExoAudioManager* UExoAudioManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	return World->GetSubsystem<UExoAudioManager>();
}
