// ExoHeatShimmer.cpp — Rising heat wisps from overheating weapons
#include "Visual/ExoHeatShimmer.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoHeatShimmer::AExoHeatShimmer()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.8f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	for (int32 i = 0; i < NUM_WISPS; i++)
	{
		FName Name = *FString::Printf(TEXT("Wisp%d"), i);
		Wisps[i] = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Wisps[i]->SetupAttachment(Root);
		if (SphF.Succeeded()) Wisps[i]->SetStaticMesh(SphF.Object);
		Wisps[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Wisps[i]->CastShadow = false;
		Wisps[i]->SetRelativeScale3D(FVector(0.02f));
		Wisps[i]->SetVisibility(false);
	}
}

void AExoHeatShimmer::InitShimmer(float Intensity)
{
	IntensityScale = FMath::Clamp(Intensity, 0.1f, 1.f);
	Lifetime = 0.4f + IntensityScale * 0.4f;

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();

	for (int32 i = 0; i < NUM_WISPS; i++)
	{
		Wisps[i]->SetVisibility(true);

		if (BaseMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(3.f * IntensityScale, 1.f * IntensityScale, 0.2f * IntensityScale));
			Wisps[i]->SetMaterial(0, Mat);
		}

		float Angle = (i / (float)NUM_WISPS) * 360.f + FMath::RandRange(-30.f, 30.f);
		float Spread = 3.f + FMath::RandRange(0.f, 5.f);
		Velocities[i] = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Spread,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Spread,
			15.f + FMath::RandRange(0.f, 10.f) * IntensityScale);

		float StartScale = 0.015f + FMath::RandRange(0.f, 0.01f);
		Wisps[i]->SetRelativeScale3D(FVector(StartScale));
		Wisps[i]->SetRelativeLocation(FVector(
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(-1.f, 1.f), 0.f));
	}
}

void AExoHeatShimmer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	for (int32 i = 0; i < NUM_WISPS; i++)
	{
		if (!Wisps[i]) continue;

		// Rise and drift
		FVector Pos = Wisps[i]->GetRelativeLocation();
		Pos += Velocities[i] * DeltaTime;
		Velocities[i].Z *= 0.98f; // Slow deceleration
		Wisps[i]->SetRelativeLocation(Pos);

		// Expand as they rise
		float BaseScale = 0.015f + Age * 0.04f * IntensityScale;
		Wisps[i]->SetRelativeScale3D(FVector(BaseScale));

		// Fade emissive — hot core fading out
		UMaterialInstanceDynamic* Mat = Cast<UMaterialInstanceDynamic>(Wisps[i]->GetMaterial(0));
		if (Mat)
		{
			float HeatFade = Alpha * IntensityScale;
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(3.f * HeatFade, 1.f * HeatFade, 0.2f * HeatFade));
		}
	}
}

void AExoHeatShimmer::SpawnShimmer(UWorld* World, const FVector& Pos, float Intensity)
{
	if (!World) return;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoHeatShimmer* Shimmer = World->SpawnActor<AExoHeatShimmer>(
		AExoHeatShimmer::StaticClass(), Pos, FRotator::ZeroRotator, Params);
	if (Shimmer) Shimmer->InitShimmer(Intensity);
}
