// ExoSlideTrail.cpp — Flat energy streak VFX for player sliding
#include "Visual/ExoSlideTrail.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoSlideTrail::AExoSlideTrail()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = Lifetime + 0.1f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	Streak = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Streak"));
	RootComponent = Streak;
	Streak->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Streak->CastShadow = false;
	Streak->SetGenerateOverlapEvents(false);
	if (CubeFinder.Succeeded()) Streak->SetStaticMesh(CubeFinder.Object);
	Streak->SetRelativeScale3D(BaseScale);

	GroundLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GroundLight"));
	GroundLight->SetupAttachment(Streak);
	GroundLight->SetRelativeLocation(FVector(0.f, 0.f, 15.f));
	GroundLight->SetIntensity(BaseLightIntensity);
	GroundLight->SetAttenuationRadius(350.f);
	GroundLight->SetLightColor(FLinearColor(0.1f, 0.6f, 1.f));
	GroundLight->CastShadows = false;
}

void AExoSlideTrail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;

	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Slight expansion: length grows ~30%, width stays
	float LenScale = BaseScale.X * (1.f + T * 0.3f);
	float WidScale = BaseScale.Y;
	// Fade by flattening further and shrinking width toward end
	float FadeAlpha = 1.f - T * T; // Quadratic falloff
	float ZScale = BaseScale.Z * FadeAlpha;
	WidScale *= (1.f - T * 0.4f);

	Streak->SetRelativeScale3D(FVector(
		FMath::Max(LenScale, 0.001f),
		FMath::Max(WidScale, 0.001f),
		FMath::Max(ZScale, 0.001f)));

	// Fade emissive color
	if (StreakMat)
	{
		StreakMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(
				EmissiveColor.R * FadeAlpha,
				EmissiveColor.G * FadeAlpha,
				EmissiveColor.B * FadeAlpha));
	}

	// Light fades
	GroundLight->SetIntensity(BaseLightIntensity * FadeAlpha);

	if (Age >= Lifetime) Destroy();
}

void AExoSlideTrail::SpawnMark(UWorld* World, const FVector& Location)
{
	if (!World) return;

	// Trace down to find ground surface
	FHitResult Hit;
	FVector Start = Location + FVector(0.f, 0.f, 20.f);
	FVector End = Location - FVector(0.f, 0.f, 200.f);
	FVector SpawnLoc = Location;
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 1.f);
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoSlideTrail* Trail = World->SpawnActor<AExoSlideTrail>(
		AExoSlideTrail::StaticClass(), FTransform(SpawnLoc), Params);
	if (!Trail) return;

	// Cyan-blue energy streak — sci-fi theme
	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (AddMat && Trail->Streak)
	{
		Trail->StreakMat = UMaterialInstanceDynamic::Create(AddMat, Trail);
		if (!Trail->StreakMat) { return; }
		// Bright cyan-blue with high emissive multiplier for bloom
		Trail->EmissiveColor = FLinearColor(0.4f * 12.f, 0.85f * 12.f, 1.f * 12.f);
		Trail->StreakMat->SetVectorParameterValue(TEXT("EmissiveColor"), Trail->EmissiveColor);
		Trail->Streak->SetMaterial(0, Trail->StreakMat);
	}

	// Add slight random yaw variation so trails don't stack identically
	float RandomYaw = FMath::RandRange(-15.f, 15.f);
	Trail->SetActorRotation(FRotator(0.f, RandomYaw, 0.f));
}
