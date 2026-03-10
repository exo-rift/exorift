// ExoFootstepDust.cpp — Multi-particle dust/spark burst at character feet
#include "Visual/ExoFootstepDust.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoFootstepDust::AExoFootstepDust()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.2f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		FName Name = *FString::Printf(TEXT("Puff%d"), i);
		Puffs[i] = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Puffs[i]->SetupAttachment(Root);
		if (SphereFinder.Succeeded()) Puffs[i]->SetStaticMesh(SphereFinder.Object);
		Puffs[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Puffs[i]->CastShadow = false;
		Puffs[i]->SetGenerateOverlapEvents(false);
		Puffs[i]->SetRelativeScale3D(FVector(0.01f));
	}

	// Subtle ground light for landing impacts
	DustLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DustLight"));
	DustLight->SetupAttachment(Root);
	DustLight->SetIntensity(0.f);
	DustLight->SetAttenuationRadius(400.f);
	DustLight->CastShadows = false;
}

void AExoFootstepDust::InitDust(bool bLanding, float Intensity, const FLinearColor& SurfaceColor)
{
	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();

	if (bLanding)
	{
		Lifetime = 0.6f + Intensity * 0.4f;
		MaxScale = 0.6f + Intensity * 0.8f;
		LightIntensity = 3000.f * Intensity;
	}
	else
	{
		Lifetime = 0.25f + Intensity * 0.15f;
		MaxScale = 0.08f + Intensity * 0.15f;
		LightIntensity = 0.f;
	}

	DustLight->SetLightColor(SurfaceColor);
	DustLight->SetIntensity(LightIntensity);

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		float Angle = (i / (float)NUM_PUFFS) * 360.f + FMath::RandRange(-40.f, 40.f);
		float Speed = (bLanding ? 80.f : 15.f) * Intensity + FMath::RandRange(0.f, 30.f);
		PuffVelocities[i] = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * Speed,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * Speed,
			(bLanding ? 40.f : 15.f) + FMath::RandRange(0.f, 20.f) * Intensity);

		// Size variation — some puffs are larger "cloud" cores, others smaller wisps
		float StartScale = 0.01f + FMath::RandRange(0.f, 0.005f);
		Puffs[i]->SetRelativeScale3D(FVector(StartScale));

		// Surface-tinted dust with subtle emissive for bloom
		if (EmMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmMat, this);
			float Brightness = 0.3f + FMath::RandRange(0.f, 0.4f);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(SurfaceColor.R * Brightness,
					SurfaceColor.G * Brightness, SurfaceColor.B * Brightness));
			Puffs[i]->SetMaterial(0, Mat);
		}

		Puffs[i]->SetRelativeLocation(FVector(
			FMath::RandRange(-5.f, 5.f),
			FMath::RandRange(-5.f, 5.f),
			FMath::RandRange(0.f, 5.f)));
	}
}

void AExoFootstepDust::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	for (int32 i = 0; i < NUM_PUFFS; i++)
	{
		if (!Puffs[i]) continue;

		// Drift outward and rise
		FVector Pos = Puffs[i]->GetRelativeLocation();
		Pos += PuffVelocities[i] * DeltaTime;
		PuffVelocities[i] *= 0.96f; // Drag
		PuffVelocities[i].Z *= 0.98f;
		Puffs[i]->SetRelativeLocation(Pos);

		// Expand quickly, then slow — each puff at slightly different rate
		float PuffPhase = FMath::Clamp(T + i * 0.05f, 0.f, 1.f);
		float Scale = MaxScale * (0.3f + 0.7f * i / (float)NUM_PUFFS) * FMath::Sqrt(PuffPhase);

		// Fade by shrinking toward end of life
		if (T > 0.5f)
		{
			float FadeT = (T - 0.5f) / 0.5f;
			Scale *= (1.f - FadeT * FadeT);
		}

		// Flatten slightly — dust clouds are wider than tall
		Puffs[i]->SetRelativeScale3D(FVector(Scale, Scale, Scale * 0.6f));

		// Fade emissive
		UMaterialInstanceDynamic* Mat = Cast<UMaterialInstanceDynamic>(Puffs[i]->GetMaterial(0));
		if (Mat)
		{
			FLinearColor Current;
			Mat->GetVectorParameterValue(TEXT("EmissiveColor"), Current);
			float Alpha = (1.f - T * T);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Current.R * Alpha, Current.G * Alpha, Current.B * Alpha));
		}
	}

	// Light fades quickly
	if (DustLight && LightIntensity > 0.f)
	{
		float LightFade = (1.f - T) * (1.f - T);
		DustLight->SetIntensity(LightIntensity * LightFade);
	}

	if (Age >= Lifetime) Destroy();
}

void AExoFootstepDust::SpawnFootstepDust(UWorld* World, const FVector& Location, bool bSprinting)
{
	if (!World) return;

	// Spawn dust for all movement, scale by intensity
	float Intensity = bSprinting ? 1.f : 0.3f;

	// Surface-type color — default grey-brown for generic/dirt
	FLinearColor DustColor(0.35f, 0.3f, 0.25f);

	// Skip very low-intensity puffs to avoid spam
	if (!bSprinting && FMath::FRand() > 0.3f) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoFootstepDust* Dust = World->SpawnActor<AExoFootstepDust>(
		AExoFootstepDust::StaticClass(), FTransform(Location), Params);
	if (Dust) Dust->InitDust(false, Intensity, DustColor);
}

void AExoFootstepDust::SpawnLandingDust(UWorld* World, const FVector& Location, float FallSpeed)
{
	if (!World) return;
	if (FallSpeed < 300.f) return;

	// Scale intensity with fall speed — heavier landings = bigger dust cloud
	float Intensity = FMath::Clamp(FallSpeed / 1200.f, 0.3f, 1.f);

	FLinearColor DustColor(0.4f, 0.35f, 0.3f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoFootstepDust* Dust = World->SpawnActor<AExoFootstepDust>(
		AExoFootstepDust::StaticClass(), FTransform(Location), Params);
	if (Dust) Dust->InitDust(true, Intensity, DustColor);
}
