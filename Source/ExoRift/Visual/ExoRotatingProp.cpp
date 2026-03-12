// ExoRotatingProp.cpp — Animated rotating environmental props
#include "Visual/ExoRotatingProp.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoRotatingProp::AExoRotatingProp()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));

	UStaticMesh* Cube = CubeF.Succeeded() ? CubeF.Object : nullptr;
	UStaticMesh* Cyl = CylF.Succeeded() ? CylF.Object : nullptr;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	RootComponent = BaseMesh;
	if (Cyl) BaseMesh->SetStaticMesh(Cyl);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseMesh->CastShadow = false;
	BaseMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.05f));

	SpinningPart = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Spinner"));
	SpinningPart->SetupAttachment(BaseMesh);
	if (Cube) SpinningPart->SetStaticMesh(Cube);
	SpinningPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpinningPart->CastShadow = false;
	SpinningPart->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

	AccentLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Accent"));
	AccentLight->SetupAttachment(SpinningPart);
	AccentLight->SetIntensity(7000.f);
	AccentLight->SetAttenuationRadius(900.f);
	AccentLight->CastShadows = false;
}

void AExoRotatingProp::InitProp(int32 PropType, const FLinearColor& AccentColor,
	float RotSpeed, float PropScale)
{
	Type = PropType;
	RotationSpeed = RotSpeed;
	AccentLight->SetLightColor(AccentColor);

	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (!LitMat) return;

	FLinearColor DarkMetal(0.05f, 0.05f, 0.07f);

	// Base pillar
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!M) { return; }
		M->SetVectorParameterValue(TEXT("BaseColor"), DarkMetal);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		M->SetScalarParameterValue(TEXT("Metallic"), 0.88f);
		M->SetScalarParameterValue(TEXT("Roughness"), 0.25f);
		BaseMesh->SetMaterial(0, M);
		BaseMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.05f) * PropScale);
	}

	UMaterialInstanceDynamic* SpinMat = nullptr;

	switch (PropType)
	{
	case 0: // Fan — flat blades rotating horizontally
		SpinMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!SpinMat) { return; }
		SpinningPart->SetRelativeScale3D(FVector(3.f, 0.1f, 0.5f) * PropScale);
		SpinningPart->SetRelativeLocation(FVector(0.f, 0.f, 60.f * PropScale));
		SpinMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.06f, 0.06f, 0.08f));
		SpinMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		SpinMat->SetScalarParameterValue(TEXT("Metallic"), 0.85f);
		SpinMat->SetScalarParameterValue(TEXT("Roughness"), 0.3f);
		BobAmplitude = 0.f;
		BobFrequency = 0.f;
		break;

	case 1: // Radar dish — tilted disk rotating
		SpinMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!SpinMat) { return; }
		SpinningPart->SetRelativeScale3D(FVector(2.f, 2.f, 0.08f) * PropScale);
		SpinningPart->SetRelativeLocation(FVector(0.f, 0.f, 80.f * PropScale));
		SpinningPart->SetRelativeRotation(FRotator(25.f, 0.f, 0.f));
		SpinMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.07f, 0.07f, 0.09f));
		SpinMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		SpinMat->SetScalarParameterValue(TEXT("Metallic"), 0.85f);
		SpinMat->SetScalarParameterValue(TEXT("Roughness"), 0.3f);
		BobAmplitude = 0.f;
		BobFrequency = 0.f;
		break;

	case 2: // Energy coil — glowing ring that spins and bobs
		SpinMat = UMaterialInstanceDynamic::Create(
			FExoMaterialFactory::GetEmissiveAdditive(), this);
		if (!SpinMat) { return; }
		SpinningPart->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.15f) * PropScale);
		SpinningPart->SetRelativeLocation(FVector(0.f, 0.f, 100.f * PropScale));
		SpinMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(AccentColor.R * 18.f, AccentColor.G * 18.f, AccentColor.B * 18.f));
		BobAmplitude = 15.f * PropScale;
		BobFrequency = 1.5f;
		AccentLight->SetIntensity(14000.f);
		break;
	}

	if (SpinMat) SpinningPart->SetMaterial(0, SpinMat);
}

void AExoRotatingProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentAngle += RotationSpeed * DeltaTime;
	if (CurrentAngle > 360.f) CurrentAngle -= 360.f;

	FRotator BaseRot = FRotator::ZeroRotator;
	if (Type == 1) // Radar keeps its tilt
		BaseRot = FRotator(25.f, CurrentAngle, 0.f);
	else
		BaseRot = FRotator(0.f, CurrentAngle, 0.f);

	SpinningPart->SetRelativeRotation(BaseRot);

	// Vertical bob for energy coils
	if (BobAmplitude > 0.f)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Bob = FMath::Sin(Time * BobFrequency * 2.f * PI) * BobAmplitude;
		FVector Loc = SpinningPart->GetRelativeLocation();
		Loc.Z = 100.f + Bob;
		SpinningPart->SetRelativeLocation(Loc);
	}
}
