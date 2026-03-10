// ExoAbilityGrappleVFX.cpp — Grapple beam visual effects with glow and target ring
#include "Player/ExoAbilityComponent.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UExoAbilityComponent::CreateGrappleBeam()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	static UStaticMesh* CylMesh = nullptr;
	static UStaticMesh* SphMesh = nullptr;
	if (!CylMesh)
	{
		CylMesh = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Engine/BasicShapes/Cylinder"));
		SphMesh = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Engine/BasicShapes/Sphere"));
	}
	if (!CylMesh) return;

	UMaterialInterface* EmAdditive = FExoMaterialFactory::GetEmissiveAdditive();
	FLinearColor BeamCol(0.2f, 1.f, 0.5f);

	// Core beam — thin bright cylinder
	GrappleBeam = NewObject<UStaticMeshComponent>(Owner);
	GrappleBeam->SetStaticMesh(CylMesh);
	GrappleBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrappleBeam->CastShadow = false;
	GrappleBeam->RegisterComponent();

	if (EmAdditive)
	{
		GrappleBeamMat = UMaterialInstanceDynamic::Create(EmAdditive, Owner);
		GrappleBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(2.f, 12.f, 6.f));
		GrappleBeam->SetMaterial(0, GrappleBeamMat);
	}

	// Outer glow beam — wider, dimmer, additive
	GrappleGlow = NewObject<UStaticMeshComponent>(Owner);
	GrappleGlow->SetStaticMesh(CylMesh);
	GrappleGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrappleGlow->CastShadow = false;
	GrappleGlow->RegisterComponent();

	if (EmAdditive)
	{
		GrappleGlowMat = UMaterialInstanceDynamic::Create(EmAdditive, Owner);
		GrappleGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.3f, 1.5f, 0.8f));
		GrappleGlow->SetMaterial(0, GrappleGlowMat);
	}

	// Target impact ring — flat cylinder at grapple target
	GrappleTargetRing = NewObject<UStaticMeshComponent>(Owner);
	GrappleTargetRing->SetStaticMesh(CylMesh);
	GrappleTargetRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrappleTargetRing->CastShadow = false;
	GrappleTargetRing->RegisterComponent();

	if (EmAdditive)
	{
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(EmAdditive, Owner);
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(1.f, 6.f, 3.f));
		GrappleTargetRing->SetMaterial(0, RingMat);
	}

	// Source anchor flash — sphere at character
	if (SphMesh)
	{
		GrappleSourceFlash = NewObject<UStaticMeshComponent>(Owner);
		GrappleSourceFlash->SetStaticMesh(SphMesh);
		GrappleSourceFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GrappleSourceFlash->CastShadow = false;
		GrappleSourceFlash->RegisterComponent();

		if (EmAdditive)
		{
			UMaterialInstanceDynamic* SrcMat = UMaterialInstanceDynamic::Create(EmAdditive, Owner);
			SrcMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(1.5f, 8.f, 4.f));
			GrappleSourceFlash->SetMaterial(0, SrcMat);
		}
	}

	// Target glow light
	GrappleLight = NewObject<UPointLightComponent>(Owner);
	GrappleLight->SetWorldLocation(GrappleTarget);
	GrappleLight->SetIntensity(40000.f);
	GrappleLight->SetAttenuationRadius(1200.f);
	GrappleLight->SetLightColor(BeamCol);
	GrappleLight->CastShadows = false;
	GrappleLight->RegisterComponent();

	// Source anchor light
	GrappleSourceLight = NewObject<UPointLightComponent>(Owner);
	GrappleSourceLight->SetWorldLocation(Owner->GetActorLocation());
	GrappleSourceLight->SetIntensity(20000.f);
	GrappleSourceLight->SetAttenuationRadius(600.f);
	GrappleSourceLight->SetLightColor(BeamCol);
	GrappleSourceLight->CastShadows = false;
	GrappleSourceLight->RegisterComponent();
}

void UExoAbilityComponent::UpdateGrappleBeam()
{
	if (!GrappleBeam) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector CharPos = Owner->GetActorLocation();
	FVector Mid = (CharPos + GrappleTarget) * 0.5f;
	FVector Dir = GrappleTarget - CharPos;
	float Length = Dir.Size();
	float Time = GetWorld()->GetTimeSeconds();

	// Orient beam from character to target
	FRotator BeamRot = Dir.Rotation();
	BeamRot.Pitch += 90.f;
	float LenScale = Length / 100.f;

	// Core beam — thin, pulsing
	float CorePulse = 1.f + 0.2f * FMath::Sin(Time * 30.f);
	GrappleBeam->SetWorldLocation(Mid);
	GrappleBeam->SetWorldRotation(BeamRot);
	GrappleBeam->SetWorldScale3D(FVector(0.02f * CorePulse, 0.02f * CorePulse, LenScale));

	// Outer glow — wider, counter-phase pulse
	float GlowPulse = 1.f + 0.15f * FMath::Sin(Time * 25.f + PI * 0.5f);
	GrappleGlow->SetWorldLocation(Mid);
	GrappleGlow->SetWorldRotation(BeamRot);
	GrappleGlow->SetWorldScale3D(FVector(0.06f * GlowPulse, 0.06f * GlowPulse, LenScale));

	// Target ring — pulsing flat cylinder
	if (GrappleTargetRing)
	{
		float RingPulse = 0.8f + 0.4f * FMath::Sin(Time * 15.f);
		GrappleTargetRing->SetWorldLocation(GrappleTarget);
		GrappleTargetRing->SetWorldScale3D(FVector(RingPulse, RingPulse, 0.005f));
	}

	// Source flash — small pulsing sphere at character hand
	if (GrappleSourceFlash)
	{
		float SrcPulse = 0.15f + 0.08f * FMath::Sin(Time * 20.f);
		GrappleSourceFlash->SetWorldLocation(CharPos + FVector(0.f, 0.f, 40.f));
		GrappleSourceFlash->SetWorldScale3D(FVector(SrcPulse));
	}

	// Emissive pulses
	if (GrappleBeamMat)
	{
		float EmPulse = 1.f + 0.4f * FMath::Sin(Time * 25.f);
		GrappleBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(2.f * EmPulse, 12.f * EmPulse, 6.f * EmPulse));
	}
	if (GrappleGlowMat)
	{
		float GlowEm = 1.f + 0.3f * FMath::Sin(Time * 20.f + 1.f);
		GrappleGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.3f * GlowEm, 1.5f * GlowEm, 0.8f * GlowEm));
	}

	// Lights follow positions
	if (GrappleLight)
	{
		GrappleLight->SetWorldLocation(GrappleTarget);
		GrappleLight->SetIntensity(40000.f * CorePulse);
	}
	if (GrappleSourceLight)
	{
		GrappleSourceLight->SetWorldLocation(CharPos);
		GrappleSourceLight->SetIntensity(20000.f * CorePulse);
	}
}

void UExoAbilityComponent::DestroyGrappleBeam()
{
	auto SafeDestroy = [](auto*& Comp)
	{
		if (Comp) { Comp->DestroyComponent(); Comp = nullptr; }
	};

	SafeDestroy(GrappleBeam);
	SafeDestroy(GrappleGlow);
	SafeDestroy(GrappleTargetRing);
	SafeDestroy(GrappleSourceFlash);
	SafeDestroy(GrappleLight);
	SafeDestroy(GrappleSourceLight);
	GrappleBeamMat = nullptr;
	GrappleGlowMat = nullptr;
}
