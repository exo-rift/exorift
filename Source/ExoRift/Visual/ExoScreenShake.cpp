#include "Visual/ExoScreenShake.h"

TArray<FExoScreenShake::FShakeInstance> FExoScreenShake::ActiveShakes;
FRotator FExoScreenShake::CurrentOffset = FRotator::ZeroRotator;

void FExoScreenShake::AddShake(float Intensity, float Duration)
{
	if (Intensity <= 0.f || Duration <= 0.f) return;

	FShakeInstance Shake;
	Shake.Intensity = Intensity;
	Shake.Duration = Duration;
	ActiveShakes.Add(Shake);
}

void FExoScreenShake::AddExplosionShake(const FVector& ExplosionLoc,
	const FVector& PlayerLoc, float Radius, float MaxIntensity)
{
	float Dist = FVector::Dist(ExplosionLoc, PlayerLoc);
	if (Dist > Radius) return;

	float Falloff = 1.f - (Dist / Radius);
	float Intensity = MaxIntensity * Falloff * Falloff;
	AddShake(Intensity, 0.4f + Falloff * 0.3f);
}

void FExoScreenShake::Tick(float DeltaTime)
{
	FRotator Accumulated = FRotator::ZeroRotator;

	for (int32 i = ActiveShakes.Num() - 1; i >= 0; i--)
	{
		FShakeInstance& Shake = ActiveShakes[i];
		Shake.Age += DeltaTime;

		if (Shake.Age >= Shake.Duration)
		{
			ActiveShakes.RemoveAt(i);
			continue;
		}

		float Alpha = 1.f - (Shake.Age / Shake.Duration);
		Alpha *= Alpha; // Quadratic falloff
		float Magnitude = Shake.Intensity * Alpha;

		// High-frequency noise for shake feeling
		float TimeScale = Shake.Age * 25.f + i * 7.f;
		Accumulated.Pitch += FMath::Sin(TimeScale * 1.3f) * Magnitude;
		Accumulated.Yaw += FMath::Cos(TimeScale * 0.9f) * Magnitude * 0.6f;
		Accumulated.Roll += FMath::Sin(TimeScale * 1.7f) * Magnitude * 0.3f;
	}

	CurrentOffset = Accumulated;
}

FRotator FExoScreenShake::GetShakeOffset()
{
	return CurrentOffset;
}

bool FExoScreenShake::IsShaking()
{
	return ActiveShakes.Num() > 0;
}
