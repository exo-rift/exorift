#pragma once

#include "CoreMinimal.h"
#include "Core/ExoTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "ExoAudioManager.generated.h"

class USoundBase;

enum class EFootstepSurface : uint8
{
	Concrete,
	Metal,
	Water
};

// Centralized audio management — plays 2D UI sounds and manages spatial audio
UCLASS()
class EXORIFT_API UExoAudioManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// UI/Feedback sounds (2D)
	void PlayHitMarkerSound(bool bHeadshot = false);
	void PlayKillSound();
	void PlayOverheatSound();
	void PlayShieldBreakSound();
	void PlayShieldRechargeSound();
	void PlayZoneWarningSound();
	void PlayMatchPhaseSound();
	void PlayCountdownTick();
	void PlayVictoryStinger();
	void PlayDefeatStinger();
	void PlayDeathStinger();
	void PlayCombatStinger();

	// Footstep sounds
	void PlayFootstepSound(const FVector& Location, bool bIsSprinting,
		EFootstepSurface Surface = EFootstepSurface::Concrete);

	// 3D spatial sounds
	void PlayWeaponFireSound(USoundBase* Sound, const FVector& Location, float Volume = 1.f);
	void PlayWeaponFireByType(EWeaponType Type, const FVector& Location);
	void PlayGrenadeExplosionSound(const FVector& Location);
	void PlayAbilityActivateSound(const FVector& Location, float Pitch = 1.f);
	void PlayImpactSound(const FVector& Location, bool bFlesh = false);
	void PlayExplosionSound(const FVector& Location);
	void PlayDoorSlideSound(const FVector& Location, bool bOpening);
	void PlayVehicleEngineSound(const FVector& Location, bool bStartup);

	// Trap sounds
	void PlayTrapPlacedSound();
	void PlayTrapActivationBeep(const FVector& Location);

	// Movement sounds
	void PlaySlideStartSound(const FVector& Location);
	void PlaySlideLoopSound(const FVector& Location);
	void PlaySlideStopSound(const FVector& Location);
	void PlayWindSound(const FVector& Location, float Intensity = 1.f);

	static UExoAudioManager* Get(UWorld* World);

protected:
	// Sound refs loaded from content or created procedurally
	UPROPERTY()
	USoundBase* HitMarkerSound = nullptr;

	UPROPERTY()
	USoundBase* HeadshotSound = nullptr;

	UPROPERTY()
	USoundBase* KillConfirmSound = nullptr;

	UPROPERTY()
	USoundBase* OverheatSound = nullptr;

	UPROPERTY()
	USoundBase* WeaponFireSound = nullptr;
};
