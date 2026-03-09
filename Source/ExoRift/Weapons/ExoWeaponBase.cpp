#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoTracerManager.h"
#include "Core/ExoAudioManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoWeaponBase::AExoWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

float AExoWeaponBase::GetRarityDamageMultiplier() const
{
	switch (Rarity)
	{
	case EWeaponRarity::Common:    return 1.0f;
	case EWeaponRarity::Rare:      return 1.15f;
	case EWeaponRarity::Epic:      return 1.3f;
	case EWeaponRarity::Legendary: return 1.5f;
	default:                       return 1.0f;
	}
}

float AExoWeaponBase::GetRarityHeatMultiplier() const
{
	switch (Rarity)
	{
	case EWeaponRarity::Common:    return 1.0f;
	case EWeaponRarity::Rare:      return 0.95f;
	case EWeaponRarity::Epic:      return 0.85f;
	case EWeaponRarity::Legendary: return 0.75f;
	default:                       return 1.0f;
	}
}

FLinearColor AExoWeaponBase::GetRarityColor(EWeaponRarity InRarity)
{
	switch (InRarity)
	{
	case EWeaponRarity::Common:    return FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
	case EWeaponRarity::Rare:      return FLinearColor(0.2f, 0.5f, 1.0f, 1.f);
	case EWeaponRarity::Epic:      return FLinearColor(0.7f, 0.2f, 1.0f, 1.f);
	case EWeaponRarity::Legendary: return FLinearColor(1.0f, 0.85f, 0.2f, 1.f);
	default:                       return FLinearColor(0.8f, 0.8f, 0.8f, 1.f);
	}
}

void AExoWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceLastShot += DeltaTime;
	TickCooldown(DeltaTime);

	// Spread recovery
	if (!bWantsToFire || bIsOverheated)
	{
		CurrentSpread = FMath::Max(BaseSpread, CurrentSpread - SpreadRecoveryRate * DeltaTime);
	}

	// Auto-fire handling
	if (bWantsToFire && !bIsOverheated)
	{
		float FireInterval = 1.f / FireRate;
		if (TimeSinceLastShot >= FireInterval)
		{
			FireShot();
			TimeSinceLastShot = 0.f;

			if (!bIsAutomatic)
			{
				bWantsToFire = false;
			}
		}
	}
}

void AExoWeaponBase::StartFire()
{
	bWantsToFire = true;
}

void AExoWeaponBase::StopFire()
{
	bWantsToFire = false;
}

void AExoWeaponBase::FireShot()
{
	AddHeat(HeatPerShot);

	// Increase spread
	CurrentSpread = FMath::Min(CurrentSpread + SpreadPerShot, MaxSpread);

	// Fire sound
	UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld());
	if (Audio)
	{
		Audio->PlayWeaponFireSound(nullptr, GetActorLocation());
	}

	// Apply recoil to owner
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

	FHitResult Hit = DoLineTrace(MaxRange);

	// Tracer & muzzle flash
	{
		FVector MuzzleLoc = WeaponMesh ? WeaponMesh->GetSocketLocation(TEXT("Muzzle")) : GetActorLocation();
		FVector TraceEnd;
		if (Hit.bBlockingHit)
		{
			TraceEnd = Hit.ImpactPoint;
		}
		else
		{
			// Use forward direction * range as end point
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

		FExoTracerManager::SpawnTracer(GetWorld(), MuzzleLoc, TraceEnd, Hit.bBlockingHit);
		FExoTracerManager::SpawnMuzzleFlash(GetWorld(), MuzzleLoc, GetActorRotation());
	}

	if (Hit.bBlockingHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			AController* InstigatorController = nullptr;
			if (OwnerPawn)
			{
				InstigatorController = OwnerPawn->GetController();
			}

			// Base damage with rarity scaling
			float FinalDamage = Damage * GetRarityDamageMultiplier();

			// Distance-based damage falloff (applied before headshot)
			float HitDistance = Hit.Distance;
			if (HitDistance > FalloffStartRange && FalloffEndRange > FalloffStartRange)
			{
				float FalloffAlpha = FMath::Clamp(
					(HitDistance - FalloffStartRange) / (FalloffEndRange - FalloffStartRange),
					0.f, 1.f);
				float DamageMult = FMath::Lerp(1.f, MinDamageMultiplier, FalloffAlpha);
				FinalDamage *= DamageMult;
			}

			// Headshot detection
			bool bHeadshot = false;
			AExoCharacter* HitChar = Cast<AExoCharacter>(HitActor);
			if (HitChar)
			{
				FVector HitLoc = Hit.ImpactPoint;
				FVector HeadLoc = HitChar->GetActorLocation() +
					FVector(0.f, 0.f, HitChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.75f);
				float HeadDist = FVector::Dist(HitLoc, HeadLoc);
				if (HeadDist < 30.f)
				{
					FinalDamage *= HeadshotMultiplier;
					bHeadshot = true;
				}
			}

			// Check if this will be a kill
			bool bWillKill = HitChar && HitChar->GetHealth() <= FinalDamage;

			FDamageEvent DamageEvent;
			HitActor->TakeDamage(FinalDamage, DamageEvent, InstigatorController, this);

			// Hit marker feedback (only for the shooter)
			if (HitChar && OwnerPawn && OwnerPawn->IsLocallyControlled())
			{
				FExoHitMarker::AddHitMarker(bWillKill, bHeadshot);

				// Kill post-process flash
				if (bWillKill)
				{
					AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
					if (PP) PP->TriggerKillEffect();
				}

				// Audio feedback
				if (Audio)
				{
					if (bWillKill) Audio->PlayKillSound();
					else Audio->PlayHitMarkerSound(bHeadshot);
				}
			}

			// Spawn damage number
			AExoDamageNumbers* DmgNums = AExoDamageNumbers::Get(GetWorld());
			if (DmgNums)
			{
				DmgNums->SpawnDamageNumber(Hit.ImpactPoint, FinalDamage, bHeadshot);
			}

			// Impact sound
			if (Audio)
			{
				Audio->PlayImpactSound(Hit.ImpactPoint, HitChar != nullptr);
			}
		}
	}
}

void AExoWeaponBase::AddHeat(float Amount)
{
	Amount *= GetRarityHeatMultiplier();
	float OldHeat = CurrentHeat;
	CurrentHeat = FMath::Clamp(CurrentHeat + Amount, 0.f, 1.f);

	if (CurrentHeat >= 1.f && !bIsOverheated)
	{
		bIsOverheated = true;
		bWantsToFire = false;
		OnOverheated.Broadcast();
		UE_LOG(LogExoRift, Verbose, TEXT("%s overheated!"), *WeaponName);
	}

	if (CurrentHeat != OldHeat)
	{
		OnHeatChanged.Broadcast(CurrentHeat);
	}
}

void AExoWeaponBase::TickCooldown(float DeltaTime)
{
	if (CurrentHeat <= 0.f) return;
	if (bWantsToFire && !bIsOverheated) return;

	float Rate = bIsOverheated ? OverheatCooldownRate : CooldownRate;
	float OldHeat = CurrentHeat;
	CurrentHeat = FMath::Max(CurrentHeat - Rate * DeltaTime, 0.f);

	if (bIsOverheated && CurrentHeat <= OverheatRecoveryThreshold)
	{
		bIsOverheated = false;
		OnCooledDown.Broadcast();
	}

	if (CurrentHeat != OldHeat)
	{
		OnHeatChanged.Broadcast(CurrentHeat);
	}
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

	// Apply spread
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
