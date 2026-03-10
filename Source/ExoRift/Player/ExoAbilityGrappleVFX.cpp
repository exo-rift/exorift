// ExoAbilityGrappleVFX.cpp — Grapple beam visual effects
#include "Player/ExoAbilityComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void UExoAbilityComponent::CreateGrappleBeam()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	static UStaticMesh* CylMesh = nullptr;
	static UMaterialInterface* BaseMat = nullptr;
	if (!CylMesh)
	{
		CylMesh = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Engine/BasicShapes/Cylinder"));
		BaseMat = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	}

	if (!CylMesh || !BaseMat) return;

	// Beam cylinder
	GrappleBeam = NewObject<UStaticMeshComponent>(Owner);
	GrappleBeam->SetStaticMesh(CylMesh);
	GrappleBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrappleBeam->CastShadow = false;
	GrappleBeam->RegisterComponent();

	GrappleBeamMat = UMaterialInstanceDynamic::Create(BaseMat, Owner);
	FLinearColor BeamCol(1.f, 6.f, 3.f); // Bright green energy
	GrappleBeamMat->SetVectorParameterValue(TEXT("BaseColor"), BeamCol);
	GrappleBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"), BeamCol);
	GrappleBeam->SetMaterial(0, GrappleBeamMat);

	// Glow light at target
	GrappleLight = NewObject<UPointLightComponent>(Owner);
	GrappleLight->SetWorldLocation(GrappleTarget);
	GrappleLight->SetIntensity(20000.f);
	GrappleLight->SetAttenuationRadius(800.f);
	GrappleLight->SetLightColor(FLinearColor(0.2f, 1.f, 0.5f));
	GrappleLight->CastShadows = false;
	GrappleLight->RegisterComponent();
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

	// Position beam at midpoint
	GrappleBeam->SetWorldLocation(Mid);

	// Orient cylinder along the beam direction
	FRotator BeamRot = Dir.Rotation();
	BeamRot.Pitch += 90.f; // Cylinder default is Z-up
	GrappleBeam->SetWorldRotation(BeamRot);

	// Scale: thin cylinder stretched to beam length
	float LenScale = Length / 100.f;
	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = 1.f + 0.15f * FMath::Sin(Time * 30.f);
	GrappleBeam->SetWorldScale3D(FVector(0.015f * Pulse, 0.015f * Pulse, LenScale));

	// Pulse emissive
	if (GrappleBeamMat)
	{
		float EmPulse = 1.f + 0.3f * FMath::Sin(Time * 20.f);
		FLinearColor BeamCol(1.f * EmPulse, 6.f * EmPulse, 3.f * EmPulse);
		GrappleBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"), BeamCol);
	}

	if (GrappleLight)
	{
		GrappleLight->SetWorldLocation(GrappleTarget);
	}
}

void UExoAbilityComponent::DestroyGrappleBeam()
{
	if (GrappleBeam)
	{
		GrappleBeam->DestroyComponent();
		GrappleBeam = nullptr;
	}
	if (GrappleLight)
	{
		GrappleLight->DestroyComponent();
		GrappleLight = nullptr;
	}
	GrappleBeamMat = nullptr;
}
