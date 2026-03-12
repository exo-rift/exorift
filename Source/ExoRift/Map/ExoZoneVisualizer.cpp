#include "Map/ExoZoneVisualizer.h"
#include "Map/ExoZoneSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoZoneVisualizer::AExoZoneVisualizer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Cache engine meshes in constructor (ConstructorHelpers only work here)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	// Materials created at runtime via FExoMaterialFactory

	// Main zone wall — tall cylinder scaled to zone radius
	ZoneWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneWall"));
	RootComponent = ZoneWallMesh;
	if (CylinderMesh) ZoneWallMesh->SetStaticMesh(CylinderMesh);
	ZoneWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneWallMesh->CastShadow = false;

	// Target zone ring — shows where zone is shrinking to
	TargetRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetRing"));
	TargetRingMesh->SetupAttachment(nullptr); // Not attached to wall — independent position
	if (CylinderMesh) TargetRingMesh->SetStaticMesh(CylinderMesh);
	TargetRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetRingMesh->CastShadow = false;
	TargetRingMesh->SetVisibility(false);

	// Ground glow ring — thin bright ring at base of zone wall
	GroundGlowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundGlow"));
	GroundGlowMesh->SetupAttachment(nullptr);
	if (CylinderMesh) GroundGlowMesh->SetStaticMesh(CylinderMesh);
	GroundGlowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundGlowMesh->CastShadow = false;
}

void AExoZoneVisualizer::BeginPlay()
{
	Super::BeginPlay();

	// Zone wall & ground glow use additive blend so you can see through them
	UMaterialInterface* AdditiveMat = FExoMaterialFactory::GetEmissiveAdditive();
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	if (ZoneWallMesh && AdditiveMat)
	{
		ZoneMaterial = UMaterialInstanceDynamic::Create(AdditiveMat, this);
		if (!ZoneMaterial) { return; }
		// Subtle translucent glow — low emissive so it doesn't overwhelm the scene
		ZoneMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ZoneColor.R * 0.15f, ZoneColor.G * 0.15f, ZoneColor.B * 0.15f));
		ZoneWallMesh->SetMaterial(0, ZoneMaterial);
	}

	if (TargetRingMesh && AdditiveMat)
	{
		TargetMaterial = UMaterialInstanceDynamic::Create(AdditiveMat, this);
		if (!TargetMaterial) { return; }
		TargetRingMesh->SetMaterial(0, TargetMaterial);
	}

	if (GroundGlowMesh && AdditiveMat)
	{
		GlowMaterial = UMaterialInstanceDynamic::Create(AdditiveMat, this);
		if (!GlowMaterial) { return; }
		GroundGlowMesh->SetMaterial(0, GlowMaterial);
	}

	// Create lightning arc segments along the zone edge
	if (CubeMesh)
	{
		for (int32 i = 0; i < NumLightningArcs; i++)
		{
			UStaticMeshComponent* Arc = NewObject<UStaticMeshComponent>(this);
			Arc->SetStaticMesh(CubeMesh);
			Arc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Arc->CastShadow = false;
			Arc->RegisterComponent();

			UMaterialInstanceDynamic* ArcMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			if (!ArcMat) { continue; }
			ArcMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.5f, 1.f, 2.f));
			Arc->SetMaterial(0, ArcMat);

			LightningArcs.Add(Arc);
			LightningMats.Add(ArcMat);

			// Point light at each arc
			UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
			Light->SetIntensity(0.f);
			Light->SetAttenuationRadius(2000.f);
			Light->SetLightColor(FLinearColor(0.3f, 0.5f, 1.f));
			Light->CastShadows = false;
			Light->RegisterComponent();
			EdgeLights.Add(Light);
		}
	}
}

void AExoZoneVisualizer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ZoneSystem)
	{
		for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
		{
			ZoneSystem = *It;
			break;
		}
		if (!ZoneSystem) return;
	}

	UpdateZoneWall();
	UpdateTargetRing();
	UpdateGroundGlow();
	UpdateEdgeLightning();
}

void AExoZoneVisualizer::UpdateZoneWall()
{
	if (!ZoneSystem) return;

	float Radius = ZoneSystem->GetCurrentRadius();
	FVector2D Center = ZoneSystem->GetCurrentCenter();

	if (FMath::Abs(Radius - CachedRadius) < 10.f) return;
	CachedRadius = Radius;

	float RadiusScale = Radius / 50.f;
	float HeightScale = WallHeight / 100.f;

	SetActorLocation(FVector(Center.X, Center.Y, WallHeight * 0.5f));
	ZoneWallMesh->SetWorldScale3D(FVector(RadiusScale, RadiusScale, HeightScale));

	// Subtle emissive pulse — slightly brighter when shrinking
	if (ZoneMaterial)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float BaseRate = ZoneSystem->IsShrinking() ? 2.f : 0.8f;
		float PulseStrength = ZoneSystem->IsShrinking() ? 0.12f : 0.05f;
		float BaseMul = ZoneSystem->IsShrinking() ? 0.2f : 0.1f;
		float Pulse = BaseMul + PulseStrength * FMath::Abs(FMath::Sin(Time * BaseRate));
		ZoneMaterial->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ZoneColor.R * Pulse, ZoneColor.G * Pulse, ZoneColor.B * Pulse));
	}
}

void AExoZoneVisualizer::UpdateTargetRing()
{
	if (!ZoneSystem || !TargetRingMesh) return;

	bool bShrinking = ZoneSystem->IsShrinking();
	float TargetRadius = ZoneSystem->GetTargetRadius();
	float CurrentRadius = ZoneSystem->GetCurrentRadius();

	// Only show target ring when zone is about to shrink or currently shrinking
	bool bShowTarget = bShrinking || (ZoneSystem->IsInHoldPhase() &&
		FMath::Abs(TargetRadius - CurrentRadius) > 100.f);

	TargetRingMesh->SetVisibility(bShowTarget);
	if (!bShowTarget) return;

	FVector2D TargetCenter = ZoneSystem->GetTargetCenter();

	if (FMath::Abs(TargetRadius - CachedTargetRadius) > 10.f)
	{
		CachedTargetRadius = TargetRadius;
		float RadiusScale = TargetRadius / 50.f;
		float RingHeight = 200.f; // Thin ring, not full wall
		TargetRingMesh->SetWorldScale3D(FVector(RadiusScale, RadiusScale, RingHeight / 100.f));
	}

	TargetRingMesh->SetWorldLocation(FVector(TargetCenter.X, TargetCenter.Y, 100.f));

	// Subtle pulsing white-blue emissive
	if (TargetMaterial)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Pulse = 0.5f + 0.5f * FMath::Sin(Time * 2.5f);
		FLinearColor EmCol(0.1f * Pulse, 0.2f * Pulse, 0.4f * Pulse);
		TargetMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), EmCol);
	}
}

void AExoZoneVisualizer::UpdateGroundGlow()
{
	if (!ZoneSystem || !GroundGlowMesh) return;

	float Radius = ZoneSystem->GetCurrentRadius();
	FVector2D Center = ZoneSystem->GetCurrentCenter();

	// Ground glow is a very flat, slightly larger cylinder at ground level
	float GlowRadius = (Radius + 200.f) / 50.f;
	float GlowHeight = 0.1f; // Nearly flat
	GroundGlowMesh->SetWorldScale3D(FVector(GlowRadius, GlowRadius, GlowHeight));
	GroundGlowMesh->SetWorldLocation(FVector(Center.X, Center.Y, 5.f));

	if (GlowMaterial)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Pulse = 0.6f + 0.4f * FMath::Sin(Time * 1.2f);
		bool bShrinking = ZoneSystem->IsShrinking();

		// Subtle ground glow — brighter when shrinking, shifts toward red
		FLinearColor GlowCol;
		if (bShrinking)
		{
			GlowCol = FLinearColor(0.3f * Pulse, 0.08f * Pulse, 0.12f * Pulse);
		}
		else
		{
			GlowCol = FLinearColor(0.06f * Pulse, 0.12f * Pulse, 0.3f * Pulse);
		}
		GlowMaterial->SetVectorParameterValue(TEXT("EmissiveColor"), GlowCol);
	}
}

void AExoZoneVisualizer::UpdateEdgeLightning()
{
	if (!ZoneSystem || LightningArcs.Num() == 0) return;

	float Radius = ZoneSystem->GetCurrentRadius();
	FVector2D Center = ZoneSystem->GetCurrentCenter();
	float Time = GetWorld()->GetTimeSeconds();
	bool bShrinking = ZoneSystem->IsShrinking();

	for (int32 i = 0; i < NumLightningArcs; i++)
	{
		if (!LightningArcs.IsValidIndex(i) || !LightningArcs[i]) continue;

		// Each arc orbits the zone edge at different speed with jitter
		float BaseAngle = (2.f * PI * i) / NumLightningArcs;
		float Drift = FMath::Sin(Time * (1.3f + i * 0.2f)) * 0.3f;
		float Angle = BaseAngle + Time * 0.15f + Drift;

		float ArcX = Center.X + FMath::Cos(Angle) * Radius;
		float ArcY = Center.Y + FMath::Sin(Angle) * Radius;

		// Height jitter — arcs dance up and down the wall
		float HeightJitter = 500.f + 3000.f * FMath::Abs(
			FMath::Sin(Time * (3.f + i * 0.7f)));

		LightningArcs[i]->SetWorldLocation(FVector(ArcX, ArcY, HeightJitter));

		// Orient tangent to the circle
		FRotator ArcRot(0.f, FMath::RadiansToDegrees(Angle) + 90.f, 0.f);
		// Tilt for visual variety
		ArcRot.Roll = FMath::Sin(Time * (5.f + i)) * 25.f;
		LightningArcs[i]->SetWorldRotation(ArcRot);

		// Flickering scale — arcs randomly stretch and shrink
		float Flicker = FMath::Abs(FMath::Sin(Time * (15.f + i * 7.3f)));
		float LenScale = (bShrinking ? 12.f : 6.f) * (0.5f + Flicker);
		float ThickScale = 0.08f + 0.05f * Flicker;
		LightningArcs[i]->SetWorldScale3D(FVector(LenScale, ThickScale, ThickScale));

		// Emissive pulse — brighter during shrink, flickers rapidly
		if (LightningMats.IsValidIndex(i) && LightningMats[i])
		{
			float EmIntensity = bShrinking ? (1.5f + 2.f * Flicker) : (0.5f + 1.f * Flicker);
			FLinearColor EmCol(
				ZoneColor.R * EmIntensity,
				ZoneColor.G * EmIntensity,
				ZoneColor.B * EmIntensity * 1.5f);
			LightningMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), EmCol);
		}

		// Light follows the arc
		if (EdgeLights.IsValidIndex(i) && EdgeLights[i])
		{
			EdgeLights[i]->SetWorldLocation(FVector(ArcX, ArcY, HeightJitter));
			float LightPower = (bShrinking ? 3000.f : 1000.f) * Flicker;
			EdgeLights[i]->SetIntensity(LightPower);
		}
	}
}
