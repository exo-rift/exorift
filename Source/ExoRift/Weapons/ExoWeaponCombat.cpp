// ExoWeaponCombat.cpp — FireShot, line trace, damage application
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoArmorComponent.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoTracerManager.h"
#include "Visual/ExoWeaponViewModel.h"
#include "Core/ExoAudioManager.h"
#include "Core/ExoPlayerState.h"
#include "UI/ExoPickupNotification.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

void AExoWeaponBase::FireShot()
{
	if (CurrentEnergy < EnergyPerShot) return;
	CurrentEnergy = FMath::Max(CurrentEnergy - EnergyPerShot, 0.f);
	OnEnergyChanged.Broadcast(CurrentEnergy);

	AddHeat(HeatPerShot);
	CurrentSpread = FMath::Min(CurrentSpread + SpreadPerShot, MaxSpread);

	UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld());
	if (Audio)
	{
		Audio->PlayWeaponFireSound(nullptr, GetActorLocation());
	}

	// Recoil
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		AController* PC = OwnerPawn->GetController();
		if (PC)
		{
			float RecoilY = FMath::RandRange(-RecoilYawRange, RecoilYawRange);
			PC->SetControlRotation(PC->GetControlRotation() +
				FRotator(RecoilPitch, RecoilY, 0.f));
		}
	}

	// Track shots
	if (OwnerPawn && OwnerPawn->GetController())
	{
		if (AExoPlayerState* PS = OwnerPawn->GetController()->GetPlayerState<AExoPlayerState>())
			PS->ShotsFired++;
	}

	FHitResult Hit = DoLineTrace(MaxRange);

	// Tracer & muzzle flash
	{
		FVector MuzzleLoc = ViewModel ? ViewModel->GetMuzzleLocation() : GetActorLocation();
		FVector TraceEnd;
		if (Hit.bBlockingHit)
		{
			TraceEnd = Hit.ImpactPoint;
		}
		else
		{
			FVector ViewStart;
			FRotator ViewDir;
			APawn* VP = Cast<APawn>(GetOwner());
			if (VP && VP->GetController())
			{
				VP->GetController()->GetPlayerViewPoint(ViewStart, ViewDir);
				TraceEnd = ViewStart + ViewDir.Vector() * MaxRange;
			}
			else
			{
				TraceEnd = MuzzleLoc + GetActorForwardVector() * MaxRange;
			}
		}

		FExoTracerManager::SpawnTracer(GetWorld(), MuzzleLoc, TraceEnd,
			Hit.bBlockingHit, WeaponType);
		FExoTracerManager::SpawnMuzzleFlash(GetWorld(), MuzzleLoc,
			GetActorRotation(), WeaponType);
	}

	if (!Hit.bBlockingHit) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	AController* InstigatorController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	// Damage with rarity scaling + distance falloff
	float FinalDamage = Damage * GetRarityDamageMultiplier();
	float HitDistance = Hit.Distance;
	if (HitDistance > FalloffStartRange && FalloffEndRange > FalloffStartRange)
	{
		float FalloffAlpha = FMath::Clamp(
			(HitDistance - FalloffStartRange) / (FalloffEndRange - FalloffStartRange),
			0.f, 1.f);
		FinalDamage *= FMath::Lerp(1.f, MinDamageMultiplier, FalloffAlpha);
	}

	// Headshot detection
	bool bHeadshot = false;
	AExoCharacter* HitChar = Cast<AExoCharacter>(HitActor);
	if (HitChar)
	{
		FVector HitLoc = Hit.ImpactPoint;
		FVector HeadLoc = HitChar->GetActorLocation() +
			FVector(0.f, 0.f, HitChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.75f);
		if (FVector::Dist(HitLoc, HeadLoc) < 30.f)
		{
			FinalDamage *= HeadshotMultiplier;
			bHeadshot = true;
			if (HitChar->GetArmorComponent())
				FinalDamage *= HitChar->GetArmorComponent()->GetHeadshotReduction();
		}
	}

	bool bWillKill = HitChar && HitChar->GetHealth() <= FinalDamage;

	FDamageEvent DamageEvent;
	HitActor->TakeDamage(FinalDamage, DamageEvent, InstigatorController, this);

	// Combat stats
	if (HitChar && InstigatorController)
	{
		if (AExoPlayerState* PS = InstigatorController->GetPlayerState<AExoPlayerState>())
		{
			PS->DamageDealt += FMath::RoundToInt32(FinalDamage);
			PS->ShotsHit++;
			if (bWillKill)
			{
				if (HitDistance > PS->LongestKillDistance)
					PS->LongestKillDistance = HitDistance;
				if (bHeadshot)
					PS->HeadshotKills++;
			}
		}
	}

	// Hit marker & kill feedback
	if (HitChar && OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		FExoHitMarker::AddHitMarker(bWillKill, bHeadshot);
		if (bWillKill)
		{
			AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
			if (PP) PP->TriggerKillEffect();

			AExoPlayerState* VictimPS = HitChar->GetController()
				? HitChar->GetController()->GetPlayerState<AExoPlayerState>() : nullptr;
			FString VictimName = VictimPS ? VictimPS->DisplayName : TEXT("Enemy");
			FExoPickupNotification::ShowElimination(VictimName, bHeadshot);
		}
		if (Audio)
		{
			if (bWillKill) Audio->PlayKillSound();
			else Audio->PlayHitMarkerSound(bHeadshot);
		}
	}

	// Damage numbers & impact
	AExoDamageNumbers* DmgNums = AExoDamageNumbers::Get(GetWorld());
	if (DmgNums) DmgNums->SpawnDamageNumber(Hit.ImpactPoint, FinalDamage, bHeadshot);
	if (Audio) Audio->PlayImpactSound(Hit.ImpactPoint, HitChar != nullptr);
	FExoTracerManager::SpawnImpactEffect(GetWorld(), Hit.ImpactPoint,
		Hit.ImpactNormal, HitChar != nullptr);
}

FHitResult AExoWeaponBase::DoLineTrace(float Range) const
{
	FHitResult Hit;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return Hit;

	AController* PC = OwnerPawn->GetController();
	if (!PC) return Hit;

	FVector Start;
	FRotator Direction;
	PC->GetPlayerViewPoint(Start, Direction);

	if (CurrentSpread > 0.f)
	{
		float SpreadRad = FMath::DegreesToRadians(CurrentSpread);
		FVector SpreadDir = FMath::VRandCone(Direction.Vector(), SpreadRad);
		Direction = SpreadDir.Rotation();
	}

	FVector End = Start + Direction.Vector() * Range;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerPawn);
	Params.bTraceComplex = true;

	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	return Hit;
}
