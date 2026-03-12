// ExoDropPodVisuals.cpp — Pod mesh construction, thruster VFX, landing dust,
// contrail, scorch, and door steam effects
#include "Map/ExoDropPod.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoKillScorch.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

void AExoDropPod::BuildPodMesh()
{
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	auto MakeComp = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator) -> UStaticMeshComponent*
	{
		if (!Mesh) return nullptr;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = true;
		C->RegisterComponent();
		if (LitMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, this);
			if (!Mat) { return nullptr; }
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
			if (Lum > 0.15f)
			{
				Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(Color.R * 40.f, Color.G * 40.f, Color.B * 40.f));
				Mat->SetScalarParameterValue(TEXT("Metallic"), 0.4f);
				Mat->SetScalarParameterValue(TEXT("Roughness"), 0.15f);
			}
			else
			{
				Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
				Mat->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
				Mat->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
			}
			C->SetMaterial(0, Mat);
		}
		return C;
	};

	FLinearColor HullColor(0.06f, 0.07f, 0.09f);
	FLinearColor AccentColor(0.1f, 0.3f, 0.8f);
	FLinearColor ThrusterColor(0.15f, 0.15f, 0.2f);

	HullBody = MakeComp(CylinderMesh, FVector(0.f, 0.f, 0.f),
		FVector(1.2f, 1.2f, 2.f), HullColor);
	HullNose = MakeComp(ConeMesh ? ConeMesh : CylinderMesh, FVector(0.f, 0.f, 200.f),
		FVector(1.0f, 1.0f, 0.8f), HullColor);

	FinLeft = MakeComp(CubeMesh, FVector(0.f, -100.f, -80.f),
		FVector(0.05f, 0.6f, 0.3f), AccentColor, FRotator(0.f, 0.f, 15.f));
	FinRight = MakeComp(CubeMesh, FVector(0.f, 100.f, -80.f),
		FVector(0.05f, 0.6f, 0.3f), AccentColor, FRotator(0.f, 0.f, -15.f));

	MakeComp(CubeMesh, FVector(100.f, 0.f, -80.f),
		FVector(0.6f, 0.05f, 0.3f), AccentColor);
	MakeComp(CubeMesh, FVector(-100.f, 0.f, -80.f),
		FVector(0.6f, 0.05f, 0.3f), AccentColor);

	ThrusterCone = MakeComp(CylinderMesh, FVector(0.f, 0.f, -160.f),
		FVector(0.6f, 0.6f, 0.4f), ThrusterColor);

	// Blue accent stripes
	MakeComp(CubeMesh, FVector(0.f, 60.f, 40.f),
		FVector(0.02f, 0.02f, 1.2f), AccentColor);
	MakeComp(CubeMesh, FVector(0.f, -60.f, 40.f),
		FVector(0.02f, 0.02f, 1.2f), AccentColor);

	// Thruster flame
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	if (CylinderMesh)
	{
		ThrusterFlame = NewObject<UStaticMeshComponent>(this);
		ThrusterFlame->SetupAttachment(RootComponent);
		ThrusterFlame->SetStaticMesh(CylinderMesh);
		ThrusterFlame->SetRelativeLocation(FVector(0.f, 0.f, -250.f));
		ThrusterFlame->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.01f));
		ThrusterFlame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThrusterFlame->CastShadow = false;
		ThrusterFlame->RegisterComponent();
		FlameMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!FlameMat) { return; }
		FlameMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(11.f, 22.f, 56.f));
		ThrusterFlame->SetMaterial(0, FlameMat);
	}

	// Heat shield glow
	if (SphereMesh)
	{
		HeatShield = NewObject<UStaticMeshComponent>(this);
		HeatShield->SetupAttachment(RootComponent);
		HeatShield->SetStaticMesh(SphereMesh);
		HeatShield->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
		HeatShield->SetRelativeScale3D(FVector(2.5f, 2.5f, 3.5f));
		HeatShield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HeatShield->CastShadow = false;
		HeatShield->RegisterComponent();
		HeatMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!HeatMat) { return; }
		HeatMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(16.f, 4.5f, 0.9f));
		HeatShield->SetMaterial(0, HeatMat);

		HeatLight = NewObject<UPointLightComponent>(this);
		HeatLight->SetupAttachment(RootComponent);
		HeatLight->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
		HeatLight->SetIntensity(0.f);
		HeatLight->SetAttenuationRadius(2800.f);
		HeatLight->SetLightColor(FLinearColor(2.2f, 0.9f, 0.22f));
		HeatLight->CastShadows = false;
		HeatLight->RegisterComponent();
	}
}

void AExoDropPod::SpawnLandingDust()
{
	if (!CylinderMesh) return;

	// Expanding dust ring at ground level
	DustRing = NewObject<UStaticMeshComponent>(this);
	DustRing->SetStaticMesh(CylinderMesh);
	DustRing->SetWorldLocation(GetActorLocation() - FVector(0.f, 0.f, 150.f));
	DustRing->SetWorldScale3D(FVector(1.f, 1.f, 0.05f));
	DustRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DustRing->CastShadow = false;
	DustRing->RegisterComponent();
	UMaterialInterface* EmissiveDustMat = FExoMaterialFactory::GetEmissiveOpaque();
	DustMat = UMaterialInstanceDynamic::Create(EmissiveDustMat, this);
	if (!DustMat) { return; }
	DustMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.66f, 0.55f, 0.33f));
	DustRing->SetMaterial(0, DustMat);
	DustAge = 0.f;

	// Bright flash on impact
	ImpactFlash = NewObject<UPointLightComponent>(this);
	ImpactFlash->SetWorldLocation(GetActorLocation());
	ImpactFlash->SetIntensity(220000.f);
	ImpactFlash->SetAttenuationRadius(7000.f);
	ImpactFlash->SetLightColor(FLinearColor(1.8f, 2.0f, 2.2f));
	ImpactFlash->CastShadows = false;
	ImpactFlash->RegisterComponent();
}

void AExoDropPod::UpdateThrusterVFX(float DeltaTime, float BrakeAlpha)
{
	if (!ThrusterLight) return;

	float Time = GetWorld()->GetTimeSeconds();

	float TargetIntensity = 0.f;
	if (Phase == EDropPodPhase::FreeFall)
		TargetIntensity = 11000.f;
	else if (Phase == EDropPodPhase::Braking)
		TargetIntensity = FMath::Lerp(11000.f, 180000.f, BrakeAlpha);
	else if (Phase == EDropPodPhase::Landing)
		TargetIntensity = 180000.f;

	ThrusterLight->SetIntensity(FMath::FInterpTo(
		ThrusterLight->Intensity, TargetIntensity, DeltaTime, 4.f));

	FLinearColor ThrusterColor = FMath::Lerp(
		FLinearColor(0.66f, 1.1f, 2.2f),
		FLinearColor(1.8f, 2.0f, 2.2f),
		BrakeAlpha);
	ThrusterLight->SetLightColor(ThrusterColor);

	// Thruster flame
	if (ThrusterFlame)
	{
		float FlameLen = 0.f;
		float FlameWidth = 0.3f;
		if (Phase == EDropPodPhase::FreeFall)
		{
			FlameLen = 0.5f; FlameWidth = 0.15f;
		}
		else if (Phase == EDropPodPhase::Braking)
		{
			FlameLen = FMath::Lerp(0.5f, 4.f, BrakeAlpha);
			FlameWidth = FMath::Lerp(0.15f, 0.5f, BrakeAlpha);
		}
		else if (Phase == EDropPodPhase::Landing)
		{
			FlameLen = 4.f; FlameWidth = 0.5f;
		}

		float Flicker = 1.f + 0.15f * FMath::Sin(Time * 25.f)
			+ 0.1f * FMath::Sin(Time * 37.f);
		ThrusterFlame->SetRelativeScale3D(
			FVector(FlameWidth * Flicker, FlameWidth * Flicker, FlameLen));
		ThrusterFlame->SetRelativeLocation(
			FVector(0.f, 0.f, -200.f - FlameLen * 50.f));

		if (FlameMat)
		{
			FLinearColor FlameCol = FMath::Lerp(
				FLinearColor(5.5f, 16.f, 45.f),
				FLinearColor(40.f, 40.f, 56.f),
				BrakeAlpha);
			FlameMat->SetVectorParameterValue(TEXT("EmissiveColor"), FlameCol);
		}
	}

	// Heat shield
	if (HeatShield)
	{
		float HeatAlpha = 0.f;
		if (Phase == EDropPodPhase::FreeFall)
			HeatAlpha = 0.6f + 0.2f * FMath::Sin(Time * 3.f);
		else if (Phase == EDropPodPhase::Braking)
			HeatAlpha = FMath::Max(0.f, 0.6f * (1.f - BrakeAlpha));

		HeatShield->SetVisibility(HeatAlpha > 0.01f);
		if (HeatMat && HeatAlpha > 0.01f)
		{
			FLinearColor HeatCol(16.f * HeatAlpha, 4.5f * HeatAlpha, 0.9f * HeatAlpha);
			HeatMat->SetVectorParameterValue(TEXT("EmissiveColor"), HeatCol);
		}
		if (HeatLight)
		{
			HeatLight->SetIntensity(HeatAlpha * 160000.f);
		}
	}
}

// --- Contrail: glowing energy trail left behind during descent ---

void AExoDropPod::BuildContrail()
{
	if (!CylinderMesh) return;

	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!AddMat) return;

	bContrailBuilt = true;

	for (int32 i = 0; i < CONTRAIL_SEGMENTS; i++)
	{
		UStaticMeshComponent* Seg = NewObject<UStaticMeshComponent>(this);
		Seg->SetStaticMesh(CylinderMesh);
		Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Seg->CastShadow = false;
		Seg->SetWorldScale3D(FVector(0.08f, 0.08f, 0.01f));
		Seg->SetVisibility(false); // Hidden until trail starts
		Seg->RegisterComponent();

		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(AddMat, this);
		if (!Mat) { continue; }
		// Bright cyan-blue sci-fi contrail with white core
		float Brightness = 1.f - (static_cast<float>(i) / CONTRAIL_SEGMENTS) * 0.5f;
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(8.f * Brightness, 18.f * Brightness, 50.f * Brightness));
		Seg->SetMaterial(0, Mat);

		ContrailSegments[i] = Seg;
		ContrailMats[i] = Mat;
	}

	// Trailing light that follows the pod
	ContrailLight = NewObject<UPointLightComponent>(this);
	ContrailLight->SetupAttachment(RootComponent);
	ContrailLight->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	ContrailLight->SetIntensity(60000.f);
	ContrailLight->SetAttenuationRadius(3500.f);
	ContrailLight->SetLightColor(FLinearColor(0.3f, 0.6f, 1.4f));
	ContrailLight->CastShadows = false;
	ContrailLight->RegisterComponent();
}

void AExoDropPod::UpdateContrail(float DeltaTime)
{
	if (!bContrailBuilt) return;

	FVector CurrentLoc = GetActorLocation();
	float MoveDist = FVector::Distance(CurrentLoc, PrevPodLocation);

	// Shift segments (oldest = highest index), place newest at pod tail
	for (int32 i = CONTRAIL_SEGMENTS - 1; i > 0; i--)
	{
		if (!ContrailSegments[i] || !ContrailSegments[i - 1]) continue;
		ContrailSegments[i]->SetWorldLocation(ContrailSegments[i - 1]->GetComponentLocation());
		ContrailSegments[i]->SetWorldRotation(ContrailSegments[i - 1]->GetComponentRotation());
		ContrailSegments[i]->SetWorldScale3D(ContrailSegments[i - 1]->GetComponentScale());
	}

	if (ContrailSegments[0] && MoveDist > 10.f)
	{
		FVector TrailStart = PrevPodLocation + FVector(0.f, 0.f, -160.f);
		FVector TrailEnd = CurrentLoc + FVector(0.f, 0.f, -160.f);
		FVector Dir = (TrailEnd - TrailStart);
		float SegLen = Dir.Size();
		Dir.Normalize();

		FRotator SegRot = Dir.Rotation();
		SegRot.Pitch += 90.f;

		FVector Center = (TrailStart + TrailEnd) * 0.5f;
		float LenScale = FMath::Clamp(SegLen / 100.f, 0.01f, 6.f);
		float WidthScale = (Phase == EDropPodPhase::FreeFall) ? 0.12f : 0.06f;

		ContrailSegments[0]->SetWorldLocation(Center);
		ContrailSegments[0]->SetWorldRotation(SegRot);
		ContrailSegments[0]->SetWorldScale3D(FVector(WidthScale, WidthScale, LenScale));
		ContrailSegments[0]->SetVisibility(true);
	}

	// Fade older segments
	for (int32 i = 0; i < CONTRAIL_SEGMENTS; i++)
	{
		if (!ContrailSegments[i] || !ContrailMats[i]) continue;
		ContrailSegments[i]->SetVisibility(true);

		float AgeFrac = static_cast<float>(i) / CONTRAIL_SEGMENTS;
		float Fade = FMath::Pow(1.f - AgeFrac, 1.3f);

		// Widen older segments (energy dissipation)
		FVector S = ContrailSegments[i]->GetComponentScale();
		float Expand = 1.f + AgeFrac * 1.5f;
		ContrailSegments[i]->SetWorldScale3D(
			FVector(S.X * Expand, S.Y * Expand, S.Z));

		ContrailMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(8.f * Fade, 18.f * Fade, 50.f * Fade));
	}

	// Update contrail light intensity based on speed
	if (ContrailLight)
	{
		float SpeedFrac = MoveDist / (MaxDescentSpeed * DeltaTime + 1.f);
		ContrailLight->SetIntensity(60000.f * FMath::Clamp(SpeedFrac, 0.2f, 1.f));
	}

	PrevPodLocation = CurrentLoc;
}

// --- Landing scorch mark (persistent ground burn) ---

void AExoDropPod::SpawnLandingScorch()
{
	if (bScorchSpawned) return;
	bScorchSpawned = true;

	// Spawn a scorch mark at impact location using the kill-scorch pattern
	FVector ScorchLoc = GetActorLocation() - FVector(0.f, 0.f, 150.f);
	AExoKillScorch::SpawnScorch(GetWorld(), ScorchLoc, true); // true = large variant
}

// --- Door open steam/energy burst ---

void AExoDropPod::SpawnDoorSteam()
{
	if (!SphereMesh) return;

	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!AddMat) return;

	SteamAge = 0.f;

	// Door direction: forward and to the side from pod
	FVector DoorOrigin = GetActorLocation() + FVector(120.f, 0.f, 0.f);
	FRotator PodYaw = FRotator(0.f, GetActorRotation().Yaw, 0.f);

	for (int32 i = 0; i < STEAM_PUFFS; i++)
	{
		UStaticMeshComponent* Puff = NewObject<UStaticMeshComponent>(this);
		Puff->SetStaticMesh(SphereMesh);
		Puff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Puff->CastShadow = false;
		Puff->SetWorldScale3D(FVector(0.15f, 0.15f, 0.1f));
		Puff->RegisterComponent();

		// Fan outward from door opening
		float Angle = -60.f + (120.f * i / (STEAM_PUFFS - 1));
		FVector Dir = PodYaw.RotateVector(
			FVector(FMath::Cos(FMath::DegreesToRadians(Angle)),
				FMath::Sin(FMath::DegreesToRadians(Angle)),
				FMath::RandRange(0.1f, 0.4f)).GetSafeNormal());

		float Speed = FMath::RandRange(300.f, 600.f);
		SteamVelocities[i] = Dir * Speed;

		// Slight vertical offset per puff
		FVector PuffLoc = DoorOrigin
			+ FVector(0.f, 0.f, FMath::RandRange(-30.f, 50.f));
		Puff->SetWorldLocation(PuffLoc);

		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(AddMat, this);
		if (!Mat) { continue; }
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(22.f, 28.f, 40.f)); // Cool white-blue energy steam
		Puff->SetMaterial(0, Mat);

		SteamPuffs[i] = Puff;
		SteamMats[i] = Mat;
	}
}
