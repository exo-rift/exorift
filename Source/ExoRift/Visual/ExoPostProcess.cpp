#include "Visual/ExoPostProcess.h"
#include "Components/PostProcessComponent.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoPostProcess::AExoPostProcess()
{
	PrimaryActorTick.bCanEverTick = true;

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	RootComponent = PostProcessComp;
	PostProcessComp->bUnbound = true; // Affects entire scene
	PostProcessComp->Priority = 1.f;

	// Base settings for sci-fi look
	PostProcessComp->Settings.bOverride_BloomIntensity = true;
	PostProcessComp->Settings.BloomIntensity = 0.7f;

	PostProcessComp->Settings.bOverride_AutoExposureBias = true;
	PostProcessComp->Settings.AutoExposureBias = 0.f;

	PostProcessComp->Settings.bOverride_VignetteIntensity = true;
	PostProcessComp->Settings.VignetteIntensity = 0.3f;

	PostProcessComp->Settings.bOverride_FilmGrainIntensity = true;
	PostProcessComp->Settings.FilmGrainIntensity = 0.05f;

	PostProcessComp->Settings.bOverride_ColorContrastShadows = true;
	PostProcessComp->Settings.ColorContrastShadows = FVector4(1.05f, 1.05f, 1.1f, 1.f);

	PostProcessComp->Settings.bOverride_SceneFringeIntensity = true;
	PostProcessComp->Settings.SceneFringeIntensity = 0.f;
}

void AExoPostProcess::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Decay damage flash
	if (DamageFlashIntensity > 0.f)
	{
		DamageFlashIntensity = FMath::Max(0.f, DamageFlashIntensity - DamageFlashDecayRate * DeltaTime);
	}

	// Smooth low health blend
	LowHealthBlend = FMath::FInterpTo(LowHealthBlend, TargetLowHealthBlend, DeltaTime, 3.f);

	// Decay kill effect
	if (KillEffectIntensity > 0.f)
	{
		KillEffectIntensity = FMath::Max(0.f, KillEffectIntensity - KillEffectDecayRate * DeltaTime);
	}

	// Smooth zone damage
	ZoneDamageIntensity = FMath::FInterpTo(ZoneDamageIntensity, TargetZoneDamageIntensity, DeltaTime, 5.f);

	// Smooth low-health heartbeat pulse
	LowHealthPulseBlend = FMath::FInterpTo(LowHealthPulseBlend, TargetLowHealthPulseBlend, DeltaTime, 4.f);

	// Smooth speed boost
	SpeedBoostAlpha = FMath::FInterpTo(SpeedBoostAlpha, TargetSpeedBoostAlpha, DeltaTime, 6.f);

	// --- Combine all effects ---

	// Vignette: base + damage flash + low health + zone damage + heartbeat
	float TotalVignette = 0.3f;
	TotalVignette += DamageFlashIntensity * 0.5f;
	TotalVignette += LowHealthBlend * 0.3f;
	TotalVignette += ZoneDamageIntensity * 0.4f;

	// Heartbeat pulse at critical health (<25%)
	if (LowHealthPulseBlend > 0.01f)
	{
		float Time = GetWorld()->GetTimeSeconds();
		// Double-beat pattern: fast beat, pause, fast beat
		float HeartPhase = FMath::Fmod(Time * 1.8f, 1.f);
		float Beat = 0.f;
		if (HeartPhase < 0.15f)
			Beat = FMath::Sin(HeartPhase / 0.15f * PI);
		else if (HeartPhase > 0.25f && HeartPhase < 0.40f)
			Beat = FMath::Sin((HeartPhase - 0.25f) / 0.15f * PI) * 0.7f;

		TotalVignette += Beat * LowHealthPulseBlend * 0.35f;
	}

	PostProcessComp->Settings.VignetteIntensity = TotalVignette;

	// Scene color tint: damage flash (warm) + zone damage (red)
	float DamageTint = DamageFlashIntensity * 0.3f;
	float ZoneTint = ZoneDamageIntensity * 0.4f;
	float CombinedTint = DamageTint + ZoneTint;

	PostProcessComp->Settings.bOverride_SceneColorTint = (CombinedTint > 0.01f);
	if (CombinedTint > 0.01f)
	{
		PostProcessComp->Settings.SceneColorTint = FLinearColor(
			1.f + CombinedTint,
			1.f - CombinedTint * 0.5f,
			1.f - CombinedTint * 0.5f, 1.f);
	}

	// Low health: desaturation + pulsing vignette
	if (LowHealthBlend > 0.01f)
	{
		PostProcessComp->Settings.bOverride_ColorSaturation = true;
		float Desat = 1.f - LowHealthBlend * 0.4f;
		PostProcessComp->Settings.ColorSaturation = FVector4(Desat, Desat, Desat, 1.f);

		if (LowHealthBlend > 0.5f)
		{
			float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f));
			PostProcessComp->Settings.VignetteIntensity += Pulse * LowHealthBlend * 0.2f;
		}
	}
	else
	{
		PostProcessComp->Settings.bOverride_ColorSaturation = false;
	}

	// Chromatic aberration: kill effect + zone damage
	float TotalFringe = KillEffectIntensity * 3.f + ZoneDamageIntensity * 2.f;
	PostProcessComp->Settings.SceneFringeIntensity = TotalFringe;

	// Speed boost: motion blur + bloom increase
	PostProcessComp->Settings.bOverride_MotionBlurAmount = (SpeedBoostAlpha > 0.01f);
	if (SpeedBoostAlpha > 0.01f)
	{
		PostProcessComp->Settings.MotionBlurAmount = SpeedBoostAlpha * 0.6f;
	}
	PostProcessComp->Settings.BloomIntensity = 0.7f + SpeedBoostAlpha * 0.4f;
}

void AExoPostProcess::TriggerDamageFlash(float Intensity)
{
	DamageFlashIntensity = FMath::Min(DamageFlashIntensity + Intensity, 1.f);
}

void AExoPostProcess::SetLowHealthEffect(float HealthPercent)
{
	// Effect ramps up below 30% health
	if (HealthPercent < 0.3f)
	{
		TargetLowHealthBlend = 1.f - (HealthPercent / 0.3f);
	}
	else
	{
		TargetLowHealthBlend = 0.f;
	}
}

void AExoPostProcess::TriggerKillEffect()
{
	KillEffectIntensity = 1.f;
}

void AExoPostProcess::ApplyZoneDamageEffect(float Intensity)
{
	TargetZoneDamageIntensity = FMath::Clamp(Intensity, 0.f, 1.f);
}

void AExoPostProcess::ApplyLowHealthPulse(float HealthPercent)
{
	// Heartbeat pulse kicks in below 25% health, intensifies toward 0%
	if (HealthPercent < 0.25f && HealthPercent > 0.f)
	{
		TargetLowHealthPulseBlend = 1.f - (HealthPercent / 0.25f);
	}
	else
	{
		TargetLowHealthPulseBlend = 0.f;
	}
}

void AExoPostProcess::ApplySpeedBoostEffect(float Alpha)
{
	TargetSpeedBoostAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
}

AExoPostProcess* AExoPostProcess::Get(UWorld* World)
{
	if (!World) return nullptr;

	for (TActorIterator<AExoPostProcess> It(World); It; ++It)
	{
		return *It;
	}

	return World->SpawnActor<AExoPostProcess>();
}
