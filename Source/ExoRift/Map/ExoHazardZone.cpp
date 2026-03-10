#include "Map/ExoHazardZone.h"
#include "Player/ExoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/DamageEvents.h"
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
	AmbientGlow->SetIntensity(15000.f);
	AmbientGlow->SetAttenuationRadius(8000.f);
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
		DiskMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * 0.5f, Col.G * 0.5f, Col.B * 0.5f));
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
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * 2.f, Col.G * 2.f, Col.B * 2.f));
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

	// Pulsing ground disk emissive
	if (DiskMat)
	{
		float Pulse = 0.3f + 0.2f * FMath::Sin(T * 1.5f);
		DiskMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * Pulse, Col.G * Pulse, Col.B * Pulse));
	}

	// Boundary ring pulse — faster, brighter
	if (RingMat)
	{
		float RingPulse = 1.5f + 1.f * FMath::Sin(T * 3.f);
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Col.R * RingPulse, Col.G * RingPulse, Col.B * RingPulse));
	}

	// Light flicker
	if (AmbientGlow)
	{
		float Flicker = 12000.f + 5000.f * FMath::Sin(T * 2.f)
			+ 2000.f * FMath::Sin(T * 7.3f);
		AmbientGlow->SetIntensity(Flicker);
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
