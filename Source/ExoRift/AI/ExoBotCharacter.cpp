#include "AI/ExoBotCharacter.h"
#include "Core/ExoAudioManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

AExoBotCharacter::AExoBotCharacter()
{
	// Bots use third-person mesh visible to all
	GetMesh()->SetOwnerNoSee(false);
	// Disable FP arms for bots
	if (FPArms)
	{
		FPArms->SetVisibility(false);
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AExoBotCharacter::Tick(float DeltaTime)
{
	// LOD-based tick rate throttling
	float Interval = 0.f;
	switch (CurrentLODLevel)
	{
	case EAILODLevel::Full: Interval = FullTickInterval; break;
	case EAILODLevel::Simplified: Interval = SimplifiedTickInterval; break;
	case EAILODLevel::Basic: Interval = BasicTickInterval; break;
	}

	if (Interval > 0.f)
	{
		TickAccumulator += DeltaTime;
		if (TickAccumulator < Interval) return;
		TickAccumulator = 0.f;
	}

	// Decrement audio callout cooldowns
	PainCalloutCooldown = FMath::Max(0.f, PainCalloutCooldown - DeltaTime);
	SpottedCalloutCooldown = FMath::Max(0.f, SpottedCalloutCooldown - DeltaTime);
	FireCalloutCooldown = FMath::Max(0.f, FireCalloutCooldown - DeltaTime);

	Super::Tick(DeltaTime);
}

void AExoBotCharacter::SetAILODLevel(EAILODLevel NewLevel)
{
	if (CurrentLODLevel == NewLevel) return;

	EAILODLevel OldLevel = CurrentLODLevel;
	CurrentLODLevel = NewLevel;

	// Adjust movement at lower LODs
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (CMC)
	{
		switch (NewLevel)
		{
		case EAILODLevel::Full:
			CMC->bUseRVOAvoidance = true;
			break;
		case EAILODLevel::Simplified:
			CMC->bUseRVOAvoidance = false;
			break;
		case EAILODLevel::Basic:
			CMC->bUseRVOAvoidance = false;
			break;
		}
	}

	UE_LOG(LogExoRift, Verbose, TEXT("%s LOD: %d -> %d"), *GetName(),
		static_cast<int32>(OldLevel), static_cast<int32>(NewLevel));
}

// ---------------------------------------------------------------------------
// Audio callouts — spatial cues that alert nearby players to bot activity
// ---------------------------------------------------------------------------

float AExoBotCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Bot-specific pain callout — audible to nearby players
	if (ActualDamage > 0.f && IsAlive() && PainCalloutCooldown <= 0.f)
	{
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			// Flesh impact thud + a short vocal-like high-pitch yelp
			Audio->PlayImpactSound(GetActorLocation(), true);
			Audio->PlayWeaponFireSound(nullptr, GetActorLocation(), 0.15f);
		}
		PainCalloutCooldown = 1.5f; // Don't spam pain sounds
	}

	return ActualDamage;
}

void AExoBotCharacter::PlaySpottedCallout()
{
	if (SpottedCalloutCooldown > 0.f) return;

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		// Sharp, high-pitch alert chirp — simulates a "spotted" radio callout
		Audio->PlayAbilityActivateSound(GetActorLocation(), 1.4f);
	}
	SpottedCalloutCooldown = 8.f; // Throttle spotted callouts
}

void AExoBotCharacter::PlayFireCallout()
{
	if (FireCalloutCooldown > 0.f) return;

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		// Brief mechanical click/charge-up before sustained fire
		Audio->PlayWeaponFireSound(nullptr, GetActorLocation(), 0.3f);
	}
	FireCalloutCooldown = 5.f; // Throttle fire callouts
}
