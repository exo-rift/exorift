// ExoLaunchColumn.cpp — Vertical energy burst VFX for jump pads
#include "Visual/ExoLaunchColumn.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoLaunchColumn::AExoLaunchColumn()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	auto MakeMesh = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (CylFinder.Succeeded()) C->SetStaticMesh(CylFinder.Object);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		return C;
	};

	InnerBeam = MakeMesh(TEXT("InnerBeam"));
	RootComponent = InnerBeam;

	OuterGlow = MakeMesh(TEXT("OuterGlow"));
	OuterGlow->SetupAttachment(InnerBeam);

	BaseRing = MakeMesh(TEXT("BaseRing"));
	BaseRing->SetupAttachment(InnerBeam);

	ColumnLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ColumnLight"));
	ColumnLight->SetupAttachment(InnerBeam);
	ColumnLight->SetIntensity(80000.f);
	ColumnLight->SetAttenuationRadius(2000.f);
	ColumnLight->CastShadows = false;
}

void AExoLaunchColumn::InitColumn(const FLinearColor& Color, float Height)
{
	ColumnHeight = Height;

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!BaseMat) return;

	// Inner beam — bright core shooting upward
	UMaterialInstanceDynamic* InnerMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (!InnerMat) { return; }
	FLinearColor CoreCol(Color.R * 200.f, Color.G * 200.f, Color.B * 200.f);
	InnerMat->SetVectorParameterValue(TEXT("EmissiveColor"), CoreCol);
	InnerBeam->SetMaterial(0, InnerMat);
	InnerBeam->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.01f)); // Starts small

	// Outer glow — wider, softer halo
	UMaterialInstanceDynamic* OuterMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (!OuterMat) { return; }
	FLinearColor GlowCol(Color.R * 60.f, Color.G * 60.f, Color.B * 60.f);
	OuterMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowCol);
	OuterGlow->SetMaterial(0, OuterMat);
	OuterGlow->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.01f));

	// Base ring — expanding shockwave at ground level
	UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (!RingMat) { return; }
	FLinearColor RingCol(Color.R * 120.f, Color.G * 120.f, Color.B * 120.f);
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"), RingCol);
	BaseRing->SetMaterial(0, RingMat);
	BaseRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.005f));

	ColumnLight->SetLightColor(Color);
	ColumnLight->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.3f));
}

void AExoLaunchColumn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;

	// Inner beam grows upward quickly, then fades
	float BeamH = ColumnHeight * FMath::Min(T * 3.f, 1.f); // Reaches full height at 33%
	float BeamR = 0.3f * Alpha;
	InnerBeam->SetRelativeScale3D(FVector(BeamR, BeamR, BeamH / 100.f));
	InnerBeam->SetRelativeLocation(FVector(0.f, 0.f, BeamH * 0.5f));

	// Outer glow expands outward and fades
	float OuterR = 1.2f + T * 2.f;
	float OuterH = BeamH * 0.8f;
	OuterGlow->SetRelativeScale3D(FVector(OuterR * Alpha, OuterR * Alpha, OuterH / 100.f));
	OuterGlow->SetRelativeLocation(FVector(0.f, 0.f, OuterH * 0.4f));

	// Base ring expands rapidly
	float RingR = 0.5f + T * 5.f;
	BaseRing->SetRelativeScale3D(FVector(RingR, RingR, 0.005f * Alpha));

	// Light fades with cubic falloff
	ColumnLight->SetIntensity(80000.f * Alpha * Alpha * Alpha);
	ColumnLight->SetRelativeLocation(FVector(0.f, 0.f, BeamH * 0.5f));

	if (Age >= Lifetime) Destroy();
}

void AExoLaunchColumn::SpawnColumn(UWorld* World, const FVector& Location,
	const FLinearColor& Color, float Height)
{
	if (!World) return;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoLaunchColumn* Col = World->SpawnActor<AExoLaunchColumn>(
		AExoLaunchColumn::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Col) Col->InitColumn(Color, Height);
}
