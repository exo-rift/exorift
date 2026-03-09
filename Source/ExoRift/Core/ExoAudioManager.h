#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ExoAudioManager.generated.h"

class USoundBase;

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
	void PlayZoneWarningSound();
	void PlayMatchPhaseSound();
	void PlayCountdownTick();

	// Footstep sounds
	void PlayFootstepSound(const FVector& Location, bool bIsSprinting);

	// 3D spatial sounds
	void PlayWeaponFireSound(USoundBase* Sound, const FVector& Location, float Volume = 1.f);
	void PlayImpactSound(const FVector& Location, bool bFlesh = false);
	void PlayExplosionSound(const FVector& Location);

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
