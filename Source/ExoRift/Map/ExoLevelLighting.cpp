#include "Map/ExoLevelLighting.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "ExoRift.h"

AExoLevelLighting::AExoLevelLighting()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Sun (directional light)
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(Root);
	SunLight->SetIntensity(SunIntensity);
	SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f));
	SunLight->bUsedAsAtmosphereSunLight = true;
	SunLight->SetCastShadows(true);
	SunLight->SetDynamicShadowCascades(4);

	// Sky light
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetIntensity(1.0f);
	SkyLight->bRealTimeCapture = true;

	// Atmosphere
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(Root);

	// Height fog
	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(Root);
	HeightFog->SetFogDensity(FogDensity);
	HeightFog->SetFogMaxOpacity(FogMaxOpacity);
	HeightFog->SetFogInscatteringColor(FLinearColor(0.45f, 0.55f, 0.7f));
	HeightFog->SetVolumetricFog(true);

	// Volumetric clouds
	VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
	VolumetricClouds->SetupAttachment(Root);
	VolumetricClouds->SetLayerBottomAltitude(8.f);
	VolumetricClouds->SetLayerHeight(10.f);
}

void AExoLevelLighting::BeginPlay()
{
	Super::BeginPlay();
	SetupLighting();
}

void AExoLevelLighting::SetupLighting()
{
	UpdateSunPosition();
	UpdateLightColors();
	UE_LOG(LogExoRift, Log, TEXT("LevelLighting: initialized, time=%s"),
		*UEnum::GetValueAsString(TimeOfDay));
}

void AExoLevelLighting::SetTimeOfDay(ETimeOfDay NewTime)
{
	TimeOfDay = NewTime;
	UpdateSunPosition();
	UpdateLightColors();
}

void AExoLevelLighting::UpdateSunPosition()
{
	float Pitch = 0.f;
	float Yaw = -45.f;

	switch (TimeOfDay)
	{
	case ETimeOfDay::Dawn:
		Pitch = -5.f; Yaw = -90.f; break;
	case ETimeOfDay::Morning:
		Pitch = -30.f; Yaw = -60.f; break;
	case ETimeOfDay::Noon:
		Pitch = -75.f; Yaw = -45.f; break;
	case ETimeOfDay::Afternoon:
		Pitch = -45.f; Yaw = 30.f; break;
	case ETimeOfDay::Dusk:
		Pitch = -10.f; Yaw = 80.f; break;
	case ETimeOfDay::Night:
		Pitch = 15.f; Yaw = 120.f; break;
	}

	SunLight->SetWorldRotation(FRotator(Pitch, Yaw, 0.f));
}

void AExoLevelLighting::UpdateLightColors()
{
	FLinearColor SunColor;
	FLinearColor FogColor;
	float Intensity = SunIntensity;
	float FogDens = FogDensity;

	switch (TimeOfDay)
	{
	case ETimeOfDay::Dawn:
		SunColor = FLinearColor(1.0f, 0.6f, 0.3f);
		FogColor = FLinearColor(0.6f, 0.4f, 0.3f);
		Intensity = 4.f;
		FogDens = 0.005f;
		break;
	case ETimeOfDay::Morning:
		SunColor = FLinearColor(1.0f, 0.9f, 0.75f);
		FogColor = FLinearColor(0.5f, 0.6f, 0.7f);
		Intensity = 8.f;
		FogDens = 0.003f;
		break;
	case ETimeOfDay::Noon:
		SunColor = FLinearColor(1.0f, 0.98f, 0.95f);
		FogColor = FLinearColor(0.45f, 0.55f, 0.7f);
		Intensity = 10.f;
		FogDens = 0.002f;
		break;
	case ETimeOfDay::Afternoon:
		SunColor = FLinearColor(1.0f, 0.85f, 0.65f);
		FogColor = FLinearColor(0.5f, 0.5f, 0.55f);
		Intensity = 7.f;
		FogDens = 0.003f;
		break;
	case ETimeOfDay::Dusk:
		SunColor = FLinearColor(1.0f, 0.45f, 0.2f);
		FogColor = FLinearColor(0.5f, 0.3f, 0.25f);
		Intensity = 3.f;
		FogDens = 0.006f;
		break;
	case ETimeOfDay::Night:
		SunColor = FLinearColor(0.15f, 0.2f, 0.35f);
		FogColor = FLinearColor(0.05f, 0.08f, 0.15f);
		Intensity = 0.5f;
		FogDens = 0.004f;
		break;
	}

	SunLight->SetLightColor(SunColor);
	SunLight->SetIntensity(Intensity);
	HeightFog->SetFogDensity(FogDens);
	HeightFog->SetFogInscatteringColor(FogColor);
}
