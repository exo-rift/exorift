// ExoHoverVehicleVFX.cpp — Visual effects: engine glow, thruster animation, body accent
#include "Vehicles/ExoHoverVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoHoverVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Dynamic materials for body and thrusters
	UMaterialInterface* Base = VehicleMesh->GetMaterial(0);
	if (Base)
	{
		BodyMat = UMaterialInstanceDynamic::Create(Base, this);
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.06f, 0.07f, 0.09f));
		VehicleMesh->SetMaterial(0, BodyMat);
	}

	if (ThrusterL)
	{
		UMaterialInterface* TBase = ThrusterL->GetMaterial(0);
		if (TBase)
		{
			ThrusterMat = UMaterialInstanceDynamic::Create(TBase, this);
			ThrusterMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.1f, 0.4f, 0.8f));
			ThrusterMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.2f, 1.f, 3.f));
			ThrusterL->SetMaterial(0, ThrusterMat);
			if (ThrusterR) ThrusterR->SetMaterial(0, ThrusterMat);
		}
	}
}

void AExoHoverVehicle::UpdateVFX(float DeltaTime)
{
	float Time = GetWorld()->GetTimeSeconds();
	float SpeedPct = FMath::Clamp(FMath::Abs(CurrentSpeed) / MaxSpeed, 0.f, 1.f);
	bool bBoosted = bIsBoosting && CurrentBoostEnergy > 0.f;

	// Engine glow intensity scales with speed
	float BaseIntensity = 3000.f + 8000.f * SpeedPct;
	float BoostExtra = bBoosted ? 12000.f : 0.f;
	float Pulse = 1.f + 0.2f * FMath::Sin(Time * 15.f);
	float Intensity = (BaseIntensity + BoostExtra) * Pulse;

	// Color shifts from blue to orange during boost
	FLinearColor EngineCol = bBoosted
		? FLinearColor(1.f, 0.5f, 0.1f)
		: FLinearColor(0.1f, 0.5f, 1.f);

	if (EngineGlowL)
	{
		EngineGlowL->SetIntensity(Intensity);
		EngineGlowL->SetLightColor(EngineCol);
	}
	if (EngineGlowR)
	{
		EngineGlowR->SetIntensity(Intensity);
		EngineGlowR->SetLightColor(EngineCol);
	}

	// Thruster flame scale — grows with speed, pulses during boost
	if (ThrusterL && ThrusterR)
	{
		float FlameLen = 0.1f + 0.25f * SpeedPct;
		if (bBoosted) FlameLen *= 1.5f;
		float FlamePulse = 1.f + 0.15f * FMath::Sin(Time * 30.f);
		float FlameRad = 0.06f + 0.04f * SpeedPct;
		FVector FlameScale(FlameRad * FlamePulse, FlameRad * FlamePulse, FlameLen);

		ThrusterL->SetRelativeScale3D(FlameScale);
		ThrusterR->SetRelativeScale3D(FlameScale);
	}

	// Thruster material emissive
	if (ThrusterMat)
	{
		float EmScale = 1.f + 4.f * SpeedPct;
		if (bBoosted) EmScale *= 2.f;
		FLinearColor EmCol = bBoosted
			? FLinearColor(3.f * EmScale, 1.5f * EmScale, 0.3f * EmScale)
			: FLinearColor(0.2f * EmScale, 1.f * EmScale, 3.f * EmScale);
		ThrusterMat->SetVectorParameterValue(TEXT("EmissiveColor"), EmCol);
	}

	// Body material accent stripe glow when boosting
	if (BodyMat)
	{
		float BodyEm = bBoosted ? 0.3f + 0.15f * FMath::Sin(Time * 10.f) : 0.f;
		FLinearColor BodyEmCol = bBoosted
			? FLinearColor(BodyEm * 2.f, BodyEm, BodyEm * 0.2f)
			: FLinearColor(0.f, 0.f, 0.f);
		BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), BodyEmCol);
	}
}
