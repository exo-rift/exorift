#include "Weapons/ExoWeaponMelee.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoArmorComponent.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "Visual/ExoPostProcess.h"
#include "Core/ExoAudioManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoWeaponMelee::AExoWeaponMelee()
{
	WeaponName = TEXT("Plasma Blade");
	WeaponType = EWeaponType::Melee;
	bIsAutomatic = false;
	FireRate = 2.f;
	Damage = 55.f;
	MaxRange = 200.f;

	// No heat system
	HeatPerShot = 0.f;
	CooldownRate = 999.f;
	OverheatCooldownRate = 999.f;

	// No energy cost
	MaxEnergy = 999.f;
	CurrentEnergy = 999.f;
	EnergyPerShot = 0.f;

	// No spread or recoil
	BaseSpread = 0.f;
	MaxSpread = 0.f;
	SpreadPerShot = 0.f;
	RecoilPitch = 0.f;
	RecoilYawRange = 0.f;

	// Headshot
	HeadshotMultiplier = 1.5f;

	// No falloff (melee range)
	FalloffStartRange = 999999.f;
	FalloffEndRange = 999999.f;
	MinDamageMultiplier = 1.f;
}

void AExoWeaponMelee::FireShot()
{
	// Consume energy / add heat (both effectively free for melee)
	AddHeat(HeatPerShot);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	AController* PC = OwnerPawn->GetController();
	if (!PC) return;

	// Lunge forward
	ApplyLunge();

	// Sphere sweep from the player's view point
	FVector Start;
	FRotator ViewDir;
	PC->GetPlayerViewPoint(Start, ViewDir);
	FVector End = Start + ViewDir.Vector() * SweepRange;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(SweepRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerPawn);

	FHitResult Hit;
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, Start, End, FQuat::Identity, ECC_Visibility, Sphere, Params);

	if (!bHit) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	AController* InstigatorController = PC;
	float FinalDamage = Damage * GetRarityDamageMultiplier();

	// Headshot detection
	bool bHeadshot = false;
	AExoCharacter* HitChar = Cast<AExoCharacter>(HitActor);
	if (HitChar)
	{
		FVector HeadLoc = HitChar->GetActorLocation() +
			FVector(0.f, 0.f, HitChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.75f);
		if (FVector::Dist(Hit.ImpactPoint, HeadLoc) < 30.f)
		{
			FinalDamage *= HeadshotMultiplier;
			bHeadshot = true;

			if (HitChar->GetArmorComponent())
			{
				FinalDamage *= HitChar->GetArmorComponent()->GetHeadshotReduction();
			}
		}
	}

	bool bWillKill = HitChar && HitChar->GetHealth() <= FinalDamage;

	FDamageEvent DamageEvent;
	HitActor->TakeDamage(FinalDamage, DamageEvent, InstigatorController, this);

	// Hit marker & audio feedback
	if (HitChar && OwnerPawn->IsLocallyControlled())
	{
		FExoHitMarker::AddHitMarker(bWillKill, bHeadshot);

		if (bWillKill)
		{
			AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
			if (PP) PP->TriggerKillEffect();
		}

		UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld());
		if (Audio)
		{
			if (bWillKill) Audio->PlayKillSound();
			else Audio->PlayHitMarkerSound(bHeadshot);
		}
	}

	// Damage numbers
	AExoDamageNumbers* DmgNums = AExoDamageNumbers::Get(GetWorld());
	if (DmgNums)
	{
		DmgNums->SpawnDamageNumber(Hit.ImpactPoint, FinalDamage, bHeadshot);
	}
}

void AExoWeaponMelee::ApplyLunge()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	AController* PC = OwnerChar->GetController();
	if (!PC) return;

	FVector ViewStart;
	FRotator ViewDir;
	PC->GetPlayerViewPoint(ViewStart, ViewDir);

	// Horizontal-only lunge (project forward vector onto XY plane)
	FVector Forward = ViewDir.Vector();
	Forward.Z = 0.f;
	Forward.Normalize();

	OwnerChar->LaunchCharacter(Forward * LungeDistance, false, false);
}
