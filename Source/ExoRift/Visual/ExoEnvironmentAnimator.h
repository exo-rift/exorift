#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoEnvironmentAnimator.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Lightweight animator that pulses, rotates, and flickers environment
 * props registered by the level builder. Single instance per level.
 */
UCLASS()
class AExoEnvironmentAnimator : public AActor
{
	GENERATED_BODY()

public:
	AExoEnvironmentAnimator();
	virtual void Tick(float DeltaTime) override;

	// Registration functions — call after spawning props
	void RegisterPylonRing(UStaticMeshComponent* Ring, UPointLightComponent* Light);
	void RegisterConsoleScreen(UMaterialInstanceDynamic* ScreenMat, UPointLightComponent* Light);
	void RegisterWindowGlow(UMaterialInstanceDynamic* WinMat, float Phase);
	void RegisterHologram(UStaticMeshComponent* Mesh, UPointLightComponent* Light);

	static AExoEnvironmentAnimator* Get(UWorld* World);

private:
	struct FPylonEntry
	{
		TWeakObjectPtr<UStaticMeshComponent> Ring;
		TWeakObjectPtr<UPointLightComponent> Light;
		float Phase;
	};
	TArray<FPylonEntry> Pylons;

	struct FConsoleEntry
	{
		TWeakObjectPtr<UMaterialInstanceDynamic> Mat;
		TWeakObjectPtr<UPointLightComponent> Light;
		float FlickerTimer;
		float BaseIntensity;
	};
	TArray<FConsoleEntry> Consoles;

	struct FWindowEntry
	{
		TWeakObjectPtr<UMaterialInstanceDynamic> Mat;
		float Phase;
	};
	TArray<FWindowEntry> Windows;

	struct FHologramEntry
	{
		TWeakObjectPtr<UStaticMeshComponent> Mesh;
		TWeakObjectPtr<UPointLightComponent> Light;
		float Phase;
		float BaseZ = 0.f;
	};
	TArray<FHologramEntry> Holograms;
};
