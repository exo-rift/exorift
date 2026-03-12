#include "Weapons/ExoWeaponMelee.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoArmorComponent.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoTracerManager.h"
#include "Visual/ExoScreenShake.h"
#include "Visual/ExoMeleeSlash.h"
#include "Core/ExoAudioManager.h"
#include "Core/ExoMusicManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoWeaponMelee::AExoWeaponMelee()
{
	WeaponName = TEXT("Plasma Blade");
	WeaponType = EWeaponType::Melee;
	bIsAutomatic = false;
	FireRate = 2.f;  Damage = 55.f;  MaxRange = 200.f;
	HeatPerShot = 0.f;  CooldownRate = 999.f;  OverheatCooldownRate = 999.f;
	MaxEnergy = 999.f;  CurrentEnergy = 999.f;  EnergyPerShot = 0.f;
	BaseSpread = 0.f;  MaxSpread = 0.f;  SpreadPerShot = 0.f;
	RecoilPitch = 0.f;  RecoilYawRange = 0.f;
	HeadshotMultiplier = 1.5f;
	FalloffStartRange = 999999.f;  FalloffEndRange = 999999.f;  MinDamageMultiplier = 1.f;
}

void AExoWeaponMelee::FireShot()
{
	AddHeat(HeatPerShot);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	AController* PC = OwnerPawn->GetController();
	if (!PC) return;

	ApplyLunge();

	FVector Start; FRotator ViewDir;
	PC->GetPlayerViewPoint(Start, ViewDir);

	// Plasma blade slash arc VFX
	AExoMeleeSlash::SpawnSlash(GetWorld(),
		Start + ViewDir.Vector() * 60.f, ViewDir.Vector());
	FCollisionQueryParams QParams;
	QParams.AddIgnoredActor(this);
	QParams.AddIgnoredActor(OwnerPawn);

	FHitResult Hit;
	bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start,
		Start + ViewDir.Vector() * SweepRange, FQuat::Identity,
		ECC_Visibility, FCollisionShape::MakeSphere(SweepRadius), QParams);

	// Melee swing shake regardless of hit
	FExoScreenShake::AddShake(0.3f, 0.1f);

	if (!bHit || !Hit.GetActor()) return;

	// Impact VFX at contact point
	FExoTracerManager::SpawnImpactEffect(GetWorld(), Hit.ImpactPoint,
		Hit.ImpactNormal, Cast<AExoCharacter>(Hit.GetActor()) != nullptr);

	float FinalDamage = Damage * GetRarityDamageMultiplier();
	bool bHeadshot = false;
	AExoCharacter* HitChar = Cast<AExoCharacter>(Hit.GetActor());
	if (HitChar)
	{
		FVector HeadLoc = HitChar->GetActorLocation() +
			FVector(0.f, 0.f, HitChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.75f);
		if (FVector::Dist(Hit.ImpactPoint, HeadLoc) < 30.f)
		{
			FinalDamage *= HeadshotMultiplier;
			bHeadshot = true;
			if (HitChar->GetArmorComponent())
				FinalDamage *= HitChar->GetArmorComponent()->GetHeadshotReduction();
		}
	}

	bool bWillKill = HitChar && HitChar->GetHealth() <= FinalDamage;
	FDamageEvent DamageEvent;
	Hit.GetActor()->TakeDamage(FinalDamage, DamageEvent, PC, this);

	if (HitChar && OwnerPawn->IsLocallyControlled())
	{
		FExoHitMarker::AddHitMarker(bWillKill, bHeadshot);

		// Notify adaptive music of combat (attacker dealing damage)
		if (auto* Music = UExoMusicManager::Get(GetWorld()))
			Music->NotifyCombatEvent();
		if (bWillKill) { if (auto* PP = AExoPostProcess::Get(GetWorld())) PP->TriggerKillEffect(); }
		if (auto* Audio = UExoAudioManager::Get(GetWorld()))
		{
			if (bWillKill) Audio->PlayKillSound();
			else Audio->PlayHitMarkerSound(bHeadshot);
		}
	}
	if (auto* DmgNums = AExoDamageNumbers::Get(GetWorld()))
		DmgNums->SpawnDamageNumber(Hit.ImpactPoint, FinalDamage, bHeadshot);
}

void AExoWeaponMelee::ApplyLunge()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;
	AController* PC = OwnerChar->GetController();
	if (!PC) return;
	FVector ViewStart; FRotator ViewDir;
	PC->GetPlayerViewPoint(ViewStart, ViewDir);
	FVector Forward = ViewDir.Vector();
	Forward.Z = 0.f;
	if (Forward.Normalize())
		OwnerChar->LaunchCharacter(Forward * LungeDistance, false, false);
}
