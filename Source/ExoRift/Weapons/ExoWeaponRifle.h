#pragma once

#include "CoreMinimal.h"
#include "Weapons/ExoWeaponBase.h"
#include "ExoWeaponRifle.generated.h"

UENUM()
enum class ERifleFireMode : uint8
{
	FullAuto,
	Burst
};

UCLASS()
class EXORIFT_API AExoWeaponRifle : public AExoWeaponBase
{
	GENERATED_BODY()

public:
	AExoWeaponRifle();

	virtual void Tick(float DeltaTime) override;
	virtual void StartFire() override;
	virtual void ToggleFireMode() override;

	ERifleFireMode GetFireMode() const { return FireMode; }

protected:
	ERifleFireMode FireMode = ERifleFireMode::FullAuto;

	// Burst state
	int32 BurstShotsRemaining = 0;
	float BurstFireInterval = 0.06f; // Faster than normal fire rate within burst
	float BurstCooldown = 0.3f;      // Delay between bursts
	float BurstTimer = 0.f;
	bool bInBurstCooldown = false;
};
