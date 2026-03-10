// ExoForceFieldGate.cpp — Animated energy force field gate (visual only)
#include "Map/ExoForceFieldGate.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoForceFieldGate::AExoForceFieldGate()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubF.Succeeded()) CubeMesh = CubF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	// Materials created at runtime via FExoMaterialFactory

	auto MakeMesh = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) C->SetStaticMesh(Mesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		return C;
	};

	LeftPillar = MakeMesh(TEXT("LeftPillar"), CylinderMesh);
	RootComponent = LeftPillar;

	RightPillar = MakeMesh(TEXT("RightPillar"), CylinderMesh);
	RightPillar->SetupAttachment(LeftPillar);

	TopBeam = MakeMesh(TEXT("TopBeam"), CubeMesh);
	TopBeam->SetupAttachment(LeftPillar);

	BarrierMesh = MakeMesh(TEXT("Barrier"), CubeMesh);
	BarrierMesh->SetupAttachment(LeftPillar);

	LeftLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LeftLight"));
	LeftLight->SetupAttachment(LeftPillar);
	LeftLight->SetIntensity(15000.f);
	LeftLight->SetAttenuationRadius(1200.f);
	LeftLight->CastShadows = false;

	RightLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RightLight"));
	RightLight->SetupAttachment(LeftPillar);
	RightLight->SetIntensity(15000.f);
	RightLight->SetAttenuationRadius(1200.f);
	RightLight->CastShadows = false;
}

void AExoForceFieldGate::InitGate(float Width, float Height, const FLinearColor& Color)
{
	GateWidth = Width;
	GateHeight = Height;
	GateColor = Color;

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	auto MakeEmissiveMat = [&](const FLinearColor& Emissive) -> UMaterialInstanceDynamic*
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), Emissive);
		return M;
	};

	FLinearColor PillarCol(0.06f, 0.06f, 0.08f);
	FLinearColor PillarEm(0.01f, 0.02f, 0.03f);

	// Left pillar
	LeftPillar->SetRelativeScale3D(FVector(1.f, 1.f, Height / 100.f));
	LeftPillar->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.5f));
	LeftPillar->SetMaterial(0, MakeEmissiveMat(PillarEm));

	// Right pillar
	RightPillar->SetRelativeLocation(FVector(0.f, Width, 0.f));
	RightPillar->SetRelativeScale3D(FVector(1.f, 1.f, Height / 100.f));
	RightPillar->SetMaterial(0, MakeEmissiveMat(PillarEm));

	// Top beam connecting pillars
	TopBeam->SetRelativeLocation(FVector(0.f, Width * 0.5f, Height * 0.5f));
	TopBeam->SetRelativeScale3D(FVector(0.5f, Width / 100.f, 0.3f));
	TopBeam->SetMaterial(0, MakeEmissiveMat(PillarEm));

	// Energy barrier — thin, glowing plane filling the gate
	BarrierMesh->SetRelativeLocation(FVector(0.f, Width * 0.5f, 0.f));
	BarrierMesh->SetRelativeScale3D(FVector(0.01f, Width / 100.f, Height / 100.f));
	BarrierMat = MakeEmissiveMat(
		FLinearColor(Color.R * 8.f, Color.G * 8.f, Color.B * 8.f));
	BarrierMesh->SetMaterial(0, BarrierMat);

	// Accent lights on pillar tops
	LeftLight->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.5f));
	LeftLight->SetLightColor(Color);
	RightLight->SetRelativeLocation(FVector(0.f, Width, Height * 0.5f));
	RightLight->SetLightColor(Color);
}

void AExoForceFieldGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Barrier shimmer — wave pattern across the barrier
	if (BarrierMat)
	{
		float Wave = FMath::Sin(Time * 3.f) * 0.3f;
		float Flicker = 1.f + Wave + 0.1f * FMath::Sin(Time * 17.f);
		BarrierMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(
				GateColor.R * 8.f * Flicker,
				GateColor.G * 8.f * Flicker,
				GateColor.B * 8.f * Flicker));
	}

	// Lights pulse in sync
	float LP = 1.f + 0.3f * FMath::Sin(Time * 2.5f);
	LeftLight->SetIntensity(15000.f * LP);
	RightLight->SetIntensity(15000.f * LP);
}
