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
	UPointLightComponent* FlashLight;

	float Age = 0.f;
	float Lifetime = 0.07f;
	float BaseIntensity = 0.f;
	float CoreScale = 0.15f;
	float CrossScale = 0.12f;
	float FlareScale = 0.10f;
};
