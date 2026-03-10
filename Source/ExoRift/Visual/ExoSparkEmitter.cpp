// ExoSparkEmitter.cpp — Periodic electrical spark bursts
#include "Visual/ExoSparkEmitter.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoSparkEmitter::AExoSparkEmitter()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CF.Succeeded()) CubeMesh = CF.Object;

	// Materials created at runtime via FExoMaterialFactory

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Flash"));
	FlashLight->SetupAttachment(Root);
	FlashLight->SetIntensity(0.f);
	FlashLight->SetAttenuationRadius(800.f);
	FlashLight->CastShadows = false;
}

void AExoSparkEmitter::InitSparks(const FLinearColor& Color, float BurstInterval)
{
	SparkColor = Color;
	Interval = BurstInterval;
	Timer = FMath::RandRange(0.f, Interval); // Stagger initial burst
	SparkOrigin = GetActorLocation();
	FlashLight->SetLightColor(Color);

	if (!CubeMesh) return;

	UMaterialInterface* SparkMat = FExoMaterialFactory::GetEmissiveAdditive();
	SparkMeshes.SetNum(MAX_SPARKS);
	Sparks.SetNum(MAX_SPARKS);
	for (int32 i = 0; i < MAX_SPARKS; i++)
	{
		UStaticMeshComponent* S = NewObject<UStaticMeshComponent>(this);
		S->SetupAttachment(RootComponent);
		S->SetStaticMesh(CubeMesh);
		S->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		S->CastShadow = false;
		S->SetRelativeScale3D(FVector(0.03f, 0.015f, 0.015f));
		S->SetVisibility(false);
		S->RegisterComponent();

		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(SparkMat, this);
		FLinearColor Em(Color.R * 30.f, Color.G * 30.f, Color.B * 30.f);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		S->SetMaterial(0, M);

		SparkMeshes[i] = S;
		Sparks[i].Age = -1.f;
	}
}

void AExoSparkEmitter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Timer += DeltaTime;

	// Trigger a burst
	if (Timer >= Interval)
	{
		Timer -= Interval;
		// Random jitter so it doesn't feel mechanical
		Timer -= FMath::RandRange(0.f, Interval * 0.3f);

		FlashDecay = 1.f;
		FlashLight->SetIntensity(40000.f);

		for (int32 i = 0; i < MAX_SPARKS; i++)
		{
			Sparks[i].Age = 0.f;
			// Random outward spray
			FVector Dir = FMath::VRandCone(GetActorForwardVector(), FMath::DegreesToRadians(60.f));
			Sparks[i].Velocity = Dir * FMath::RandRange(400.f, 1200.f);

			SparkMeshes[i]->SetWorldLocation(SparkOrigin);
			SparkMeshes[i]->SetVisibility(true);
			SparkMeshes[i]->SetRelativeScale3D(FVector(0.03f, 0.015f, 0.015f));
		}
	}

	// Flash decay
	if (FlashDecay > 0.f)
	{
		FlashDecay = FMath::Max(0.f, FlashDecay - DeltaTime * 8.f);
		FlashLight->SetIntensity(40000.f * FlashDecay * FlashDecay);
	}

	// Update active sparks
	for (int32 i = 0; i < MAX_SPARKS; i++)
	{
		if (Sparks[i].Age < 0.f) continue;
		Sparks[i].Age += DeltaTime;

		if (Sparks[i].Age > SparkLifetime)
		{
			SparkMeshes[i]->SetVisibility(false);
			Sparks[i].Age = -1.f;
			continue;
		}

		// Gravity + drag
		Sparks[i].Velocity.Z -= 980.f * DeltaTime;
		Sparks[i].Velocity *= FMath::Pow(0.95f, DeltaTime * 60.f);

		FVector Pos = SparkMeshes[i]->GetComponentLocation();
		Pos += Sparks[i].Velocity * DeltaTime;
		SparkMeshes[i]->SetWorldLocation(Pos);

		// Orient along velocity
		SparkMeshes[i]->SetWorldRotation(Sparks[i].Velocity.Rotation());

		// Fade and shrink
		float T = Sparks[i].Age / SparkLifetime;
		float Alpha = 1.f - T;
		SparkMeshes[i]->SetRelativeScale3D(FVector(0.03f * Alpha, 0.015f * Alpha, 0.015f * Alpha));

		UMaterialInstanceDynamic* M = Cast<UMaterialInstanceDynamic>(SparkMeshes[i]->GetMaterial(0));
		if (M)
		{
			FLinearColor Em(SparkColor.R * 30.f * Alpha, SparkColor.G * 30.f * Alpha,
				SparkColor.B * 30.f * Alpha);
			M->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		}
	}
}
