#include "Visual/ExoEnvironmentAnimator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"

AExoEnvironmentAnimator::AExoEnvironmentAnimator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.033f; // ~30Hz is enough for ambient
}

void AExoEnvironmentAnimator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float Time = GetWorld()->GetTimeSeconds();

	// --- Pylon rings: slow rotation + brightness pulse ---
	for (FPylonEntry& P : Pylons)
	{
		if (!P.Ring.IsValid()) continue;

		// Rotate the ring slowly
		FRotator Rot = P.Ring->GetRelativeRotation();
		Rot.Yaw += 45.f * DeltaTime; // 45 deg/sec
		P.Ring->SetRelativeRotation(Rot);

		// Pulse brightness
		float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 2.5f + P.Phase);
		if (P.Light.IsValid())
		{
			P.Light->SetIntensity(8000.f * Pulse);
		}
	}

	// --- Console screens: random flicker ---
	for (FConsoleEntry& C : Consoles)
	{
		C.FlickerTimer -= DeltaTime;
		if (C.FlickerTimer <= 0.f)
		{
			// Random flicker interval between 0.5s and 4s
			C.FlickerTimer = FMath::RandRange(0.5f, 4.f);

			if (C.Mat.IsValid())
			{
				// Brief dim-to-bright flash
				float Flicker = FMath::RandRange(0.6f, 1.2f);
				C.Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.1f * Flicker * 8.f, 0.4f * Flicker * 8.f, 0.2f * Flicker * 8.f));
			}
			if (C.Light.IsValid())
			{
				float LightFlicker = FMath::RandRange(0.5f, 1.3f);
				C.Light->SetIntensity(C.BaseIntensity * LightFlicker);
			}
		}
	}

	// --- Window glow: subtle slow breathing ---
	for (FWindowEntry& W : Windows)
	{
		if (!W.Mat.IsValid()) continue;

		float Breath = 0.8f + 0.2f * FMath::Sin(Time * 0.8f + W.Phase);
		// Warm interior light — subtle variation
		W.Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.8f * Breath * 5.f, 0.6f * Breath * 5.f, 0.3f * Breath * 5.f));
	}

	// --- Holograms: bob up/down + rotate + flicker ---
	for (FHologramEntry& H : Holograms)
	{
		if (!H.Mesh.IsValid()) continue;

		// Slow rotation
		FRotator Rot = H.Mesh->GetRelativeRotation();
		Rot.Yaw += 30.f * DeltaTime;
		H.Mesh->SetRelativeRotation(Rot);

		// Bob up and down — absolute offset to prevent drift
		FVector Loc = H.Mesh->GetRelativeLocation();
		Loc.Z = H.BaseZ + FMath::Sin(Time * 1.5f + H.Phase) * 15.f;
		H.Mesh->SetRelativeLocation(Loc);

		// Light pulse
		if (H.Light.IsValid())
		{
			float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 3.f + H.Phase);
			H.Light->SetIntensity(15000.f * Pulse);
		}
	}
}

void AExoEnvironmentAnimator::RegisterPylonRing(UStaticMeshComponent* Ring,
	UPointLightComponent* Light)
{
	FPylonEntry Entry;
	Entry.Ring = Ring;
	Entry.Light = Light;
	Entry.Phase = FMath::RandRange(0.f, PI * 2.f);
	Pylons.Add(Entry);
}

void AExoEnvironmentAnimator::RegisterConsoleScreen(UMaterialInstanceDynamic* ScreenMat,
	UPointLightComponent* Light)
{
	FConsoleEntry Entry;
	Entry.Mat = ScreenMat;
	Entry.Light = Light;
	Entry.FlickerTimer = FMath::RandRange(0.f, 3.f);
	Entry.BaseIntensity = Light ? Light->Intensity : 1800.f;
	Consoles.Add(Entry);
}

void AExoEnvironmentAnimator::RegisterWindowGlow(UMaterialInstanceDynamic* WinMat, float Phase)
{
	FWindowEntry Entry;
	Entry.Mat = WinMat;
	Entry.Phase = Phase;
	Windows.Add(Entry);
}

void AExoEnvironmentAnimator::RegisterHologram(UStaticMeshComponent* Mesh,
	UPointLightComponent* Light)
{
	FHologramEntry Entry;
	Entry.Mesh = Mesh;
	Entry.Light = Light;
	Entry.Phase = FMath::RandRange(0.f, PI * 2.f);
	Entry.BaseZ = Mesh ? Mesh->GetRelativeLocation().Z : 0.f;
	Holograms.Add(Entry);
}

AExoEnvironmentAnimator* AExoEnvironmentAnimator::Get(UWorld* World)
{
	if (!World) return nullptr;
	for (TActorIterator<AExoEnvironmentAnimator> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
