#include "Map/ExoHazardZone.h"
#include "Player/ExoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Visual/ExoMaterialFactory.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoHazardZone::AExoHazardZone()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Ground disk — flat cylinder showing hazard area
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	if (CylFinder.Succeeded())
	{
		GroundDisk = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundDisk"));
		GroundDisk->SetupAttachment(RootComponent);
		GroundDisk->SetStaticMesh(CylFinder.Object);
		GroundDisk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GroundDisk->CastShadow = false;

		// Boundary ring — thin cylinder at edge
		BoundaryRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoundaryRing"));
		BoundaryRing->SetupAttachment(RootComponent);
		BoundaryRing->SetStaticMesh(CylFinder.Object);
		BoundaryRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BoundaryRing->CastShadow = false;
	}

	// Ambient glow light at center
	AmbientGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("AmbientGlow"));
	AmbientGlow->SetupAttachment(RootComponent);
	AmbientGlow->SetRelativeLocation(FVector(0.f, 0.f, 500.f));
	AmbientGlow->SetIntensity(8000.f);
	AmbientGlow->SetAttenuationRadius(4000.f);
	AmbientGlow->CastShadows = false;
}

void AExoHazardZone::BeginPlay()
{
	Super::BeginPlay();

	FLinearColor Col = GetHazardColor();

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	// Scale ground disk to hazard radius
	if (GroundDisk)
	{
		float DiskScale = HazardRadius / 50.f; // Cylinder is 100 units diameter
		GroundDisk->SetRelativeScale3D(FVector(DiskScale, DiskScale, 0.02f));
		GroundDisk->SetRelativeLocation(FVector(0.f, 0.f, 5.f)); // Just above ground

		DiskMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!DiskMat) { return; }
		DiskMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * 1.5f, Col.G * 1.5f, Col.B * 1.5f));
		GroundDisk->SetMaterial(0, DiskMat);
	}

	// Boundary ring — slightly larger, thin
	if (BoundaryRing)
	{
		float RingOuter = HazardRadius / 50.f;
		float RingInner = (HazardRadius - 100.f) / 50.f;
		// Use outer scale with thin height for a border effect
		BoundaryRing->SetRelativeScale3D(FVector(RingOuter, RingOuter, 0.5f));
		BoundaryRing->SetRelativeLocation(FVector(0.f, 0.f, 25.f));

		RingMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!RingMat) { return; }
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * 5.f, Col.G * 5.f, Col.B * 5.f));
		BoundaryRing->SetMaterial(0, RingMat);
	}

	// Light color
	if (AmbientGlow)
	{
		AmbientGlow->SetLightColor(Col);
		AmbientGlow->SetAttenuationRadius(HazardRadius * 0.8f);
	}

	UE_LOG(LogExoRift, Log, TEXT("HazardZone '%s' initialized — radius %.0f, type %d"),
		*HazardName, HazardRadius, static_cast<int32>(HazardType));
}

void AExoHazardZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;

	if (bEnabled)
	{
		ApplyHazardDamage(DeltaTime);
		UpdateVFX(DeltaTime);
		UpdateHazardAudio(DeltaTime);
	}
	else
	{
		// Zone disabled — reset audio state
		bPlayerInsideZone = false;
		HumTimer = 0.f;
	}
}

void AExoHazardZone::SetHazardEnabled(bool bEnable)
{
	bEnabled = bEnable;
	if (GroundDisk) GroundDisk->SetVisibility(bEnable);
	if (BoundaryRing) BoundaryRing->SetVisibility(bEnable);
	if (AmbientGlow) AmbientGlow->SetVisibility(bEnable);

	UE_LOG(LogExoRift, Log, TEXT("Hazard '%s' %s"), *HazardName,
		bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AExoHazardZone::ApplyHazardDamage(float DeltaTime)
{
	if (!HasAuthority()) return;

	const FVector Origin = GetActorLocation();
	const float RadiusSq = HazardRadius * HazardRadius;
	const float FrameDamage = DamagePerSecond * DeltaTime;

	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive()) continue;

		float DistSq = FVector::DistSquared(Origin, Char->GetActorLocation());
		if (DistSq > RadiusSq) continue;

		FDamageEvent DamageEvent;
		Char->TakeDamage(FrameDamage, DamageEvent, nullptr, this);
	}
}

void AExoHazardZone::UpdateVFX(float DeltaTime)
{
	FLinearColor Col = GetHazardColor();
	float T = Age;

	// Ground disk: dual-frequency pulse for organic breathing feel
	if (DiskMat)
	{
		float SlowBreath = FMath::Sin(T * 1.2f);           // Slow breathing
		float FastFlutter = FMath::Sin(T * 4.7f) * 0.15f;  // Fast flutter overlay
		float Pulse = 0.4f + 0.35f * SlowBreath + FastFlutter;
		Pulse = FMath::Clamp(Pulse, 0.08f, 0.9f);
		DiskMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * Pulse, Col.G * Pulse, Col.B * Pulse));
	}

	// Boundary ring: heartbeat-style sharp pulse (pow of abs-sine)
	if (RingMat)
	{
		// Primary heartbeat: sharp spike every ~2 seconds
		float HeartPhase = FMath::Fmod(T * 0.55f, 1.f);
		float Beat1 = FMath::Pow(FMath::Max(FMath::Sin(HeartPhase * PI), 0.f), 4.f);
		// Secondary beat shortly after (double-tap heartbeat)
		float Beat2Phase = FMath::Fmod(T * 0.55f - 0.12f, 1.f);
		float Beat2 = FMath::Pow(FMath::Max(FMath::Sin(Beat2Phase * PI), 0.f), 6.f) * 0.6f;
		// Continuous low glow + heartbeat spikes
		float RingPulse = 1.2f + 4.f * Beat1 + 2.5f * Beat2;
		// Add high-freq shimmer on top
		RingPulse += 0.3f * FMath::Sin(T * 11.f);
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * RingPulse, Col.G * RingPulse, Col.B * RingPulse));
	}

	// Light: complex multi-frequency flicker with occasional surges
	if (AmbientGlow)
	{
		float Base = BaseLightIntensity * 0.4f;     // 12000
		float Wave1 = FMath::Sin(T * 1.8f);         // Slow swell
		float Wave2 = FMath::Sin(T * 5.3f);         // Medium flicker
		float Wave3 = FMath::Sin(T * 13.7f);        // Fast jitter
		// Occasional intensity surge (every ~5 seconds)
		float SurgePhase = FMath::Fmod(T * 0.2f, 1.f);
		float Surge = FMath::Pow(FMath::Max(FMath::Sin(SurgePhase * PI), 0.f), 8.f);
		float Intensity = Base
			+ 6000.f * Wave1
			+ 3000.f * Wave2
			+ 1000.f * Wave3
			+ 15000.f * Surge;
		Intensity = FMath::Max(Intensity, 2000.f);
		AmbientGlow->SetIntensity(Intensity);

		// Subtle color temperature shift during surges
		FLinearColor LightCol = Col;
		LightCol = FMath::Lerp(LightCol,
			FLinearColor(1.f, 1.f, 1.f), Surge * 0.3f);
		AmbientGlow->SetLightColor(LightCol);
	}
}

void AExoHazardZone::UpdateHazardAudio(float DeltaTime)
{
	// Check if any local player is inside the hazard zone
	const FVector Origin = GetActorLocation();
	const float RadiusSq = HazardRadius * HazardRadius;
	bool bAnyPlayerInside = false;

	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive()) continue;
		if (!Char->IsLocallyControlled()) continue;

		float DistSq = FVector::DistSquared(Origin, Char->GetActorLocation());
		if (DistSq <= RadiusSq)
		{
			bAnyPlayerInside = true;
			break;
		}
	}

	// Lazy-load the drone sound reference
	if (!HumSound)
	{
		HumSound = LoadObject<USoundBase>(nullptr,
			TEXT("/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02"));
	}

	if (bAnyPlayerInside && !bPlayerInsideZone)
	{
		// Player just entered — play a warning thud
		bPlayerInsideZone = true;
		HumTimer = 0.f;
		if (HumSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), HumSound, Origin, 0.2f, 0.3f);
		}
	}
	else if (!bAnyPlayerInside && bPlayerInsideZone)
	{
		bPlayerInsideZone = false;
		HumTimer = 0.f;
	}

	// Periodic ambient drone while player is inside
	if (bPlayerInsideZone && HumSound)
	{
		HumTimer += DeltaTime;
		if (HumTimer >= 1.6f)
		{
			HumTimer -= 1.6f;
			// Very low pitch = ominous hum/drone
			float Pitch = 0.18f + 0.04f * FMath::Sin(Age * 0.7f);
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), HumSound, Origin, 0.1f, Pitch);
			// Layer a second hit offset for thickness
			FTimerHandle Handle;
			float LayerPitch = Pitch + 0.05f;
			GetWorld()->GetTimerManager().SetTimer(Handle,
				[this, Origin, LayerPitch]()
				{
					if (HumSound)
					{
						UGameplayStatics::PlaySoundAtLocation(
							GetWorld(), HumSound, Origin, 0.07f, LayerPitch);
					}
				}, 0.2f, false);
		}
	}
}

FLinearColor AExoHazardZone::GetHazardColor() const
{
	switch (HazardType)
	{
	case EHazardType::Radiation: return FLinearColor(0.2f, 1.f, 0.1f);   // Green
	case EHazardType::Fire:      return FLinearColor(1.f, 0.4f, 0.05f);  // Orange
	case EHazardType::Electric:  return FLinearColor(0.3f, 0.5f, 1.f);   // Blue
	case EHazardType::Toxic:     return FLinearColor(0.6f, 0.1f, 0.8f);  // Purple
	default:                     return FLinearColor(1.f, 0.3f, 0.3f);
	}
}
