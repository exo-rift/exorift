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

	bool bDirty = false;

	// Decay damage flash
	if (DamageFlashIntensity > 0.f)
	{
		DamageFlashIntensity = FMath::Max(0.f, DamageFlashIntensity - DamageFlashDecayRate * DeltaTime);
		bDirty = true;
	}

	// Smooth low health blend
	LowHealthBlend = FMath::FInterpTo(LowHealthBlend, TargetLowHealthBlend, DeltaTime, 3.f);

	// Decay kill effect
	if (KillEffectIntensity > 0.f)
	{
		KillEffectIntensity = FMath::Max(0.f, KillEffectIntensity - KillEffectDecayRate * DeltaTime);
		bDirty = true;
	}

	// Apply combined effects
	float TotalVignette = 0.3f; // Base
	TotalVignette += DamageFlashIntensity * 0.5f;
	TotalVignette += LowHealthBlend * 0.3f;

	PostProcessComp->Settings.VignetteIntensity = TotalVignette;

	// Damage flash: warm tint
	float DamageTint = DamageFlashIntensity * 0.3f;
	PostProcessComp->Settings.bOverride_SceneColorTint = (DamageTint > 0.01f);
	if (DamageTint > 0.01f)
	{
		PostProcessComp->Settings.SceneColorTint = FLinearColor(1.f + DamageTint, 1.f - DamageTint * 0.5f, 1.f - DamageTint * 0.5f, 1.f);
	}

	// Low health: desaturation + pulsing vignette
	if (LowHealthBlend > 0.01f)
	{
		PostProcessComp->Settings.bOverride_ColorSaturation = true;
		float Desat = 1.f - LowHealthBlend * 0.4f;
		PostProcessComp->Settings.ColorSaturation = FVector4(Desat, Desat, Desat, 1.f);

		// Heartbeat pulse when very low
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

	// Kill chromatic aberration
	PostProcessComp->Settings.SceneFringeIntensity = KillEffectIntensity * 3.f;
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

AExoPostProcess* AExoPostProcess::Get(UWorld* World)
{
	if (!World) return nullptr;

	for (TActorIterator<AExoPostProcess> It(World); It; ++It)
	{
		return *It;
	}

	return World->SpawnActor<AExoPostProcess>();
}
