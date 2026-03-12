#include "Visual/ExoWeatherSystem.h"
#include "Visual/ExoPostProcess.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoWeatherSystem::AExoWeatherSystem()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AExoWeatherSystem::BeginPlay()
{
	Super::BeginPlay();

	// Create fog component for weather
	FogComp = NewObject<UExponentialHeightFogComponent>(this);
	FogComp->SetupAttachment(RootComponent);
	FogComp->RegisterComponent();
	FogComp->SetFogDensity(0.0f);
	FogComp->SetFogHeightFalloff(0.2f);
	FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.8f, 1.f));
	FogComp->SetStartDistance(5000.f);

	// Create dimming directional light for overcast/storm
	WeatherLightComp = NewObject<UDirectionalLightComponent>(this);
	WeatherLightComp->SetupAttachment(RootComponent);
	WeatherLightComp->RegisterComponent();
	WeatherLightComp->SetWorldRotation(FRotator(-60.f, 0.f, 0.f));
	WeatherLightComp->SetIntensity(0.f); // Off by default
	WeatherLightComp->SetLightColor(FLinearColor(0.66f, 0.77f, 1.1f));
	WeatherLightComp->CastShadows = false;

	// Initialize current values from Clear state
	bool bDummy;
	GetWeatherParams(EExoWeatherState::Clear, CurrentFogDensity, CurrentFogColor,
		CurrentWindStrength, CurrentVisibility, bDummy);
	bCurrentRaining = false;

	ChangeTimer = WeatherChangeInterval;
	TransitionAlpha = 1.f;
	LightningCooldown = FMath::RandRange(5.f, 15.f);
}

AExoWeatherSystem* AExoWeatherSystem::Get(UWorld* World)
{
	if (!World) return nullptr;

	for (TActorIterator<AExoWeatherSystem> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

// ---------------------------------------------------------------------------
// Tick — timer + blending
// ---------------------------------------------------------------------------

void AExoWeatherSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTransitioning)
	{
		TransitionAlpha += DeltaTime / TransitionDuration;
		if (TransitionAlpha >= 1.f)
		{
			TransitionAlpha = 1.f;
			bTransitioning = false;
			CurrentWeather = TargetWeather;
			bCurrentRaining = bTargetRaining;

			OnWeatherChanged.Broadcast(CurrentWeather);
			UE_LOG(LogExoRift, Log, TEXT("Weather: transitioned to %d"), static_cast<int32>(CurrentWeather));
		}

		// Lerp all values
		float A = FMath::Clamp(TransitionAlpha, 0.f, 1.f);
		CurrentFogDensity = FMath::Lerp(FromFogDensity, TargetFogDensity, A);
		CurrentFogColor = FMath::Lerp(FromFogColor, TargetFogColor, A);
		CurrentWindStrength = FMath::Lerp(FromWindStrength, TargetWindStrength, A);
		CurrentVisibility = FMath::Lerp(FromVisibility, TargetVisibility, A);

		ApplyToPostProcess();
	}
	else
	{
		ChangeTimer -= DeltaTime;
		if (ChangeTimer <= 0.f)
		{
			PickNextWeather();
			ChangeTimer = WeatherChangeInterval;
		}

		// Still apply (for steady-state fog updates)
		ApplyToPostProcess();
	}

	// Rain particles when raining
	if (bCurrentRaining || bTargetRaining)
	{
		SpawnRainParticles(DeltaTime);
	}

	// Lightning during storms — dramatic multi-flash bursts
	UpdateLightning(DeltaTime);
}

// ---------------------------------------------------------------------------
// Weather transition
// ---------------------------------------------------------------------------

void AExoWeatherSystem::PickNextWeather()
{
	EExoWeatherState NewWeather = CurrentWeather;
	int32 Safety = 10;
	while (NewWeather == CurrentWeather && Safety-- > 0)
	{
		NewWeather = static_cast<EExoWeatherState>(FMath::RandRange(0, 4));
	}

	TargetWeather = NewWeather;

	// Snapshot current blended values as the "from" state
	FromFogDensity = CurrentFogDensity;
	FromFogColor = CurrentFogColor;
	FromWindStrength = CurrentWindStrength;
	FromVisibility = CurrentVisibility;

	// Look up target values
	GetWeatherParams(TargetWeather, TargetFogDensity, TargetFogColor,
		TargetWindStrength, TargetVisibility, bTargetRaining);

	TransitionAlpha = 0.f;
	bTransitioning = true;

	UE_LOG(LogExoRift, Log, TEXT("Weather: transitioning from %d to %d"),
		static_cast<int32>(CurrentWeather), static_cast<int32>(TargetWeather));
}

// ---------------------------------------------------------------------------
// Per-state parameter table
// ---------------------------------------------------------------------------

void AExoWeatherSystem::GetWeatherParams(EExoWeatherState State, float& OutFogDensity,
	FLinearColor& OutFogColor, float& OutWindStrength,
	float& OutVisibility, bool& bOutRaining) const
{
	switch (State)
	{
	case EExoWeatherState::Clear:
		OutFogDensity = 0.0002f;
		OutFogColor = FLinearColor(0.7f, 0.8f, 1.f, 1.f);
		OutWindStrength = 0.f;
		OutVisibility = 1.f;
		bOutRaining = false;
		break;

	case EExoWeatherState::Overcast:
		OutFogDensity = 0.001f;
		OutFogColor = FLinearColor(0.5f, 0.55f, 0.6f, 1.f);
		OutWindStrength = 0.3f;
		OutVisibility = 0.8f;
		bOutRaining = false;
		break;

	case EExoWeatherState::Rain:
		OutFogDensity = 0.003f;
		OutFogColor = FLinearColor(0.4f, 0.45f, 0.55f, 1.f);
		OutWindStrength = 0.5f;
		OutVisibility = 0.6f;
		bOutRaining = true;
		break;

	case EExoWeatherState::Storm:
		OutFogDensity = 0.005f;
		OutFogColor = FLinearColor(0.25f, 0.28f, 0.35f, 1.f);
		OutWindStrength = 1.f;
		OutVisibility = 0.4f;
		bOutRaining = true;
		break;

	case EExoWeatherState::Fog:
		OutFogDensity = 0.01f;
		OutFogColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.f);
		OutWindStrength = 0.1f;
		OutVisibility = 0.3f;
		bOutRaining = false;
		break;
	}
}

// ---------------------------------------------------------------------------
// Post-process & fog integration
// ---------------------------------------------------------------------------

void AExoWeatherSystem::ApplyToPostProcess()
{
	// Update fog component
	if (FogComp)
	{
		FogComp->SetFogDensity(CurrentFogDensity);
		FogComp->SetFogInscatteringColor(CurrentFogColor);

		// Increase fog max opacity during heavy weather
		float FogOpacity = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, 0.01f), FVector2D(0.3f, 0.95f), CurrentFogDensity);
		FogComp->SetFogMaxOpacity(FogOpacity);
	}

	// WeatherLightComp is managed by UpdateLightning for proper flash integration

	// Adjust post-process for weather
	AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
	if (!PP || !PP->PostProcessComp) return;

	// Bloom increases modestly during fog, rain, and lightning
	float WeatherBloom = 0.7f + CurrentFogDensity * 15.f + LightningAlpha * 0.5f;
	PP->PostProcessComp->Settings.BloomIntensity = FMath::Min(WeatherBloom, 1.5f);

	// Color grading shifts cooler during storms, whiter during lightning
	float CoolShift = (1.f - CurrentVisibility) * 0.15f;
	float LightningWhite = LightningBoltAlpha * 0.4f;
	PP->PostProcessComp->Settings.bOverride_ColorGamma = (CoolShift > 0.01f || LightningWhite > 0.01f);
	if (CoolShift > 0.01f || LightningWhite > 0.01f)
	{
		PP->PostProcessComp->Settings.ColorGamma = FVector4(
			1.f - CoolShift * 0.3f + LightningWhite,
			1.f - CoolShift * 0.1f + LightningWhite,
			1.f + CoolShift * 0.2f + LightningWhite * 1.2f, // Extra blue
			1.f);
	}

	// Auto-exposure: darkens during storms, brightens during lightning
	float ExposureBias = CurrentVisibility * 0.5f - 0.2f + LightningBoltAlpha * 1.5f;
	PP->PostProcessComp->Settings.AutoExposureBias = ExposureBias;

	// Lightning triggers screen shake and damage flash
	if (LightningBoltAlpha > 0.8f && PP)
	{
		PP->TriggerDamageFlash(LightningBoltAlpha * 0.3f);
	}
}

float AExoWeatherSystem::GetRainIntensity() const
{
	if (!bCurrentRaining && !bTargetRaining) return 0.f;
	float Intensity = bCurrentRaining ? CurrentWindStrength : 0.f;
	if (bTargetRaining && bTransitioning)
	{
		Intensity = FMath::Max(Intensity, TransitionAlpha * TargetWindStrength);
	}
	return Intensity;
}

// ---------------------------------------------------------------------------
// Rain particle simulation
// ---------------------------------------------------------------------------

void AExoWeatherSystem::SpawnRainParticles(float DeltaTime)
{
	// Get player camera position for rain origin
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

	// Rain intensity based on weather
	float RainIntensity = bCurrentRaining ? CurrentWindStrength : 0.f;
	if (bTargetRaining && bTransitioning)
	{
		RainIntensity = FMath::Max(RainIntensity, TransitionAlpha * TargetWindStrength);
	}

	if (RainIntensity <= 0.01f)
	{
		RainDrops.Empty();
		return;
	}

	// Spawn rate proportional to intensity
	float SpawnRate = RainIntensity * 200.f;
	RainSpawnAccum += SpawnRate * DeltaTime;

	int32 MaxDrops = FMath::RoundToInt32(RainIntensity * 300.f);

	while (RainSpawnAccum >= 1.f && RainDrops.Num() < MaxDrops)
	{
		RainSpawnAccum -= 1.f;

		FRainDrop Drop;
		Drop.Position = PlayerLoc + FVector(
			FMath::RandRange(-5000.f, 5000.f),
			FMath::RandRange(-5000.f, 5000.f),
			FMath::RandRange(2000.f, 4000.f));

		// Rain falls with wind
		Drop.Velocity = FVector(
			CurrentWindStrength * 500.f,
			FMath::RandRange(-100.f, 100.f),
			FMath::RandRange(-4000.f, -6000.f));

		Drop.Life = 0.f;
		RainDrops.Add(Drop);
	}

	// Update & remove dead drops
	for (int32 i = RainDrops.Num() - 1; i >= 0; --i)
	{
		FRainDrop& D = RainDrops[i];
		D.Position += D.Velocity * DeltaTime;
		D.Life += DeltaTime;

		if (D.Life > 1.5f || D.Position.Z < PlayerLoc.Z - 500.f)
		{
			RainDrops.RemoveAtSwap(i);
		}
	}

	// Render rain drops as mesh streaks
	UpdateRainMeshes();
}

// UpdateLightning is in ExoWeatherRain.cpp
