#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoFlickerLight.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/**
 * Flickering/damaged light fixture.
 * Adds atmospheric tension with irregular flicker patterns.
 */
UCLASS()
class AExoFlickerLight : public AActor
{
	GENERATED_BODY()

public:
	AExoFlickerLight();

	virtual void Tick(float DeltaTime) override;

	void InitLight(const FLinearColor& Color, float InBaseIntensity = 8000.f);

private:
	UPROPERTY()
	UPointLightComponent* Light;

	UPROPERTY()
	UStaticMeshComponent* FixtureMesh;

	float BaseIntensity = 8000.f;
	float FlickerTimer = 0.f;
	float NextFlickerTime = 0.f;
	bool bIsOff = false;
	float OffDuration = 0.f;
	float OffTimer = 0.f;
};
