// ExoCharacterDamage.cpp — TakeDamage, shield break, damage feedback
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoArmorComponent.h"
#include "Player/ExoEmoteComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Core/ExoAudioManager.h"
#include "Core/ExoMusicManager.h"
#include "Core/ExoPlayerState.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoScreenShake.h"
#include "Visual/ExoShieldShatter.h"
#include "UI/ExoHitMarker.h"
#include "UI/ExoHitDirectionIndicator.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

float AExoCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Track last damage source for DBNO bleed-out attribution
	LastDamageInstigator = EventInstigator;
	if (AExoWeaponBase* W = Cast<AExoWeaponBase>(DamageCauser)) LastDamageWeaponName = W->GetWeaponName();
	else LastDamageWeaponName = TEXT("Zone");

	// If already DBNO, damage goes to DBNO health
	if (bIsDBNO)
	{
		DBNOHealthRemaining = FMath::Max(DBNOHealthRemaining - ActualDamage, 0.f);
		if (DBNOHealthRemaining <= 0.f) Die(EventInstigator, LastDamageWeaponName);
		return ActualDamage;
	}

	// Armor vest absorbs body damage, then shield absorbs remainder
	if (ArmorComp) ActualDamage = ArmorComp->AbsorbBodyDamage(ActualDamage);
	bool bHadShield = ShieldComp && ShieldComp->HasShield();
	if (bHadShield) ActualDamage = ShieldComp->AbsorbDamage(ActualDamage);
	bool bShieldBroke = bHadShield && !ShieldComp->HasShield();
	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);

	// Shield break audio + VFX feedback
	if (bShieldBroke)
	{
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
			Audio->PlayShieldBreakSound();
		// 3D shield shatter VFX — hex fragments scatter from hit direction
		FVector HitDir = DamageCauser ? (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal() : FVector::ZeroVector;
		AExoShieldShatter::SpawnShatter(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 50.f), HitDir);
		if (IsLocallyControlled())
		{
			AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
			if (PP) PP->TriggerDamageFlash(0.6f);
			FExoScreenShake::AddShake(0.3f, 0.2f);
		}
	}

	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP) PP->TriggerDamageFlash(FMath::Clamp(DamageAmount / 30.f, 0.2f, 1.f));
		// Screen shake proportional to damage
		FExoScreenShake::AddShake(FMath::Clamp(DamageAmount / 50.f, 0.1f, 0.8f), 0.25f);
		if (DamageCauser && DamageCauser != this)
		{
			float RelativeAngle = (DamageCauser->GetActorLocation() - GetActorLocation()).Rotation().Yaw
				- GetControlRotation().Yaw;
			FExoHitMarker::AddDamageIndicator(RelativeAngle);
			FExoHitDirectionIndicator::AddHit(DamageCauser->GetActorLocation());
		}
	}

	// If executor takes damage, cancel the execution
	if (bIsExecuting) CancelExecution();

	// Cancel any active emote on damage
	if (EmoteComp && EmoteComp->IsEmoting()) EmoteComp->CancelEmote();

	// Track damage taken stat
	if (AController* MyController = GetController())
	{
		if (AExoPlayerState* PS = MyController->GetPlayerState<AExoPlayerState>())
			PS->DamageTaken += ActualDamage;
	}

	// Combat audio: stinger + adaptive music notification (local player only, throttled)
	if (IsLocallyControlled() && DamageCauser && DamageCauser != this)
	{
		// Notify adaptive music system of combat event
		if (UExoMusicManager* Music = UExoMusicManager::Get(GetWorld()))
			Music->NotifyCombatEvent();

		// Play combat stinger (throttled to every 4 seconds)
		if (CombatStingerCooldown <= 0.f)
		{
			if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
				Audio->PlayCombatStinger();
			CombatStingerCooldown = 4.f;
		}
	}

	if (Health <= 0.f) EnterDBNO();

	return ActualDamage;
}
