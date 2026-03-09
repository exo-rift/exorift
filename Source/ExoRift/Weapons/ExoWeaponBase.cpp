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

void AExoWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// Build procedural weapon view model
	ViewModel = NewObject<UExoWeaponViewModel>(this);
	ViewModel->SetupAttachment(RootComponent);
	ViewModel->SetRelativeLocation(FVector(20.f, 10.f, -8.f)); // Offset to right of camera
	ViewModel->RegisterComponent();
	ViewModel->BuildModel(WeaponType, GetRarityColor(Rarity));
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

	TickWeaponSway(DeltaTime);

	// Auto-fire handling
	if (bWantsToFire && !bIsOverheated && CurrentEnergy >= EnergyPerShot)
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
	if (CurrentEnergy < EnergyPerShot) return;
	CurrentEnergy = FMath::Max(CurrentEnergy - EnergyPerShot, 0.f);
	OnEnergyChanged.Broadcast(CurrentEnergy);

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

	// Track shots fired
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

					// Helmet reduces headshot bonus damage
					if (HitChar->GetArmorComponent())
					{
						FinalDamage *= HitChar->GetArmorComponent()->GetHeadshotReduction();
					}
				}
			}

			// Check if this will be a kill
			bool bWillKill = HitChar && HitChar->GetHealth() <= FinalDamage;

			FDamageEvent DamageEvent;
			HitActor->TakeDamage(FinalDamage, DamageEvent, InstigatorController, this);

			// Track combat stats on instigator
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

			// Hit marker feedback (only for the shooter)
			if (HitChar && OwnerPawn && OwnerPawn->IsLocallyControlled())
			{
				FExoHitMarker::AddHitMarker(bWillKill, bHeadshot);

				// Kill effects
				if (bWillKill)
				{
					AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
					if (PP) PP->TriggerKillEffect();

					// Elimination notification
					AExoPlayerState* VictimPS = HitChar->GetController()
						? HitChar->GetController()->GetPlayerState<AExoPlayerState>() : nullptr;
					FString VictimName = VictimPS ? VictimPS->DisplayName : TEXT("Enemy");
					FExoPickupNotification::ShowElimination(VictimName, bHeadshot);
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

			// Impact sparks at hit location
			FExoTracerManager::SpawnImpactEffect(GetWorld(), Hit.ImpactPoint,
				Hit.ImpactNormal, HitChar != nullptr);
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

void AExoWeaponBase::AddEnergy(float Amount)
{
	float OldEnergy = CurrentEnergy;
	CurrentEnergy = FMath::Clamp(CurrentEnergy + Amount, 0.f, MaxEnergy);
	if (CurrentEnergy != OldEnergy) OnEnergyChanged.Broadcast(CurrentEnergy);
}

void AExoWeaponBase::TickWeaponSway(float DeltaTime)
{
	if (!ViewModel) return;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	AController* PC = OwnerPawn->GetController();
	if (!PC) return;

	FRotator CurrentRot = PC->GetControlRotation();
	FRotator DeltaRot = CurrentRot - PrevControlRotation;
	DeltaRot.Normalize();
	PrevControlRotation = CurrentRot;

	// Accumulate sway offset from mouse movement
	SwayOffset.Y += DeltaRot.Yaw * SwayAmount;
	SwayOffset.Z += DeltaRot.Pitch * SwayAmount;

	// Clamp
	SwayOffset.Y = FMath::Clamp(SwayOffset.Y, -MaxSwayOffset, MaxSwayOffset);
	SwayOffset.Z = FMath::Clamp(SwayOffset.Z, -MaxSwayOffset, MaxSwayOffset);

	// Return to center
	SwayOffset = FMath::VInterpTo(SwayOffset, FVector::ZeroVector, DeltaTime, SwayReturnSpeed);

	// Idle sway — subtle breathing motion
	float Time = GetWorld()->GetTimeSeconds();
	float IdleY = FMath::Sin(Time * 1.2f) * 0.15f;
	float IdleZ = FMath::Sin(Time * 0.8f + 0.7f) * 0.1f;

	FVector BasePos(20.f, 10.f, -8.f);
	ViewModel->SetRelativeLocation(BasePos + SwayOffset + FVector(0.f, IdleY, IdleZ));
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
