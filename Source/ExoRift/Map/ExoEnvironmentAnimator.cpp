// ExoEnvironmentAnimator.cpp — Brings the static level to life with light animation
#include "Map/ExoEnvironmentAnimator.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "EngineUtils.h"

AExoEnvironmentAnimator::AExoEnvironmentAnimator()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AExoEnvironmentAnimator::BeginPlay()
{
	Super::BeginPlay();
}

void AExoEnvironmentAnimator::GatherLights()
{
	bGathered = true;

	// Find all point lights in the level — classify by intensity/color
	for (TObjectIterator<UPointLightComponent> It; It; ++It)
	{
		UPointLightComponent* PL = *It;
		if (!PL || !PL->GetWorld() || PL->GetWorld() != GetWorld()) continue;

		float Intensity = PL->Intensity;
		FLinearColor Col = PL->GetLightColor();

		// Neon-range lights (intensity 2000-6000, colored) — flicker candidates
		if (Intensity >= 2000.f && Intensity <= 6000.f)
		{
			float Luminance = Col.R * 0.3f + Col.G * 0.6f + Col.B * 0.1f;
			// Colored lights (not pure white) are likely neon/accent
			if (Luminance < 0.7f && (Col.R > 0.3f || Col.G > 0.3f || Col.B > 0.3f))
			{
				// Only flicker ~30% of qualifying lights for variety
				if (FMath::RandRange(0, 100) < 30)
				{
					FlickerLights.Add(PL);
					FlickerPhases.Add(FMath::RandRange(0.f, 2.f * PI));
					FlickerBaseIntensity.Add(Intensity);
				}
			}
		}

		// Ambient compound lights (intensity 5000-8000) — gentle pulse
		if (Intensity >= 5000.f && Intensity <= 8000.f && PL->AttenuationRadius > 3000.f)
		{
			AmbientLights.Add(PL);
			AmbientPhases.Add(FMath::RandRange(0.f, 2.f * PI));
			AmbientBaseIntensity.Add(Intensity);
		}
	}

	// Find spotlights for sweep
	for (TObjectIterator<USpotLightComponent> It; It; ++It)
	{
		USpotLightComponent* SL = *It;
		if (!SL || !SL->GetWorld() || SL->GetWorld() != GetWorld()) continue;

		// Only sweep lights pointing down (beacons/searchlights)
		FRotator Rot = SL->GetComponentRotation();
		if (Rot.Pitch < -60.f)
		{
			SweepSpots.Add(SL);
			SweepPhases.Add(FMath::RandRange(0.f, 2.f * PI));
		}
	}
}

void AExoEnvironmentAnimator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Delay gather to ensure level builder has finished
	if (!bGathered)
	{
		GatherLights();
		return;
	}

	float Time = GetWorld()->GetTimeSeconds();
	TickFlickerLights(Time);
	TickSpotlightSweep(Time);
	TickAmbientPulse(Time);
}

void AExoEnvironmentAnimator::TickFlickerLights(float Time)
{
	for (int32 i = 0; i < FlickerLights.Num(); i++)
	{
		if (!FlickerLights[i]) continue;

		float Phase = FlickerPhases[i];
		float Base = FlickerBaseIntensity[i];

		// Multi-frequency flicker: fast random + slow pulse
		float Fast = FMath::Sin(Time * (45.f + i * 7.3f) + Phase);
		float Slow = FMath::Sin(Time * (2.f + i * 0.3f) + Phase) * 0.15f;

		// Occasional brief dropout (off for a frame)
		float Dropout = (FMath::Sin(Time * (8.f + i * 3.f) + Phase) > 0.92f) ? 0.1f : 1.f;

		float Flicker = FMath::Clamp(0.7f + Fast * 0.3f + Slow, 0.1f, 1.3f) * Dropout;
		FlickerLights[i]->SetIntensity(Base * Flicker);
	}
}

void AExoEnvironmentAnimator::TickSpotlightSweep(float Time)
{
	for (int32 i = 0; i < SweepSpots.Num(); i++)
	{
		if (!SweepSpots[i]) continue;

		float Phase = SweepPhases[i];
		FRotator Base = SweepSpots[i]->GetComponentRotation();

		// Gentle yaw sweep +-15 degrees
		float Sweep = FMath::Sin(Time * 0.3f + Phase) * 15.f;
		FRotator NewRot(-80.f, Base.Yaw + Sweep * GetWorld()->GetDeltaSeconds(), 0.f);
		SweepSpots[i]->SetWorldRotation(
			FMath::RInterpTo(SweepSpots[i]->GetComponentRotation(), NewRot,
				GetWorld()->GetDeltaSeconds(), 2.f));
	}
}

void AExoEnvironmentAnimator::TickAmbientPulse(float Time)
{
	for (int32 i = 0; i < AmbientLights.Num(); i++)
	{
		if (!AmbientLights[i]) continue;

		float Phase = AmbientPhases[i];
		float Base = AmbientBaseIntensity[i];

		// Very gentle breathing pulse
		float Pulse = 1.f + 0.08f * FMath::Sin(Time * 0.7f + Phase)
			+ 0.04f * FMath::Sin(Time * 1.3f + Phase * 2.f);
		AmbientLights[i]->SetIntensity(Base * Pulse);
	}
}
