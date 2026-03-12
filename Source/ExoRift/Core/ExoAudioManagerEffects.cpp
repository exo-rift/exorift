// Copyright Spot Cloud b.v. (2026). All Rights Reserved.

#include "Core/ExoAudioManager.h"
#include "Kismet/GameplayStatics.h"

// ---------------------------------------------------------------------------
// Specialized effect sounds — shields, doors, vehicles, stingers, slides, traps
// Core audio functions live in ExoAudioManager.cpp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Shield effects
// ---------------------------------------------------------------------------

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

void UExoAudioManager::PlayShieldRechargeSound()
{
	if (!WeaponFireSound) return;

	// Ascending shimmer — shield restored confirmation
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, 1.2f);
	FTimerHandle Handle1, Handle2;
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 1.8f);
	}, 0.06f, false);
	GetWorld()->GetTimerManager().SetTimer(Handle2, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.12f, 2.4f);
	}, 0.14f, false);
}

// ---------------------------------------------------------------------------
// Combat stingers
// ---------------------------------------------------------------------------

void UExoAudioManager::PlayDeathStinger()
{
	if (!WeaponFireSound) return;

	// Dramatic death stinger: deep bass hit + dissonant mid + fading high
	// Initial heavy impact
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.5f, 0.25f);

	FTimerHandle Handle1, Handle2, Handle3, Handle4;
	// Dissonant mid tone
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.35f, 0.45f);
	}, 0.06f, false);
	// Eerie high whine
	GetWorld()->GetTimerManager().SetTimer(Handle2, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 1.8f);
	}, 0.12f, false);
	// Second bass rumble (echo)
	GetWorld()->GetTimerManager().SetTimer(Handle3, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.3f, 0.3f);
	}, 0.25f, false);
	// Final fading tone
	GetWorld()->GetTimerManager().SetTimer(Handle4, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, 0.6f);
	}, 0.5f, false);
}

void UExoAudioManager::PlayCombatStinger()
{
	if (!WeaponFireSound) return;
	// Tense low rumble + sharp attack
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.15f, 0.35f);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.1f, 0.6f);
	}, 0.1f, false);
}

// ---------------------------------------------------------------------------
// Door sounds
// ---------------------------------------------------------------------------

void UExoAudioManager::PlayDoorSlideSound(const FVector& Location, bool bOpening)
{
	if (!WeaponFireSound) return;

	if (bOpening)
	{
		// Pneumatic hiss + mechanical slide (ascending)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.3f, 2.2f);
		FTimerHandle Handle1, Handle2;
		GetWorld()->GetTimerManager().SetTimer(Handle1, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.2f, 1.6f);
		}, 0.04f, false);
		GetWorld()->GetTimerManager().SetTimer(Handle2, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.15f, 0.8f);
		}, 0.12f, false);
	}
	else
	{
		// Reverse slide + pneumatic clamp (descending)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.2f, 1.4f);
		FTimerHandle Handle1, Handle2;
		GetWorld()->GetTimerManager().SetTimer(Handle1, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.25f, 0.6f);
		}, 0.06f, false);
		GetWorld()->GetTimerManager().SetTimer(Handle2, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.35f, 0.35f);
		}, 0.15f, false);
	}
}

// ---------------------------------------------------------------------------
// Vehicle sounds
// ---------------------------------------------------------------------------

void UExoAudioManager::PlayVehicleEngineSound(const FVector& Location, bool bStartup)
{
	if (!WeaponFireSound) return;

	if (bStartup)
	{
		// Hover engine startup — low rumble building to whine
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.5f, 0.25f);
		FTimerHandle Handle1, Handle2, Handle3;
		GetWorld()->GetTimerManager().SetTimer(Handle1, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.4f, 0.5f);
		}, 0.08f, false);
		GetWorld()->GetTimerManager().SetTimer(Handle2, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.3f, 1.0f);
		}, 0.18f, false);
		GetWorld()->GetTimerManager().SetTimer(Handle3, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.2f, 1.8f);
		}, 0.3f, false);
	}
	else
	{
		// Shutdown — whine descending to low rumble
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.3f, 1.5f);
		FTimerHandle Handle1, Handle2;
		GetWorld()->GetTimerManager().SetTimer(Handle1, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.4f, 0.7f);
		}, 0.1f, false);
		GetWorld()->GetTimerManager().SetTimer(Handle2, [this, Location]()
		{
			if (WeaponFireSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.5f, 0.3f);
		}, 0.22f, false);
	}
}

// ---------------------------------------------------------------------------
// Trap sounds
// ---------------------------------------------------------------------------

void UExoAudioManager::PlayTrapPlacedSound()
{
	if (!WeaponFireSound) return;

	// Confirmation chirp — short ascending double-beep (sci-fi deploy confirm)
	UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.2f, 1.6f);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySound2D(GetWorld(), WeaponFireSound, 0.25f, 2.0f);
	}, 0.08f, false);
}

void UExoAudioManager::PlayTrapActivationBeep(const FVector& Location)
{
	if (!WeaponFireSound) return;

	// Rapid warning beeps — three escalating high-pitched pings at mine location
	for (int32 i = 0; i < 3; ++i)
	{
		float Delay = i * 0.12f;
		float Pitch = 1.8f + i * 0.3f;
		float Volume = 0.4f + i * 0.1f;
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle,
			[this, Location, Volume, Pitch]()
			{
				if (WeaponFireSound)
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound,
						Location, Volume, Pitch);
			}, Delay, false);
	}
}

// ---------------------------------------------------------------------------
// Movement / slide sounds
// ---------------------------------------------------------------------------

void UExoAudioManager::PlaySlideStartSound(const FVector& Location)
{
	if (!WeaponFireSound) return;

	// Sci-fi suit scrape — low whoosh + sharp onset
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.35f, 0.4f);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, Location]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.2f, 1.6f);
	}, 0.03f, false);
}

void UExoAudioManager::PlaySlideLoopSound(const FVector& Location)
{
	if (!WeaponFireSound) return;

	// Continuous scrape tick — low rumble with slight variation
	float Pitch = 0.35f + FMath::RandRange(-0.05f, 0.05f);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.12f, Pitch);
}

void UExoAudioManager::PlaySlideStopSound(const FVector& Location)
{
	if (!WeaponFireSound) return;

	// Braking scrape — short ascending screech
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.25f, 0.6f);
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, Location]()
	{
		if (WeaponFireSound)
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location, 0.15f, 1.0f);
	}, 0.04f, false);
}

void UExoAudioManager::PlayWindSound(const FVector& Location, float Intensity)
{
	if (!WeaponFireSound) return;

	// Rushing wind — layered low-pitch whoosh
	float Vol = FMath::Clamp(Intensity * 0.15f, 0.05f, 0.2f);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponFireSound, Location,
		Vol, FMath::RandRange(0.15f, 0.25f));
}
