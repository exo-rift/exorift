#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoEnvironmentAnimator.generated.h"

class UPointLightComponent;
class USpotLightComponent;

/**
 * Animates static environment elements placed by the level builder:
 * flickering neon, rotating spotlights, pulsing holograms.
 * Spawned by world setup; finds and modifies lights in the level.
 */
UCLASS()
class AExoEnvironmentAnimator : public AActor
{
	GENERATED_BODY()

public:
	AExoEnvironmentAnimator();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void GatherLights();
	void TickFlickerLights(float Time);
	void TickSpotlightSweep(float Time);
	void TickAmbientPulse(float Time);

	// Neon/accent lights that flicker — subset of all point lights
	UPROPERTY()
	TArray<UPointLightComponent*> FlickerLights;
	TArray<float> FlickerPhases;
	TArray<float> FlickerBaseIntensity;

	// Spotlights that sweep slowly
	UPROPERTY()
	TArray<USpotLightComponent*> SweepSpots;
	TArray<float> SweepPhases;

	// Compound ambient lights that pulse gently
	UPROPERTY()
	TArray<UPointLightComponent*> AmbientLights;
	TArray<float> AmbientPhases;
	TArray<float> AmbientBaseIntensity;

	bool bGathered = false;
};
