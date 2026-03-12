// ExoLevelBuilderLighting.cpp — Compound-specific ambient lighting for distinct area identity
#include "Map/ExoLevelBuilder.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildCompoundLighting()
{
	// Each compound gets a distinct color theme via ambient point lights,
	// accent floor panels, and overhead color wash.

	struct FCompoundTheme
	{
		FVector Center;
		FLinearColor AccentColor;
		float Radius;
	};

	TArray<FCompoundTheme> Themes = {
		// Command Center — white/cyan (authority, clean)
		{FVector(0.f, 0.f, 0.f), FLinearColor(0.4f, 0.8f, 1.f), 12000.f},
		// Industrial — amber/orange (machinery, heat)
		{FVector(0.f, NorthY, 0.f), FLinearColor(1.f, 0.6f, 0.15f), 15000.f},
		// Research Labs — green (science, bio)
		{FVector(0.f, SouthY, 0.f), FLinearColor(0.2f, 1.f, 0.4f), 13000.f},
		// Power Station — electric blue (energy, arcs)
		{FVector(EastX, 0.f, 0.f), FLinearColor(0.15f, 0.4f, 1.f), 14000.f},
		// Barracks — red (military, alert)
		{FVector(WestX, 0.f, 0.f), FLinearColor(1.f, 0.2f, 0.15f), 14000.f},
	};

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	for (const FCompoundTheme& T : Themes)
	{
		// 4 ambient lights at ground level around the compound perimeter
		float Spread = T.Radius * 0.6f;
		FVector Offsets[] = {
			{Spread, 0.f, 100.f}, {-Spread, 0.f, 100.f},
			{0.f, Spread, 100.f}, {0.f, -Spread, 100.f},
		};
		for (const FVector& Offset : Offsets)
		{
			UPointLightComponent* Ambient = NewObject<UPointLightComponent>(this);
			Ambient->SetupAttachment(RootComponent);
			Ambient->SetWorldLocation(T.Center + Offset);
			Ambient->SetIntensity(3000.f); // Subtle accent, not overpowering
			Ambient->SetAttenuationRadius(T.Radius * 0.4f);
			Ambient->SetLightColor(T.AccentColor);
			Ambient->CastShadows = false;
			Ambient->RegisterComponent();

			// Ground accent strip at each perimeter light
			if (EmissiveMat)
			{
				FVector StripDir = Offset.GetSafeNormal2D();
				float StripYaw = FMath::RadiansToDegrees(FMath::Atan2(StripDir.Y, StripDir.X));
				UStaticMeshComponent* Strip = SpawnStaticMesh(
					T.Center + FVector(Offset.X, Offset.Y, 3.f),
					FVector(2.f, 0.06f, 0.02f), FRotator(0.f, StripYaw, 0.f),
					CubeMesh, T.AccentColor);
				if (Strip)
				{
					UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(EmissiveMat, this);
					if (!SM) { return; }
					SM->SetVectorParameterValue(TEXT("EmissiveColor"),
						FLinearColor(T.AccentColor.R * 0.6f, T.AccentColor.G * 0.6f,
							T.AccentColor.B * 0.6f));
					Strip->SetMaterial(0, SM);
				}
			}
		}

		// Overhead color wash light (subtle compound identity)
		UPointLightComponent* Overhead = NewObject<UPointLightComponent>(this);
		Overhead->SetupAttachment(RootComponent);
		Overhead->SetWorldLocation(T.Center + FVector(0.f, 0.f, 5000.f));
		Overhead->SetIntensity(2000.f); // Very subtle compound tint
		Overhead->SetAttenuationRadius(T.Radius);
		Overhead->SetLightColor(FLinearColor(
			T.AccentColor.R * 0.2f, T.AccentColor.G * 0.2f, T.AccentColor.B * 0.2f));
		Overhead->CastShadows = false;
		Overhead->RegisterComponent();

		// Perimeter glow ring — thin emissive cylinder marking compound boundary
		if (EmissiveMat)
		{
			float RingScale = T.Radius / 50.f;
			UStaticMeshComponent* Perimeter = SpawnStaticMesh(
				T.Center + FVector(0.f, 0.f, 2.f),
				FVector(RingScale, RingScale, 0.008f), FRotator::ZeroRotator,
				CylinderMesh, T.AccentColor);
			if (Perimeter)
			{
				UMaterialInstanceDynamic* PM = UMaterialInstanceDynamic::Create(EmissiveMat, this);
				if (!PM) { return; }
				PM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(T.AccentColor.R * 1.f, T.AccentColor.G * 1.f,
						T.AccentColor.B * 1.f));
				Perimeter->SetMaterial(0, PM);
			}
		}

		// Glowing ground markers at the compound center
		SpawnCompoundGroundMarker(T.Center, T.AccentColor);
	}

	// === FIELD AMBIENT LIGHTS — subtle color wash across open terrain ===
	// With sky atmosphere + sun + sky light, these should be much subtler
	struct FFieldLight { FVector Pos; FLinearColor Color; float Intensity; float Radius; };
	TArray<FFieldLight> FieldLights = {
		// NE quadrant — cool teal transition
		{{8000.f, 8000.f, 200.f}, FLinearColor(0.15f, 0.5f, 0.6f), 2000.f, 20000.f},
		{{5000.f, 12000.f, 150.f}, FLinearColor(0.2f, 0.6f, 0.5f), 1500.f, 18000.f},
		// NW quadrant — amber-red transition
		{{-8000.f, 8000.f, 200.f}, FLinearColor(0.7f, 0.3f, 0.1f), 1800.f, 18000.f},
		{{-12000.f, 5000.f, 150.f}, FLinearColor(0.8f, 0.25f, 0.1f), 1500.f, 16000.f},
		// SE quadrant — electric blue transition
		{{8000.f, -8000.f, 200.f}, FLinearColor(0.1f, 0.3f, 0.8f), 1800.f, 18000.f},
		{{12000.f, -12000.f, 180.f}, FLinearColor(0.15f, 0.35f, 0.7f), 1200.f, 15000.f},
		// SW quadrant — green-purple transition
		{{-8000.f, -8000.f, 200.f}, FLinearColor(0.3f, 0.6f, 0.4f), 1800.f, 18000.f},
		{{-12000.f, -12000.f, 180.f}, FLinearColor(0.4f, 0.15f, 0.6f), 1200.f, 15000.f},
		// Far corners — subtle color washes
		{{20000.f, 20000.f, 300.f}, FLinearColor(0.2f, 0.4f, 0.5f), 1000.f, 25000.f},
		{{-20000.f, 20000.f, 300.f}, FLinearColor(0.6f, 0.2f, 0.15f), 1000.f, 25000.f},
		{{20000.f, -20000.f, 300.f}, FLinearColor(0.1f, 0.25f, 0.6f), 1000.f, 25000.f},
		{{-20000.f, -20000.f, 300.f}, FLinearColor(0.35f, 0.5f, 0.15f), 1000.f, 25000.f},
	};
	for (const FFieldLight& FL : FieldLights)
	{
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetWorldLocation(FL.Pos);
		Light->SetIntensity(FL.Intensity);
		Light->SetAttenuationRadius(FL.Radius);
		Light->SetLightColor(FL.Color);
		Light->CastShadows = false;
		Light->RegisterComponent();
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Compound ambient lighting placed"));
}

void AExoLevelBuilder::SpawnCompoundGroundMarker(const FVector& Center,
	const FLinearColor& Color)
{
	// Circular glowing ring on the ground
	float RingRadius = 2000.f;
	float RingScale = RingRadius / 50.f;

	UMaterialInterface* MarkerEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	UStaticMeshComponent* Ring = SpawnStaticMesh(Center + FVector(0.f, 0.f, 5.f),
		FVector(RingScale, RingScale, 0.01f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(Color.R * 0.1f, Color.G * 0.1f, Color.B * 0.1f));
	if (Ring)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(MarkerEmissiveMat, this);
		if (!Mat) { return; }
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 0.5f, Color.G * 0.5f, Color.B * 0.5f));
		Ring->SetMaterial(0, Mat);
	}

	// Inner solid circle (darker)
	float InnerScale = RingScale * 0.6f;
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 3.f),
		FVector(InnerScale, InnerScale, 0.01f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(Color.R * 0.03f, Color.G * 0.03f, Color.B * 0.03f));

	// Radiating lines from center (4 directions)
	for (int32 i = 0; i < 4; i++)
	{
		float Angle = i * 90.f;
		float Rad = FMath::DegreesToRadians(Angle);
		FVector Dir(FMath::Cos(Rad), FMath::Sin(Rad), 0.f);
		FVector LineCenter = Center + Dir * RingRadius * 0.8f + FVector(0.f, 0.f, 4.f);

		UStaticMeshComponent* Line = SpawnStaticMesh(LineCenter,
			FVector(RingRadius * 0.6f / 100.f, 0.08f, 0.02f),
			FRotator(0.f, Angle, 0.f), CubeMesh,
			FLinearColor(Color.R * 0.08f, Color.G * 0.08f, Color.B * 0.08f));
		if (Line)
		{
			UMaterialInstanceDynamic* LMat = UMaterialInstanceDynamic::Create(MarkerEmissiveMat, this);
			if (!LMat) { return; }
			LMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 1.f, Color.G * 1.f, Color.B * 1.f));
			Line->SetMaterial(0, LMat);
		}
	}

	// Central glow light flush with ground
	UPointLightComponent* CenterLight = NewObject<UPointLightComponent>(this);
	CenterLight->SetupAttachment(RootComponent);
	CenterLight->SetWorldLocation(Center + FVector(0.f, 0.f, 50.f));
	CenterLight->SetIntensity(3000.f);
	CenterLight->SetAttenuationRadius(RingRadius * 0.8f);
	CenterLight->SetLightColor(Color);
	CenterLight->CastShadows = false;
	CenterLight->RegisterComponent();
}
