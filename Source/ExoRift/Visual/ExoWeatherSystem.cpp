#include "Visual/ExoWeatherSystem.h"
#include "Visual/ExoPostProcess.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
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

	// Initialize current values from Clear state
	bool bDummy;
	GetWeatherParams(EExoWeatherState::Clear, CurrentFogDensity, CurrentFogColor,
		CurrentWindStrength, CurrentVisibility, bDummy);
	bCurrentRaining = false;

	ChangeTimer = WeatherChangeInterval;
	TransitionAlpha = 1.f;
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
	}
}

// ---------------------------------------------------------------------------
// Weather transition
// ---------------------------------------------------------------------------

void AExoWeatherSystem::PickNextWeather()
{
	// Pick a random different weather
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
		OutFogDensity = 0.f;
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
// Post-process integration
// ---------------------------------------------------------------------------

void AExoWeatherSystem::ApplyToPostProcess()
{
	AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
	if (!PP) return;

	// Subtle bloom increase during fog/storm
	// Post-process component is not directly accessible via public API,
	// so we adjust through the existing interface indirectly.
	// For now we just log — the main gameplay effect is the visibility multiplier
	// that AI reads, plus HUD showing the state. Full fog volume integration
	// would require ExponentialHeightFog actor which can be added later.
}
