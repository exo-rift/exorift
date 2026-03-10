#include "Visual/ExoSteamVent.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoSteamVent::AExoSteamVent()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		FName Name = *FString::Printf(TEXT("Puff_%d"), i);
		UStaticMeshComponent* P = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		P->SetupAttachment(Root);
		P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		P->CastShadow = false;
		P->SetGenerateOverlapEvents(false);
		P->SetVisibility(false);
		if (SphereFinder.Succeeded()) P->SetStaticMesh(SphereFinder.Object);
		P->SetRelativeScale3D(FVector(0.01f));
		PuffMeshes.Add(P);
	}

	VentLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("VentLight"));
	VentLight->SetupAttachment(Root);
	VentLight->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	VentLight->SetIntensity(0.f);
	VentLight->SetAttenuationRadius(600.f);
	VentLight->CastShadows = false;

	// Randomize initial timer so vents don't all fire simultaneously
	Timer = FMath::RandRange(0.f, 8.f);
}

void AExoSteamVent::InitVent(float InInterval, float InDuration,
	const FLinearColor& InColor)
{
	Interval = InInterval;
	BurstDuration = InDuration;
	VentColor = InColor;

	VentLight->SetLightColor(InColor);

	UMaterialInterface* PuffMat = FExoMaterialFactory::GetEmissiveAdditive();

	for (auto* P : PuffMeshes)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(PuffMat, this);
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(InColor.R * 2.f, InColor.G * 2.f, InColor.B * 2.f));
		P->SetMaterial(0, Mat);
	}
}

void AExoSteamVent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Timer += DeltaTime;

	if (!bIsBursting && Timer >= Interval)
	{
		bIsBursting = true;
		BurstAge = 0.f;
		Timer = 0.f;
		for (auto* P : PuffMeshes) P->SetVisibility(true);
	}

	if (!bIsBursting) return;

	BurstAge += DeltaTime;
	float T = FMath::Clamp(BurstAge / BurstDuration, 0.f, 1.f);

	// Light intensity: bright flash then fade
	float LightAlpha = (T < 0.3f) ? T / 0.3f : (1.f - (T - 0.3f) / 0.7f);
	VentLight->SetIntensity(15000.f * FMath::Max(LightAlpha, 0.f));

	// Animate each puff: rise upward, expand, then fade
	for (int32 i = 0; i < PuffMeshes.Num(); i++)
	{
		float Delay = (float)i * 0.08f;
		float PuffT = FMath::Clamp((BurstAge - Delay) / (BurstDuration - Delay), 0.f, 1.f);
		if (PuffT <= 0.f)
		{
			PuffMeshes[i]->SetRelativeScale3D(FVector(0.01f));
			continue;
		}

		// Rise with some horizontal spread
		float RiseH = PuffT * (200.f + i * 80.f);
		float Spread = PuffT * 30.f * (i % 2 == 0 ? 1.f : -1.f);
		PuffMeshes[i]->SetRelativeLocation(FVector(Spread, 0.f, RiseH));

		// Expand then shrink
		float S = FMath::Sin(PuffT * PI) * (0.3f + i * 0.1f);
		PuffMeshes[i]->SetRelativeScale3D(FVector(S, S, S * 0.6f));
	}

	if (BurstAge >= BurstDuration)
	{
		bIsBursting = false;
		for (auto* P : PuffMeshes)
		{
			P->SetVisibility(false);
			P->SetRelativeScale3D(FVector(0.01f));
		}
		VentLight->SetIntensity(0.f);
	}
}
