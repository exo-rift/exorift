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
	PostProcessComp->Settings.BloomIntensity = 0.8f;

	PostProcessComp->Settings.bOverride_BloomThreshold = true;
	PostProcessComp->Settings.BloomThreshold = 0.8f;

	PostProcessComp->Settings.bOverride_AutoExposureBias = true;
	PostProcessComp->Settings.AutoExposureBias = 0.f;

	PostProcessComp->Settings.bOverride_VignetteIntensity = true;
	PostProcessComp->Settings.VignetteIntensity = 0.3f;

	PostProcessComp->Settings.bOverride_FilmGrainIntensity = true;
	PostProcessComp->Settings.FilmGrainIntensity = 0.04f;

	// Cool shadow tint — blue-shifted shadows for sci-fi atmosphere
	PostProcessComp->Settings.bOverride_ColorContrastShadows = true;
	PostProcessComp->Settings.ColorContrastShadows = FVector4(1.05f, 1.05f, 1.12f, 1.f);

	PostProcessComp->Settings.bOverride_ColorGammaShadows = true;
	PostProcessComp->Settings.ColorGammaShadows = FVector4(0.98f, 0.98f, 1.04f, 1.f);

	// Slightly warm highlights for contrast against cool shadows
	PostProcessComp->Settings.bOverride_ColorGainHighlights = true;
	PostProcessComp->Settings.ColorGainHighlights = FVector4(1.02f, 1.0f, 0.97f, 1.f);

	PostProcessComp->Settings.bOverride_SceneFringeIntensity = true;
	PostProcessComp->Settings.SceneFringeIntensity = 0.f;

	// Ambient occlusion for depth in enclosed spaces
	PostProcessComp->Settings.bOverride_AmbientOcclusionIntensity = true;
	PostProcessComp->Settings.AmbientOcclusionIntensity = 0.5f;
	PostProcessComp->Settings.bOverride_AmbientOcclusionRadius = true;
	PostProcessComp->Settings.AmbientOcclusionRadius = 200.f;
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

	// Decay dash effect (fast burst)
	if (DashEffectIntensity > 0.f)
	{
		DashEffectIntensity = FMath::Max(0.f, DashEffectIntensity - DeltaTime * 5.f);
	}

	// Smooth grapple effect
	GrappleEffectAlpha = FMath::FInterpTo(GrappleEffectAlpha, TargetGrappleAlpha, DeltaTime, 8.f);

	// Decay scan pulse (medium burst)
	if (ScanPulseIntensity > 0.f)
	{
		ScanPulseIntensity = FMath::Max(0.f, ScanPulseIntensity - DeltaTime * 3.f);
	}

	// Decay shield flash (fast burst)
	if (ShieldFlashIntensity > 0.f)
	{
		ShieldFlashIntensity = FMath::Max(0.f, ShieldFlashIntensity - DeltaTime * 6.f);
	}

	// Decay weapon fire flash (very fast)
	if (WeaponFireFlash > 0.f)
	{
		WeaponFireFlash = FMath::Max(0.f, WeaponFireFlash - DeltaTime * 12.f);
	}

	// --- Combine all effects ---

	// Vignette: base + damage flash + low health + zone damage + heartbeat + abilities
	float TotalVignette = 0.3f;
	TotalVignette += DamageFlashIntensity * 0.5f;
	TotalVignette += LowHealthBlend * 0.3f;
	TotalVignette += ZoneDamageIntensity * 0.4f;
	TotalVignette += GrappleEffectAlpha * 0.6f;  // Tunnel vision during grapple
	TotalVignette += DashEffectIntensity * 0.4f;  // Brief vignette burst on dash

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

	// Scene color tint: damage + zone + abilities
	float DamageTint = DamageFlashIntensity * 0.3f;
	float ZoneTint = ZoneDamageIntensity * 0.4f;
	float CombinedTint = DamageTint + ZoneTint;

	// Ability tint contributions
	float GrappleTintG = GrappleEffectAlpha * 0.15f;  // Green shift
	float ScanTintB = ScanPulseIntensity * 0.2f;       // Blue shift
	float ShieldTintC = ShieldFlashIntensity * 0.25f;   // Cyan shift

	bool bHasTint = (CombinedTint > 0.01f || GrappleTintG > 0.01f ||
		ScanTintB > 0.01f || ShieldTintC > 0.01f);

	PostProcessComp->Settings.bOverride_SceneColorTint = bHasTint;
	if (bHasTint)
	{
		PostProcessComp->Settings.SceneColorTint = FLinearColor(
			1.f + CombinedTint - ScanTintB * 0.3f,
			1.f - CombinedTint * 0.5f + GrappleTintG + ShieldTintC * 0.5f,
			1.f - CombinedTint * 0.5f + ScanTintB + ShieldTintC, 1.f);
	}

	// Low health: desaturation + pulsing vignette
	float TotalDesat = 0.f;
	if (LowHealthBlend > 0.01f)
	{
		TotalDesat += LowHealthBlend * 0.4f;

		if (LowHealthBlend > 0.5f)
		{
			float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f));
			PostProcessComp->Settings.VignetteIntensity += Pulse * LowHealthBlend * 0.2f;
		}
	}
	// Dash brief desaturation for "speed blur" feel
	TotalDesat += DashEffectIntensity * 0.25f;

	PostProcessComp->Settings.bOverride_ColorSaturation = (TotalDesat > 0.01f);
	if (TotalDesat > 0.01f)
	{
		float Desat = 1.f - TotalDesat;
		PostProcessComp->Settings.ColorSaturation = FVector4(Desat, Desat, Desat, 1.f);
	}

	// Chromatic aberration: kill + zone + scan + dash
	float TotalFringe = KillEffectIntensity * 3.f + ZoneDamageIntensity * 2.f
		+ ScanPulseIntensity * 2.5f + DashEffectIntensity * 1.5f;
	PostProcessComp->Settings.SceneFringeIntensity = TotalFringe;

	// Speed boost + dash: motion blur + bloom increase
	float TotalMotionBlur = SpeedBoostAlpha + DashEffectIntensity * 0.8f;
	PostProcessComp->Settings.bOverride_MotionBlurAmount = (TotalMotionBlur > 0.01f);
	if (TotalMotionBlur > 0.01f)
	{
		PostProcessComp->Settings.MotionBlurAmount = TotalMotionBlur * 0.6f;
	}
	PostProcessComp->Settings.BloomIntensity = 0.7f + SpeedBoostAlpha * 0.4f
		+ ShieldFlashIntensity * 0.6f + ScanPulseIntensity * 0.3f
		+ WeaponFireFlash * 1.5f;

	// Weapon fire: brief auto-exposure kick for visible muzzle bloom
	PostProcessComp->Settings.AutoExposureBias = WeaponFireFlash * 0.4f;
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

void AExoPostProcess::TriggerDashEffect()
{
	DashEffectIntensity = 1.f;
}

void AExoPostProcess::ApplyGrappleEffect(float Alpha)
{
	TargetGrappleAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
}

void AExoPostProcess::TriggerScanPulse()
{
	ScanPulseIntensity = 1.f;
}

void AExoPostProcess::TriggerShieldFlash()
{
	ShieldFlashIntensity = 1.f;
}

void AExoPostProcess::TriggerWeaponFireFlash(float Intensity)
{
	WeaponFireFlash = FMath::Max(WeaponFireFlash, FMath::Clamp(Intensity, 0.f, 1.f));
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
