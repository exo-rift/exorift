#include "Visual/ExoTracer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoTracer::AExoTracer()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.2f;

	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamMesh"));
	RootComponent = BeamMesh;
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->CastShadow = false;
	BeamMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded())
	{
		BeamMesh->SetStaticMesh(CubeFinder.Object);
	}

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(BeamMesh);
	GlowLight->SetIntensity(5000.f);
	GlowLight->SetAttenuationRadius(250.f);
	GlowLight->CastShadows = false;
	GlowLight->SetRelativeLocation(FVector::ZeroVector);
}

void AExoTracer::InitTracer(const FVector& Start, const FVector& End, bool bIsHit)
{
	// Position beam at midpoint, oriented from start to end
	FVector Mid = (Start + End) * 0.5f;
	FVector Dir = End - Start;
	float Distance = Dir.Size();
	if (Distance < 1.f) { Destroy(); return; }

	SetActorLocation(Mid);
	SetActorRotation(Dir.Rotation());

	// Scale: X = beam length, Y/Z = thin cross section
	float LengthScale = Distance / 100.f; // Cube is 100 units
	BeamMesh->SetWorldScale3D(FVector(LengthScale, 0.012f, 0.012f));

	// Color: cyan for miss, orange-red for hit
	BaseColor = bIsHit
		? FLinearColor(8.f, 1.5f, 0.3f, 1.f)
		: FLinearColor(0.5f, 4.f, 8.f, 1.f);

	// Create bright emissive dynamic material
	if (BeamMesh->GetMaterial(0))
	{
		DynMat = BeamMesh->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
			DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), BaseColor);
		}
	}

	// Light color matches tracer
	FLinearColor LightColor = bIsHit
		? FLinearColor(1.f, 0.3f, 0.05f)
		: FLinearColor(0.1f, 0.6f, 1.f);
	GlowLight->SetLightColor(LightColor);
	BaseIntensity = GlowLight->Intensity;

	// Place light near the end (impact) for maximum visual punch
	FVector LocalEnd = FVector(Distance * 0.4f, 0.f, 0.f);
	GlowLight->SetRelativeLocation(LocalEnd);
}

void AExoTracer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Fade light
	if (GlowLight)
	{
		GlowLight->SetIntensity(BaseIntensity * Alpha * Alpha);
	}

	// Fade material opacity (shrink Y/Z for visual fade)
	if (BeamMesh)
	{
		FVector Scale = BeamMesh->GetComponentScale();
		Scale.Y = 0.012f * Alpha;
		Scale.Z = 0.012f * Alpha;
		BeamMesh->SetWorldScale3D(Scale);
	}

	if (Age >= Lifetime)
	{
		Destroy();
	}
}
