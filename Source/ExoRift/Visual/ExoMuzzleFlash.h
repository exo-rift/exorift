#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoMuzzleFlash.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/** Brief bright flash at weapon muzzle. Attaches to owner weapon. */
UCLASS()
class AExoMuzzleFlash : public AActor
{
	GENERATED_BODY()

public:
	AExoMuzzleFlash();

	void InitFlash(const FRotator& FireDirection);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* FlashMesh;

	UPROPERTY()
	UPointLightComponent* FlashLight;

	float Age = 0.f;
	float Lifetime = 0.06f;
	float BaseIntensity = 0.f;
};
