// ExoDropPodVisuals.cpp — Pod mesh construction, thruster VFX, landing dust
#include "Map/ExoDropPod.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"

void AExoDropPod::BuildPodMesh()
{
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
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
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
	if (CylinderMesh && BaseMaterial)
	{
		ThrusterFlame = NewObject<UStaticMeshComponent>(this);
		ThrusterFlame->SetupAttachment(RootComponent);
		ThrusterFlame->SetStaticMesh(CylinderMesh);
		ThrusterFlame->SetRelativeLocation(FVector(0.f, 0.f, -250.f));
		ThrusterFlame->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.01f));
		ThrusterFlame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThrusterFlame->CastShadow = false;
		ThrusterFlame->RegisterComponent();
		FlameMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		FlameMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(2.f, 4.f, 10.f));
		FlameMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.f, 4.f, 10.f));
		ThrusterFlame->SetMaterial(0, FlameMat);
	}

	// Heat shield glow
	if (SphereMesh && BaseMaterial)
	{
		HeatShield = NewObject<UStaticMeshComponent>(this);
		HeatShield->SetupAttachment(RootComponent);
		HeatShield->SetStaticMesh(SphereMesh);
		HeatShield->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
		HeatShield->SetRelativeScale3D(FVector(2.5f, 2.5f, 3.5f));
		HeatShield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HeatShield->CastShadow = false;
		HeatShield->RegisterComponent();
		HeatMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		HeatMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.3f, 0.05f));
		HeatMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(3.f, 0.8f, 0.15f));
		HeatShield->SetMaterial(0, HeatMat);

		HeatLight = NewObject<UPointLightComponent>(this);
		HeatLight->SetupAttachment(RootComponent);
		HeatLight->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
		HeatLight->SetIntensity(0.f);
		HeatLight->SetAttenuationRadius(2000.f);
		HeatLight->SetLightColor(FLinearColor(1.f, 0.4f, 0.1f));
		HeatLight->CastShadows = false;
		HeatLight->RegisterComponent();
	}
}

void AExoDropPod::SpawnLandingDust()
{
	if (!CylinderMesh || !BaseMaterial) return;

	// Expanding dust ring at ground level
	DustRing = NewObject<UStaticMeshComponent>(this);
	DustRing->SetStaticMesh(CylinderMesh);
	DustRing->SetWorldLocation(GetActorLocation() - FVector(0.f, 0.f, 150.f));
	DustRing->SetWorldScale3D(FVector(1.f, 1.f, 0.05f));
	DustRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DustRing->CastShadow = false;
	DustRing->RegisterComponent();
	DustMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	DustMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.12f, 0.08f, 0.3f));
	DustMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.3f, 0.25f, 0.15f));
	DustRing->SetMaterial(0, DustMat);
	DustAge = 0.f;

	// Bright flash on impact
	ImpactFlash = NewObject<UPointLightComponent>(this);
	ImpactFlash->SetWorldLocation(GetActorLocation());
	ImpactFlash->SetIntensity(100000.f);
	ImpactFlash->SetAttenuationRadius(5000.f);
	ImpactFlash->SetLightColor(FLinearColor(0.8f, 0.9f, 1.f));
	ImpactFlash->CastShadows = false;
	ImpactFlash->RegisterComponent();
}

void AExoDropPod::UpdateThrusterVFX(float DeltaTime, float BrakeAlpha)
{
	if (!ThrusterLight) return;

	float Time = GetWorld()->GetTimeSeconds();

	float TargetIntensity = 0.f;
	if (Phase == EDropPodPhase::FreeFall)
		TargetIntensity = 5000.f;
	else if (Phase == EDropPodPhase::Braking)
		TargetIntensity = FMath::Lerp(5000.f, 80000.f, BrakeAlpha);
	else if (Phase == EDropPodPhase::Landing)
		TargetIntensity = 80000.f;

	ThrusterLight->SetIntensity(FMath::FInterpTo(
		ThrusterLight->Intensity, TargetIntensity, DeltaTime, 4.f));

	FLinearColor ThrusterColor = FMath::Lerp(
		FLinearColor(0.3f, 0.5f, 1.f),
		FLinearColor(0.8f, 0.9f, 1.f),
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
				FLinearColor(1.f, 3.f, 8.f),
				FLinearColor(8.f, 8.f, 10.f),
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
			FLinearColor HeatCol(3.f * HeatAlpha, 0.8f * HeatAlpha, 0.15f * HeatAlpha);
			HeatMat->SetVectorParameterValue(TEXT("EmissiveColor"), HeatCol);
		}
		if (HeatLight)
		{
			HeatLight->SetIntensity(HeatAlpha * 30000.f);
		}
	}
}
