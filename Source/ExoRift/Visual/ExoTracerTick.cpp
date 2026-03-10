// ExoTracerTick.cpp — Tracer animation: traveling, helix orbiters, fading
#include "Visual/ExoTracer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoTracer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bReachedEnd)
		UpdateTraveling(DeltaTime);
	else
		UpdateFading(DeltaTime);
}

void AExoTracer::UpdateTraveling(float DeltaTime)
{
	TraveledDist += TravelSpeed * DeltaTime;
	if (TraveledDist >= TotalDistance)
	{
		TraveledDist = TotalDistance;
		bReachedEnd = true;
		FadeAge = 0.f;
	}

	float VisibleLen = FMath::Min(TraveledDist, BeamLength);
	float TailDist = FMath::Max(TraveledDist - BeamLength, 0.f);
	FVector HeadPos = StartPos + Direction * TraveledDist;
	FVector TailPos = StartPos + Direction * TailDist;
	SetActorLocation((HeadPos + TailPos) * 0.5f);

	float LenScale = VisibleLen / 100.f;
	float Time = GetWorld()->GetTimeSeconds();

	// Multi-frequency flicker for organic energy bolt feel
	float F1 = FMath::Sin(Time * 95.f) * 0.12f;
	float F2 = FMath::Sin(Time * 61.f) * 0.08f;
	float F3 = FMath::Sin(Time * 137.f) * 0.05f;
	float CoreFlicker = 1.f + F1 + F2 + F3;

	BeamCore->SetRelativeScale3D(FVector(
		CoreRadius * CoreFlicker, CoreRadius * CoreFlicker, LenScale));

	// Glow: slower organic pulse with breathing
	float GlowPulse = 1.f + 0.18f * FMath::Sin(Time * 28.f)
		+ 0.08f * FMath::Cos(Time * 47.f);
	BeamGlow->SetRelativeScale3D(FVector(
		GlowRadius * GlowPulse, GlowRadius * GlowPulse, LenScale * 0.95f));

	// Corona: large soft outer halo with slow drift
	float CoronaPulse = 1.f + 0.25f * FMath::Sin(Time * 18.f);
	Corona->SetRelativeScale3D(FVector(
		CoronaRadius * CoronaPulse, CoronaRadius * CoronaPulse, LenScale * 0.85f));

	// Trail: extends further back
	float TrailLen = FMath::Min(TraveledDist, BeamLength * 2.2f);
	BeamTrail->SetRelativeScale3D(FVector(TrailRadius, TrailRadius, TrailLen / 100.f));

	// Head: elongated energy ball at front, pulsing
	float HeadHalf = VisibleLen * 0.5f;
	float HeadPulse = HeadScale * (1.f + 0.25f * FMath::Sin(Time * 55.f));
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, HeadHalf));
	HeadMesh->SetRelativeScale3D(FVector(
		HeadPulse, HeadPulse * 0.65f, HeadPulse * 1.4f));

	// Head shockwave ring — pulsing expansion at bolt front
	float RingPulse = 1.f + 0.4f * FMath::Sin(Time * 45.f);
	float RingS = HeadScale * 0.6f * RingPulse;
	HeadRing->SetRelativeLocation(FVector(0.f, 0.f, HeadHalf));
	HeadRing->SetRelativeScale3D(FVector(RingS, RingS, 0.015f));

	// Helix orbiters — spiral around the beam axis
	HelixAngle += HelixSpeed * DeltaTime;
	for (int32 i = 0; i < HelixOrbs.Num(); i++)
	{
		float Angle = HelixAngle + i * (PI / NUM_HELIX);
		float HR = HelixRadius / 50.f;
		float OX = FMath::Cos(Angle) * HR;
		float OY = FMath::Sin(Angle) * HR;
		// Position along middle-front of beam
		float OZ = HeadHalf * (0.3f - 0.2f * i);
		HelixOrbs[i]->SetRelativeLocation(FVector(OX, OY, OZ));
		// Spin the orbs themselves
		float Spin = Time * (300.f + i * 150.f);
		HelixOrbs[i]->SetRelativeRotation(FRotator(Spin, Spin * 0.6f, 0.f));
	}

	// Dynamic emissive pulses on core
	float EmissivePulse = 1.f + 0.3f * FMath::Sin(Time * 70.f);
	if (CoreMat)
		CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreColor * EmissivePulse);
	if (GlowMat)
	{
		float GP = 1.f + 0.2f * FMath::Sin(Time * 40.f);
		GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * GP);
	}

	// Lights: strong with flicker
	float LightFlicker = 1.f + 0.2f * FMath::Sin(Time * 35.f)
		+ 0.1f * FMath::Sin(Time * 73.f);
	HeadLight->SetRelativeLocation(FVector(0.f, 0.f, HeadHalf));
	HeadLight->SetIntensity(LightIntensity * LightFlicker);
	TailLight->SetRelativeLocation(FVector(0.f, 0.f, -VisibleLen * 0.4f));

	// Trailing sparks: scatter outward from the beam wake
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		SparkOffsets[i] += SparkVelocities[i] * DeltaTime;
		float SparkZ = -VisibleLen * (0.2f + 0.15f * i);
		SparkMeshes[i]->SetRelativeLocation(
			FVector(SparkOffsets[i].X, SparkOffsets[i].Y, SparkZ));
		float R = Time * (200.f + i * 80.f);
		SparkMeshes[i]->SetRelativeRotation(FRotator(R, R * 0.7f, R * 0.5f));
	}
}

void AExoTracer::UpdateFading(float DeltaTime)
{
	FadeAge += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(FadeAge / FadeTime, 0.f, 1.f);
	float AlphaSq = Alpha * Alpha;

	HeadLight->SetIntensity(LightIntensity * AlphaSq);
	TailLight->SetIntensity(LightIntensity * 0.35f * Alpha);

	HeadMesh->SetRelativeScale3D(FVector(HeadScale * Alpha));

	FVector CoreScale = BeamCore->GetRelativeScale3D();
	float Z = CoreScale.Z;
	BeamCore->SetRelativeScale3D(FVector(CoreRadius * Alpha, CoreRadius * Alpha, Z));
	BeamGlow->SetRelativeScale3D(FVector(
		GlowRadius * AlphaSq, GlowRadius * AlphaSq, Z * 0.95f));
	Corona->SetRelativeScale3D(FVector(
		CoronaRadius * AlphaSq * 0.5f, CoronaRadius * AlphaSq * 0.5f, Z * 0.8f));

	// Head ring expands outward and fades
	float RingFade = HeadScale * 1.5f * (1.f - Alpha);
	HeadRing->SetRelativeScale3D(FVector(RingFade, RingFade, 0.005f * Alpha));

	// Helix orbiters spiral outward as they fade
	HelixAngle += HelixSpeed * DeltaTime * 0.5f;
	for (int32 i = 0; i < HelixOrbs.Num(); i++)
	{
		float Angle = HelixAngle + i * (PI / NUM_HELIX);
		float ExpandR = (HelixRadius / 50.f) * (1.f + (1.f - Alpha) * 3.f);
		float OX = FMath::Cos(Angle) * ExpandR;
		float OY = FMath::Sin(Angle) * ExpandR;
		float OrbS = CoreRadius * 0.5f * Alpha;
		HelixOrbs[i]->SetRelativeLocation(FVector(OX, OY, 0.f));
		HelixOrbs[i]->SetRelativeScale3D(FVector(OrbS, OrbS, OrbS));
	}

	// Trail lingers longest
	float TrailAlpha = FMath::Clamp(Alpha * 1.8f, 0.f, 1.f);
	FVector TS = BeamTrail->GetRelativeScale3D();
	BeamTrail->SetRelativeScale3D(FVector(
		TrailRadius * TrailAlpha, TrailRadius * TrailAlpha, TS.Z));

	if (TrailMat)
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowColor * 0.4f * TrailAlpha);
	if (CoronaMat)
		CoronaMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			GlowColor * 0.15f * AlphaSq);

	// Sparks scatter outward and fade
	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		SparkOffsets[i] += SparkVelocities[i] * DeltaTime * 3.f;
		float S = FMath::Max(0.01f, 0.03f * Alpha);
		SparkMeshes[i]->SetRelativeScale3D(FVector(S, S * 0.4f, S * 2.f));
	}

	if (FadeAge >= FadeTime) Destroy();
}
