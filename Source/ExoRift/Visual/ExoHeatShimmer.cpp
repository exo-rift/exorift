// ExoHeatShimmer.cpp — Rising heat wisps + glow from overheating weapons
#include "Visual/ExoHeatShimmer.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoHeatShimmer::AExoHeatShimmer()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));

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

	// Expanding heat ring at the base
	HeatRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeatRing"));
	HeatRing->SetupAttachment(Root);
	if (CylF.Succeeded()) HeatRing->SetStaticMesh(CylF.Object);
	HeatRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeatRing->CastShadow = false;
	HeatRing->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.001f));
	HeatRing->SetVisibility(false);

	// Orange heat glow light
	HeatLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("HeatLight"));
	HeatLight->SetupAttachment(Root);
	HeatLight->SetIntensity(0.f);
	HeatLight->SetAttenuationRadius(300.f);
	HeatLight->SetLightColor(FLinearColor(1.f, 0.5f, 0.1f));
	HeatLight->CastShadows = false;
}

void AExoHeatShimmer::InitShimmer(float Intensity)
{
	IntensityScale = FMath::Clamp(Intensity, 0.1f, 1.f);
	Lifetime = 0.4f + IntensityScale * 0.5f;

	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();

	// Heat glow light scales with intensity
	HeatLight->SetIntensity(5000.f * IntensityScale);
	HeatLight->SetAttenuationRadius(200.f + 300.f * IntensityScale);

	// Heat ring — visible only at high intensity
	if (IntensityScale > 0.5f && EmMat)
	{
		HeatRing->SetVisibility(true);
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(EmMat, this);
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(4.f * IntensityScale, 1.5f * IntensityScale, 0.3f * IntensityScale));
		HeatRing->SetMaterial(0, RingMat);
	}

	for (int32 i = 0; i < NUM_WISPS; i++)
	{
		Wisps[i]->SetVisibility(true);

		if (EmMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmMat, this);
			// Hotter wisps are brighter white-orange, cooler ones are dim red
			float HeatGradient = (float)i / (float)NUM_WISPS;
			float R = FMath::Lerp(5.f, 2.f, HeatGradient) * IntensityScale;
			float G = FMath::Lerp(3.f, 0.5f, HeatGradient) * IntensityScale;
			float B = FMath::Lerp(1.f, 0.1f, HeatGradient) * IntensityScale;
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(R, G, B));
			Wisps[i]->SetMaterial(0, Mat);
		}

		float Angle = (i / (float)NUM_WISPS) * 360.f + FMath::RandRange(-30.f, 30.f);
		float Spread = 4.f + FMath::RandRange(0.f, 6.f) * IntensityScale;
		Velocities[i] = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Spread,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Spread,
			20.f + FMath::RandRange(0.f, 15.f) * IntensityScale);

		float StartScale = 0.012f + FMath::RandRange(0.f, 0.008f);
		Wisps[i]->SetRelativeScale3D(FVector(StartScale));
		Wisps[i]->SetRelativeLocation(FVector(
			FMath::RandRange(-2.f, 2.f),
			FMath::RandRange(-2.f, 2.f), 0.f));
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

		// Rise and drift with turbulence
		FVector Pos = Wisps[i]->GetRelativeLocation();
		Pos += Velocities[i] * DeltaTime;
		// Add slight horizontal wobble for heat distortion look
		float Wobble = FMath::Sin(Age * 12.f + i * 2.f) * 2.f * IntensityScale;
		Pos.X += Wobble * DeltaTime;
		Velocities[i].Z *= 0.97f;
		Wisps[i]->SetRelativeLocation(Pos);

		// Expand as they rise — hotter wisps expand faster
		float ExpandRate = 0.05f + 0.03f * (1.f - (float)i / NUM_WISPS);
		float BaseScale = 0.012f + Age * ExpandRate * IntensityScale;
		Wisps[i]->SetRelativeScale3D(FVector(BaseScale * Alpha));

		// Fade emissive
		UMaterialInstanceDynamic* Mat = Cast<UMaterialInstanceDynamic>(Wisps[i]->GetMaterial(0));
		if (Mat)
		{
			float HeatGradient = (float)i / (float)NUM_WISPS;
			float HeatFade = Alpha * Alpha * IntensityScale;
			float R = FMath::Lerp(5.f, 2.f, HeatGradient) * HeatFade;
			float G = FMath::Lerp(3.f, 0.5f, HeatGradient) * HeatFade;
			float B = FMath::Lerp(1.f, 0.1f, HeatGradient) * HeatFade;
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(R, G, B));
		}
	}

	// Heat ring expands and fades
	if (HeatRing && HeatRing->IsVisible())
	{
		float RingT = FMath::Clamp(Age / (Lifetime * 0.5f), 0.f, 1.f);
		float RingScale = 0.15f * IntensityScale * RingT;
		HeatRing->SetRelativeScale3D(FVector(RingScale, RingScale, 0.001f * (1.f - RingT)));
		if (RingT >= 1.f) HeatRing->SetVisibility(false);
	}

	// Light fades with cubic falloff
	if (HeatLight)
	{
		HeatLight->SetIntensity(5000.f * IntensityScale * Alpha * Alpha * Alpha);
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
