#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoMuzzleFlash.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/** Brief bright cross-shaped flash at weapon muzzle with weapon-colored light. */
UCLASS()
class AExoMuzzleFlash : public AActor
{
	GENERATED_BODY()

public:
	AExoMuzzleFlash();

	void InitFlash(const FRotator& FireDirection,
		const FLinearColor& WeaponColor, EWeaponType WeaponType);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* FlashCore;

	UPROPERTY()
	UStaticMeshComponent* FlashCross;

	UPROPERTY()
	UStaticMeshComponent* FlashFlare;

	UPROPERTY()
	UStaticMeshComponent* FlashRing; // Expanding halo ring

	UPROPERTY()
	UPointLightComponent* FlashLight;

	UPROPERTY()
	UPointLightComponent* BounceLight; // Ground-bounce secondary light

	float Age = 0.f;
	float Lifetime = 0.09f;
	float BaseIntensity = 0.f;
	float CoreScale = 0.25f;
	float CrossScale = 0.20f;
	float FlareScale = 0.16f;
	float RingScale = 0.35f;
};
