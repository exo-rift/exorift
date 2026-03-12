#include "Visual/ExoPostProcess.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoPostProcess::AExoPostProcess()
{
	PrimaryActorTick.bCanEverTick = true;

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	RootComponent = PostProcessComp;
	PostProcessComp->bUnbound = true; // Affects entire scene
	PostProcessComp->Priority = 1.f;

	// Base settings for realistic sci-fi look
	PostProcessComp->Settings.bOverride_BloomIntensity = true;
	PostProcessComp->Settings.BloomIntensity = 0.7f; // Moderate bloom — bright emissives glow, surfaces don't

	PostProcessComp->Settings.bOverride_BloomThreshold = true;
	PostProcessComp->Settings.BloomThreshold = 1.5f; // Only truly bright things bloom

	// NOTE: Do NOT override AutoExposureBias here — ExoLevelBuilder sets it to 1.2
	// and this PP (priority 1) would clobber that baseline. Only override during effects.

	PostProcessComp->Settings.bOverride_VignetteIntensity = true;
	PostProcessComp->Settings.VignetteIntensity = 0.3f;

	PostProcessComp->Settings.bOverride_FilmGrainIntensity = true;
	PostProcessComp->Settings.FilmGrainIntensity = 0.015f;

	// Cool shadow tint — blue-shifted shadows for sci-fi atmosphere
	PostProcessComp->Settings.bOverride_ColorContrastShadows = true;
	PostProcessComp->Settings.ColorContrastShadows = FVector4(1.08f, 1.08f, 1.18f, 1.f);

	PostProcessComp->Settings.bOverride_ColorGammaShadows = true;
	PostProcessComp->Settings.ColorGammaShadows = FVector4(0.96f, 0.96f, 1.06f, 1.f);

	// Warm highlights for contrast against cool shadows
	PostProcessComp->Settings.bOverride_ColorGainHighlights = true;
	PostProcessComp->Settings.ColorGainHighlights = FVector4(1.04f, 1.01f, 0.95f, 1.f);

	// Deeper midtone contrast for cinematic feel
	PostProcessComp->Settings.bOverride_ColorContrastMidtones = true;
	PostProcessComp->Settings.ColorContrastMidtones = FVector4(1.06f, 1.06f, 1.08f, 1.f);

	PostProcessComp->Settings.bOverride_SceneFringeIntensity = true;
	PostProcessComp->Settings.SceneFringeIntensity = 0.f;

	// Ambient occlusion for depth in enclosed spaces
	PostProcessComp->Settings.bOverride_AmbientOcclusionIntensity = true;
	PostProcessComp->Settings.AmbientOcclusionIntensity = 0.6f;
	PostProcessComp->Settings.bOverride_AmbientOcclusionRadius = true;
	PostProcessComp->Settings.AmbientOcclusionRadius = 250.f;
}

void AExoPostProcess::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Decay / interpolate effect intensities
	if (DamageFlashIntensity > 0.f)
		DamageFlashIntensity = FMath::Max(0.f, DamageFlashIntensity - DamageFlashDecayRate * DeltaTime);
	LowHealthBlend = FMath::FInterpTo(LowHealthBlend, TargetLowHealthBlend, DeltaTime, 3.f);
	if (KillEffectIntensity > 0.f)
		KillEffectIntensity = FMath::Max(0.f, KillEffectIntensity - KillEffectDecayRate * DeltaTime);
	ZoneDamageIntensity = FMath::FInterpTo(ZoneDamageIntensity, TargetZoneDamageIntensity, DeltaTime, 5.f);
	LowHealthPulseBlend = FMath::FInterpTo(LowHealthPulseBlend, TargetLowHealthPulseBlend, DeltaTime, 4.f);
	SpeedBoostAlpha = FMath::FInterpTo(SpeedBoostAlpha, TargetSpeedBoostAlpha, DeltaTime, 6.f);
	if (DashEffectIntensity > 0.f)
		DashEffectIntensity = FMath::Max(0.f, DashEffectIntensity - DeltaTime * 5.f);
	GrappleEffectAlpha = FMath::FInterpTo(GrappleEffectAlpha, TargetGrappleAlpha, DeltaTime, 8.f);
	if (ScanPulseIntensity > 0.f)
		ScanPulseIntensity = FMath::Max(0.f, ScanPulseIntensity - DeltaTime * 3.f);
	if (ShieldFlashIntensity > 0.f)
		ShieldFlashIntensity = FMath::Max(0.f, ShieldFlashIntensity - DeltaTime * 6.f);
	if (WeaponFireFlash > 0.f)
		WeaponFireFlash = FMath::Max(0.f, WeaponFireFlash - DeltaTime * 12.f);
	if (HeadshotKillIntensity > 0.f)
		HeadshotKillIntensity = FMath::Max(0.f, HeadshotKillIntensity - DeltaTime * 3.f);
	if (EnergyPickupFlash > 0.f)
		EnergyPickupFlash = FMath::Max(0.f, EnergyPickupFlash - DeltaTime * 4.f);
	if (DeathEffectIntensity > 0.f)
		DeathEffectIntensity = FMath::Max(0.f, DeathEffectIntensity - DeltaTime * 0.4f);

	// Hit pause recovery — restore timescale after headshot pause
	if (HitPauseRemaining > 0.f)
	{
		float RealDelta = DeltaTime / FMath::Max(UGameplayStatics::GetGlobalTimeDilation(GetWorld()), 0.01f);
		HitPauseRemaining -= RealDelta;
		if (HitPauseRemaining <= 0.f)
		{
			HitPauseRemaining = 0.f;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), HitPauseOriginalDilation);
		}
	}

	// --- Combine all effects ---

	// Vignette: base + damage flash + low health + zone damage + heartbeat + abilities + death
	float TotalVignette = 0.3f;
	TotalVignette += DamageFlashIntensity * 0.5f;
	TotalVignette += LowHealthBlend * 0.3f;
	TotalVignette += ZoneDamageIntensity * 0.4f;
	TotalVignette += GrappleEffectAlpha * 0.6f;  // Tunnel vision during grapple
	TotalVignette += DashEffectIntensity * 0.4f;  // Brief vignette burst on dash
	TotalVignette += SpeedBoostAlpha * 0.15f;      // Subtle edge darkening while sprinting
	TotalVignette += DeathEffectIntensity * 0.7f;  // Heavy vignette on death

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

	// Headshot kill golden flash
	float HeadshotGold = HeadshotKillIntensity * 0.15f;

	// Energy pickup white-blue flash
	float EnergyTintW = EnergyPickupFlash * 0.12f;  // White lift
	float EnergyTintB = EnergyPickupFlash * 0.18f;  // Blue shift

	// Death cold blue tint
	float DeathTintB = DeathEffectIntensity * 0.2f;   // Blue shift
	float DeathTintR = DeathEffectIntensity * 0.12f;   // Red drain

	bool bHasTint = (CombinedTint > 0.01f || GrappleTintG > 0.01f ||
		ScanTintB > 0.01f || ShieldTintC > 0.01f || HeadshotGold > 0.01f ||
		EnergyPickupFlash > 0.01f || DeathEffectIntensity > 0.01f);

	PostProcessComp->Settings.bOverride_SceneColorTint = bHasTint;
	if (bHasTint)
	{
		PostProcessComp->Settings.SceneColorTint = FLinearColor(
			1.f + CombinedTint - ScanTintB * 0.3f + HeadshotGold + EnergyTintW - DeathTintR,
			1.f - CombinedTint * 0.5f + GrappleTintG + ShieldTintC * 0.5f + HeadshotGold * 0.8f + EnergyTintW - DeathTintR * 0.5f,
			1.f - CombinedTint * 0.5f + ScanTintB + ShieldTintC - HeadshotGold * 0.3f + EnergyTintW + EnergyTintB + DeathTintB, 1.f);
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
	// Death heavy desaturation — world drains of color
	TotalDesat += DeathEffectIntensity * 0.65f;

	PostProcessComp->Settings.bOverride_ColorSaturation = (TotalDesat > 0.01f);
	if (TotalDesat > 0.01f)
	{
		float Desat = 1.f - TotalDesat;
		PostProcessComp->Settings.ColorSaturation = FVector4(Desat, Desat, Desat, 1.f);
	}

	// Chromatic aberration: kill + headshot kill + zone + scan + dash + sprint + death
	float TotalFringe = KillEffectIntensity * 3.f + HeadshotKillIntensity * 5.f
		+ ZoneDamageIntensity * 2.f + ScanPulseIntensity * 2.5f + DashEffectIntensity * 1.5f
		+ SpeedBoostAlpha * 0.8f  // Subtle chromatic aberration while sprinting
		+ DeathEffectIntensity * 6.f;  // Strong aberration on death
	PostProcessComp->Settings.SceneFringeIntensity = TotalFringe;

	// Speed boost + dash: motion blur + bloom increase
	// Sprint caps at 0.3 for a subtle speed feel; dash adds a stronger burst
	float SprintMotionBlur = SpeedBoostAlpha * 0.3f;
	float DashMotionBlur = DashEffectIntensity * 0.5f;
	float TotalMotionBlur = SprintMotionBlur + DashMotionBlur;
	PostProcessComp->Settings.bOverride_MotionBlurAmount = (TotalMotionBlur > 0.01f);
	if (TotalMotionBlur > 0.01f)
	{
		PostProcessComp->Settings.MotionBlurAmount = TotalMotionBlur;
	}

	// Sprint motion blur max target — also increase MotionBlurMax so UE doesn't clamp it
	PostProcessComp->Settings.bOverride_MotionBlurMax = (TotalMotionBlur > 0.01f);
	if (TotalMotionBlur > 0.01f)
	{
		PostProcessComp->Settings.MotionBlurMax = FMath::Lerp(5.f, 8.f, SpeedBoostAlpha);
	}
	PostProcessComp->Settings.BloomIntensity = 0.7f + SpeedBoostAlpha * 0.3f
		+ ShieldFlashIntensity * 0.5f + ScanPulseIntensity * 0.3f
		+ WeaponFireFlash * 1.2f + HeadshotKillIntensity * 0.8f
		+ EnergyPickupFlash * 0.6f
		+ DeathEffectIntensity * 1.0f;

	// Weapon fire: brief auto-exposure kick for visible muzzle bloom
	PostProcessComp->Settings.AutoExposureBias = WeaponFireFlash * 0.4f;

	// Endgame cinematic blend (slow ramp to dramatic look)
	EndgameBlend = FMath::FInterpTo(EndgameBlend, EndgameTarget, DeltaTime, 0.8f);
	if (EndgameBlend > 0.01f)
	{
		// Desaturation for dramatic effect (less for victory, more for defeat)
		float DesatAmount = bEndgameVictory ? 0.15f : 0.5f;
		float ExistingDesat = PostProcessComp->Settings.ColorSaturation.X;
		float FinalDesat = FMath::Lerp(ExistingDesat, 1.f - DesatAmount, EndgameBlend);
		PostProcessComp->Settings.bOverride_ColorSaturation = true;
		PostProcessComp->Settings.ColorSaturation = FVector4(FinalDesat, FinalDesat, FinalDesat, 1.f);

		// Bloom ramp for cinematic glow
		PostProcessComp->Settings.BloomIntensity += EndgameBlend * 1.5f;

		// Vignette increase
		PostProcessComp->Settings.VignetteIntensity += EndgameBlend * 0.3f;

		// Victory: golden tint. Defeat: cool blue tint
		PostProcessComp->Settings.bOverride_SceneColorTint = true;
		if (bEndgameVictory)
		{
			PostProcessComp->Settings.SceneColorTint = FLinearColor(
				1.f + EndgameBlend * 0.1f,
				1.f + EndgameBlend * 0.05f,
				1.f - EndgameBlend * 0.05f, 1.f);
		}
		else
		{
			PostProcessComp->Settings.SceneColorTint = FLinearColor(
				1.f - EndgameBlend * 0.08f,
				1.f - EndgameBlend * 0.02f,
				1.f + EndgameBlend * 0.1f, 1.f);
		}
	}
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

void AExoPostProcess::TriggerHeadshotPause()
{
	// 60ms timescale dip to 0.15 for visceral headshot feedback
	HitPauseOriginalDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.15f);
	HitPauseRemaining = 0.06f;
}

void AExoPostProcess::TriggerHeadshotKillEffect()
{
	KillEffectIntensity = 1.f;
	HeadshotKillIntensity = 1.f;
	TriggerHeadshotPause();
}

void AExoPostProcess::TriggerEnergyPickupFlash()
{
	EnergyPickupFlash = 1.f;
}

void AExoPostProcess::TriggerDeathEffect()
{
	DeathEffectIntensity = 1.f;
}

void AExoPostProcess::TriggerEndgameCinematic(bool bIsVictory)
{
	EndgameTarget = 1.f;
	bEndgameVictory = bIsVictory;
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
