// ExoLevelBuilderLighting.cpp — Compound-specific ambient lighting for distinct area identity
#include "Map/ExoLevelBuilder.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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
		{FVector(0.f, 80000.f, 0.f), FLinearColor(1.f, 0.6f, 0.15f), 15000.f},
		// Research Labs — green (science, bio)
		{FVector(0.f, -80000.f, 0.f), FLinearColor(0.2f, 1.f, 0.4f), 13000.f},
		// Power Station — electric blue (energy, arcs)
		{FVector(80000.f, 0.f, 0.f), FLinearColor(0.15f, 0.4f, 1.f), 14000.f},
		// Barracks — red (military, alert)
		{FVector(-80000.f, 0.f, 0.f), FLinearColor(1.f, 0.2f, 0.15f), 14000.f},
	};

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
			Ambient->SetIntensity(5000.f);
			Ambient->SetAttenuationRadius(T.Radius * 0.5f);
			Ambient->SetLightColor(T.AccentColor);
			Ambient->CastShadows = false;
			Ambient->RegisterComponent();
		}

		// Overhead color wash light (high above, wide radius)
		UPointLightComponent* Overhead = NewObject<UPointLightComponent>(this);
		Overhead->SetupAttachment(RootComponent);
		Overhead->SetWorldLocation(T.Center + FVector(0.f, 0.f, 5000.f));
		Overhead->SetIntensity(8000.f);
		Overhead->SetAttenuationRadius(T.Radius * 0.8f);
		Overhead->SetLightColor(FLinearColor(
			T.AccentColor.R * 0.3f, T.AccentColor.G * 0.3f, T.AccentColor.B * 0.3f));
		Overhead->CastShadows = false;
		Overhead->RegisterComponent();

		// Glowing ground markers at the compound center
		SpawnCompoundGroundMarker(T.Center, T.AccentColor);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Compound ambient lighting placed"));
}

void AExoLevelBuilder::SpawnCompoundGroundMarker(const FVector& Center,
	const FLinearColor& Color)
{
	// Circular glowing ring on the ground
	float RingRadius = 2000.f;
	float RingScale = RingRadius / 50.f;

	UStaticMeshComponent* Ring = SpawnStaticMesh(Center + FVector(0.f, 0.f, 5.f),
		FVector(RingScale, RingScale, 0.01f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(Color.R * 0.1f, Color.G * 0.1f, Color.B * 0.1f));
	if (Ring)
	{
		UMaterialInstanceDynamic* Mat = Cast<UMaterialInstanceDynamic>(
			Ring->GetMaterial(0));
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 0.5f, Color.G * 0.5f, Color.B * 0.5f));
		}
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
			UMaterialInstanceDynamic* LMat = Cast<UMaterialInstanceDynamic>(
				Line->GetMaterial(0));
			if (LMat)
			{
				LMat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(Color.R * 0.3f, Color.G * 0.3f, Color.B * 0.3f));
			}
		}
	}

	// Central glow light flush with ground
	UPointLightComponent* CenterLight = NewObject<UPointLightComponent>(this);
	CenterLight->SetupAttachment(RootComponent);
	CenterLight->SetWorldLocation(Center + FVector(0.f, 0.f, 50.f));
	CenterLight->SetIntensity(6000.f);
	CenterLight->SetAttenuationRadius(RingRadius * 1.2f);
	CenterLight->SetLightColor(Color);
	CenterLight->CastShadows = false;
	CenterLight->RegisterComponent();
}
