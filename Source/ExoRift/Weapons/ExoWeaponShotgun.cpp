#include "Weapons/ExoWeaponShotgun.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoArmorComponent.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoTracerManager.h"
#include "Core/ExoAudioManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoWeaponShotgun::AExoWeaponShotgun()
{
	WeaponName = TEXT("Scatter Storm");
	WeaponType = EWeaponType::Shotgun;
	bIsAutomatic = false;
	FireRate = 1.5f;
	Damage = 12.f; // Per pellet
	MaxRange = 8000.f;
	NumPellets = 8;
	PelletSpreadAngle = 5.f;

	// Heat system: 20/100 normalized
	HeatPerShot = 0.2f;
	CooldownRate = 0.25f;
	OverheatCooldownRate = 0.1f; // Moderate overheat penalty (2.0s feel)
	OverheatRecoveryThreshold = 0.3f;

	// Energy cell
	MaxEnergy = 40.f;
	CurrentEnergy = 40.f;
	EnergyPerShot = 4.f;

	// Wide base spread for a shotgun
	BaseSpread = 1.f;
	MaxSpread = 5.f;
	SpreadPerShot = 1.5f;
	SpreadRecoveryRate = 3.f;

	// Punchy recoil
	RecoilPitch = -0.8f;
	RecoilYawRange = 0.4f;

	// Headshot
	HeadshotMultiplier = 2.f;

	// Very short range falloff: full to 30m, zero at 80m
	FalloffStartRange = 3000.f;
	FalloffEndRange = 8000.f;
	MinDamageMultiplier = 0.2f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ShotgunMesh(
		TEXT("/Game/Weapons/Shotgun/SKM_Shotgun"));
	if (ShotgunMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(ShotgunMesh.Object);
	}
}

void AExoWeaponShotgun::FireShot()
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

	// Get aim origin
	FVector ViewStart;
	FRotator ViewDir;
	if (OwnerPawn && OwnerPawn->GetController())
	{
		OwnerPawn->GetController()->GetPlayerViewPoint(ViewStart, ViewDir);
	}
	else
	{
		return;
	}

	// Fire each pellet with random cone spread
	for (int32 i = 0; i < NumPellets; ++i)
	{
		FirePellet(ViewStart, ViewDir, OwnerPawn);
	}

	// Muzzle flash (once per shot, not per pellet)
	FVector MuzzleLoc = WeaponMesh
		? WeaponMesh->GetSocketLocation(TEXT("Muzzle"))
		: GetActorLocation();
	FExoTracerManager::SpawnMuzzleFlash(GetWorld(), MuzzleLoc, GetActorRotation());
}

void AExoWeaponShotgun::FirePellet(
	const FVector& Start, const FRotator& AimDir, APawn* OwnerPawn)
{
	// Random deviation within cone
	float SpreadRad = FMath::DegreesToRadians(PelletSpreadAngle + CurrentSpread);
	FVector PelletDir = FMath::VRandCone(AimDir.Vector(), SpreadRad);
	FVector End = Start + PelletDir * MaxRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerPawn);
	Params.bTraceComplex = true;

	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// Tracer
	FVector MuzzleLoc = WeaponMesh
		? WeaponMesh->GetSocketLocation(TEXT("Muzzle"))
		: GetActorLocation();
	FVector TraceEnd = Hit.bBlockingHit ? Hit.ImpactPoint : End;
	FExoTracerManager::SpawnTracer(GetWorld(), MuzzleLoc, TraceEnd, Hit.bBlockingHit);

	if (!Hit.bBlockingHit) return;

	AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	AController* InstigatorController = nullptr;
	if (OwnerPawn) InstigatorController = OwnerPawn->GetController();

	// Per-pellet damage with rarity scaling
	float FinalDamage = Damage * GetRarityDamageMultiplier();

	// Distance falloff
	if (Hit.Distance > FalloffStartRange && FalloffEndRange > FalloffStartRange)
	{
		float Alpha = FMath::Clamp(
			(Hit.Distance - FalloffStartRange) / (FalloffEndRange - FalloffStartRange),
			0.f, 1.f);
		FinalDamage *= FMath::Lerp(1.f, MinDamageMultiplier, Alpha);
	}

	// Headshot detection
	bool bHeadshot = false;
	AExoCharacter* HitChar = Cast<AExoCharacter>(HitActor);
	if (HitChar)
	{
		FVector HeadLoc = HitChar->GetActorLocation() +
			FVector(0.f, 0.f,
				HitChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.75f);
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
	if (HitChar && OwnerPawn && OwnerPawn->IsLocallyControlled())
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

	// Damage number
	AExoDamageNumbers* DmgNums = AExoDamageNumbers::Get(GetWorld());
	if (DmgNums) DmgNums->SpawnDamageNumber(Hit.ImpactPoint, FinalDamage, bHeadshot);

	// Impact sound
	UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld());
	if (Audio) Audio->PlayImpactSound(Hit.ImpactPoint, HitChar != nullptr);
}
