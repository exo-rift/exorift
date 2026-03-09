#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoLevelLighting.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;
class UVolumetricCloudComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Dawn,
	Morning,
	Noon,
	Afternoon,
	Dusk,
	Night
};

/**
 * Manages all level lighting, atmosphere, sky, and fog.
 * Creates a complete outdoor environment with directional sun,
 * sky atmosphere, volumetric clouds, and height fog.
 */
UCLASS()
class EXORIFT_API AExoLevelLighting : public AActor
{
	GENERATED_BODY()

public:
	AExoLevelLighting();

	/** Set time of day (affects sun angle and colors). */
	UPROPERTY(EditAnywhere, Category = "Lighting")
	ETimeOfDay TimeOfDay = ETimeOfDay::Morning;

	/** Sun intensity. */
	UPROPERTY(EditAnywhere, Category = "Lighting")
	float SunIntensity = 10.f;

	/** Fog density at ground level. */
	UPROPERTY(EditAnywhere, Category = "Lighting")
	float FogDensity = 0.002f;

	/** Fog max opacity. */
	UPROPERTY(EditAnywhere, Category = "Lighting")
	float FogMaxOpacity = 0.8f;

	/** Initialize all lighting components. */
	void SetupLighting();

	/** Change time of day at runtime. */
	void SetTimeOfDay(ETimeOfDay NewTime);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	UDirectionalLightComponent* SunLight;

	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	USkyLightComponent* SkyLight;

	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	USkyAtmosphereComponent* SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	UExponentialHeightFogComponent* HeightFog;

	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	UVolumetricCloudComponent* VolumetricClouds;

private:
	/** Apply sun rotation based on time of day. */
	void UpdateSunPosition();

	/** Apply color grading for the time of day. */
	void UpdateLightColors();
};
