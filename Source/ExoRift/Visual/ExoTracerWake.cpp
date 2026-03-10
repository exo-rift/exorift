// ExoTracerWake.cpp — Lingering energy droplets in tracer wake
#include "Visual/ExoTracerWake.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoTracerWake::AExoTracerWake()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	auto MakeMesh = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (SphereFinder.Succeeded()) C->SetStaticMesh(SphereFinder.Object);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		return C;
	};

	DotMesh = MakeMesh(TEXT("Dot"));
	RootComponent = DotMesh;

	HaloMesh = MakeMesh(TEXT("Halo"));
	HaloMesh->SetupAttachment(DotMesh);

	DotLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DotLight"));
	DotLight->SetupAttachment(DotMesh);
	DotLight->SetIntensity(5000.f);
	DotLight->SetAttenuationRadius(400.f);
	DotLight->CastShadows = false;
}

void AExoTracerWake::InitWake(const FLinearColor& Color, float Scale)
{
	BaseColor = Color;
	BaseScale = Scale;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;
	UMaterialInterface* BaseMat = MatFinder.Object;

	// Core dot — bright emissive
	DotMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor Emissive(Color.R * 50.f, Color.G * 50.f, Color.B * 50.f);
	DotMat->SetVectorParameterValue(TEXT("BaseColor"), Emissive);
	DotMat->SetVectorParameterValue(TEXT("EmissiveColor"), Emissive);
	DotMesh->SetMaterial(0, DotMat);
	DotMesh->SetRelativeScale3D(FVector(Scale));

	// Halo — larger, softer glow
	HaloMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	FLinearColor HaloCol(Color.R * 15.f, Color.G * 15.f, Color.B * 15.f);
	HaloMat->SetVectorParameterValue(TEXT("BaseColor"), HaloCol);
	HaloMat->SetVectorParameterValue(TEXT("EmissiveColor"), HaloCol);
	HaloMesh->SetMaterial(0, HaloMat);
	HaloMesh->SetRelativeScale3D(FVector(3.5f));

	DotLight->SetLightColor(Color);
	DotLight->SetIntensity(15000.f * Scale / 0.1f);
}

void AExoTracerWake::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T * T; // Quadratic fade

	// Shrink and dim
	float S = BaseScale * Alpha;
	DotMesh->SetRelativeScale3D(FVector(S));
	HaloMesh->SetRelativeScale3D(FVector(2.5f * Alpha));

	if (DotMat)
	{
		FLinearColor Em(BaseColor.R * 50.f * Alpha, BaseColor.G * 50.f * Alpha,
			BaseColor.B * 50.f * Alpha);
		DotMat->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
	}
	if (HaloMat)
	{
		FLinearColor HEm(BaseColor.R * 15.f * Alpha, BaseColor.G * 15.f * Alpha,
			BaseColor.B * 15.f * Alpha);
		HaloMat->SetVectorParameterValue(TEXT("EmissiveColor"), HEm);
	}

	DotLight->SetIntensity(15000.f * Alpha * BaseScale / 0.1f);

	if (Age >= Lifetime) Destroy();
}

void AExoTracerWake::SpawnWake(UWorld* World, const FVector& Start, const FVector& End,
	const FLinearColor& Color, float Spacing)
{
	if (!World) return;

	FVector Dir = End - Start;
	float Dist = Dir.Size();
	if (Dist < Spacing) return;

	Dir.Normalize();
	int32 NumDots = FMath::Min(FMath::FloorToInt32(Dist / Spacing), 40);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 1; i <= NumDots; i++)
	{
		FVector Pos = Start + Dir * (i * Spacing);
		// Slight random offset for organic feel
		Pos += FVector(
			FMath::RandRange(-15.f, 15.f),
			FMath::RandRange(-15.f, 15.f),
			FMath::RandRange(-10.f, 10.f));

		AExoTracerWake* Wake = World->SpawnActor<AExoTracerWake>(
			AExoTracerWake::StaticClass(), Pos, FRotator::ZeroRotator, Params);
		if (Wake)
		{
			float Scale = FMath::RandRange(0.10f, 0.20f);
			Wake->InitWake(Color, Scale);
		}
	}
}
