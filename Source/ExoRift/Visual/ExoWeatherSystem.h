#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoWeatherSystem.generated.h"

class AExoPostProcess;

UENUM(BlueprintType)
enum class EExoWeatherState : uint8
{
	Clear,
	Overcast,
	Rain,
	Storm,
	Fog
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherChanged, EExoWeatherState, NewWeather);

/**
 * Dynamic weather system. Singleton — one per level.
 * Cycles through weather states, blending fog/visibility/wind over time.
 * AI reads GetVisibilityMultiplier() to scale sight distance.
 */
UCLASS()
class EXORIFT_API AExoWeatherSystem : public AActor
{
	GENERATED_BODY()

public:
	AExoWeatherSystem();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Find the weather system in the world. */
	static AExoWeatherSystem* Get(UWorld* World);

	/** Current visibility multiplier (0-1). AI should multiply sight radius by this. */
	float GetVisibilityMultiplier() const { return CurrentVisibility; }

	/** Current weather state (for HUD display etc.). */
	EExoWeatherState GetCurrentWeather() const { return CurrentWeather; }

	/** Broadcast when weather finishes transitioning. */
	UPROPERTY(BlueprintAssignable, Category = "Weather")
	FOnWeatherChanged OnWeatherChanged;

	// --- Tuning ---

	UPROPERTY(EditAnywhere, Category = "Weather")
	float WeatherChangeInterval = 120.f;

	UPROPERTY(EditAnywhere, Category = "Weather")
	float TransitionDuration = 15.f;

protected:
	/** Look up the target values for a given weather state. */
	void GetWeatherParams(EExoWeatherState State, float& OutFogDensity,
		FLinearColor& OutFogColor, float& OutWindStrength,
		float& OutVisibility, bool& bOutRaining) const;

	void PickNextWeather();
	void ApplyToPostProcess();

private:
	EExoWeatherState CurrentWeather = EExoWeatherState::Clear;
	EExoWeatherState TargetWeather = EExoWeatherState::Clear;

	float ChangeTimer = 0.f;
	float TransitionAlpha = 1.f; // 1 = fully arrived
	bool bTransitioning = false;

	// Blended values (interpolated during transition)
	float CurrentFogDensity = 0.f;
	FLinearColor CurrentFogColor = FLinearColor::White;
	float CurrentWindStrength = 0.f;
	float CurrentVisibility = 1.f;
	bool bCurrentRaining = false;

	// Snapshot of "from" state at start of transition
	float FromFogDensity = 0.f;
	FLinearColor FromFogColor = FLinearColor::White;
	float FromWindStrength = 0.f;
	float FromVisibility = 1.f;

	// Target state values
	float TargetFogDensity = 0.f;
	FLinearColor TargetFogColor = FLinearColor::White;
	float TargetWindStrength = 0.f;
	float TargetVisibility = 1.f;
	bool bTargetRaining = false;
};
