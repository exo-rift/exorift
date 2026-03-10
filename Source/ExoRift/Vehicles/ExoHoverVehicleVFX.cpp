// ExoHoverVehicleVFX.cpp — Visual effects: engine glow, thruster animation,
// headlights, body accents, hover dust
#include "Vehicles/ExoHoverVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoHoverVehicle::BeginPlay()
{
	Super::BeginPlay();

	// Dynamic materials for body and thrusters
	UMaterialInterface* Base = VehicleMesh ? VehicleMesh->GetMaterial(0) : nullptr;
	if (Base)
	{
		BodyMat = UMaterialInstanceDynamic::Create(Base, this);
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.06f, 0.07f, 0.09f));
		VehicleMesh->SetMaterial(0, BodyMat);

		// Windshield — dark tinted with subtle cyan emissive
		if (Windshield)
		{
			WindshieldMat = UMaterialInstanceDynamic::Create(Base, this);
			WindshieldMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.02f, 0.04f, 0.06f));
			WindshieldMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.05f, 0.15f, 0.3f));
			Windshield->SetMaterial(0, WindshieldMat);
		}

		// Side panels — dark with accent stripe
		auto SetPanelMat = [&](UStaticMeshComponent* Panel)
		{
			if (!Panel) return;
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(Base, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.05f, 0.06f, 0.08f));
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.02f, 0.08f, 0.15f));
			Panel->SetMaterial(0, Mat);
		};
		SetPanelMat(SidePanelL);
		SetPanelMat(SidePanelR);

		// Rear fin
		if (RearFin)
		{
			UMaterialInstanceDynamic* FinMat = UMaterialInstanceDynamic::Create(Base, this);
			FinMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.08f, 0.08f, 0.1f));
			RearFin->SetMaterial(0, FinMat);
		}

		// Hover dust clouds — translucent grey
		if (HoverDustL)
		{
			DustMat = UMaterialInstanceDynamic::Create(Base, this);
			DustMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.2f, 0.2f, 0.18f));
			HoverDustL->SetMaterial(0, DustMat);
			if (HoverDustR) HoverDustR->SetMaterial(0, DustMat);
		}
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

	// Headlight brightness — brighter when occupied and moving
	if (HeadlightL && HeadlightR)
	{
		float HeadIntensity = bIsOccupied
			? 80000.f + 40000.f * SpeedPct
			: 20000.f; // Dim parking lights when empty
		HeadlightL->SetIntensity(HeadIntensity);
		HeadlightR->SetIntensity(HeadIntensity);
	}

	// Windshield emissive pulse — subtle HUD glow effect
	if (WindshieldMat)
	{
		float WsPulse = 0.15f + 0.05f * FMath::Sin(Time * 3.f);
		if (bBoosted) WsPulse += 0.1f;
		WindshieldMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(WsPulse * 0.3f, WsPulse, WsPulse * 2.f));
	}

	// Hover dust — scale based on proximity to ground and speed
	if (HoverDustL && HoverDustR)
	{
		FVector Loc = GetActorLocation();
		FHitResult GroundHit;
		FCollisionQueryParams QParams;
		QParams.AddIgnoredActor(this);
		bool bNearGround = GetWorld()->LineTraceSingleByChannel(
			GroundHit, Loc, Loc - FVector(0.f, 0.f, HoverHeight * 2.f),
			ECC_Visibility, QParams);

		if (bNearGround && SpeedPct > 0.05f)
		{
			float ProximityFactor = 1.f - FMath::Clamp(
				GroundHit.Distance / (HoverHeight * 1.5f), 0.f, 1.f);
			float DustScale = ProximityFactor * SpeedPct * 1.5f;
			if (bBoosted) DustScale *= 1.8f;

			float DustPulse = 1.f + 0.3f * FMath::Sin(Time * 8.f);
			FVector DS(DustScale * DustPulse, DustScale * DustPulse,
				DustScale * 0.4f * DustPulse);

			HoverDustL->SetRelativeScale3D(DS);
			HoverDustR->SetRelativeScale3D(DS);
			HoverDustL->SetVisibility(DustScale > 0.05f);
			HoverDustR->SetVisibility(DustScale > 0.05f);

			// Position dust at ground level
			float GroundOffset = -GroundHit.Distance + 20.f;
			HoverDustL->SetRelativeLocation(FVector(-40.f, -30.f, GroundOffset));
			HoverDustR->SetRelativeLocation(FVector(-40.f, 30.f, GroundOffset));
		}
		else
		{
			HoverDustL->SetVisibility(false);
			HoverDustR->SetVisibility(false);
		}
	}
}
