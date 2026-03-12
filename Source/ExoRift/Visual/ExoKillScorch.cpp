#include "Visual/ExoKillScorch.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoKillScorch::AExoKillScorch()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = TotalLifetime + 1.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	BurnMark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BurnMark"));
	RootComponent = BurnMark;
	BurnMark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BurnMark->CastShadow = false;
	if (CylFinder.Succeeded()) BurnMark->SetStaticMesh(CylFinder.Object);
	BurnMark->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.005f));

	OuterRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterRing"));
	OuterRing->SetupAttachment(BurnMark);
	OuterRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OuterRing->CastShadow = false;
	if (CylFinder.Succeeded()) OuterRing->SetStaticMesh(CylFinder.Object);
	OuterRing->SetRelativeScale3D(FVector(2.5f, 2.5f, 0.3f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	EmberCore = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmberCore"));
	EmberCore->SetupAttachment(BurnMark);
	EmberCore->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EmberCore->CastShadow = false;
	if (SphFinder.Succeeded()) EmberCore->SetStaticMesh(SphFinder.Object);
	EmberCore->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.1f));
	EmberCore->SetRelativeLocation(FVector(0.f, 0.f, 5.f));

	ScorchLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ScorchLight"));
	ScorchLight->SetupAttachment(BurnMark);
	ScorchLight->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	ScorchLight->SetIntensity(50000.f);
	ScorchLight->SetAttenuationRadius(600.f);
	ScorchLight->SetLightColor(FLinearColor(1.f, 0.3f, 0.05f));
	ScorchLight->CastShadows = false;
}

void AExoKillScorch::Init(bool bHeadshot)
{
	bIsHeadshot = bHeadshot;

	// Trace down to find ground
	FHitResult Hit;
	FVector Start = GetActorLocation() + FVector(0.f, 0.f, 50.f);
	FVector End = GetActorLocation() - FVector(0.f, 0.f, 200.f);
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, 1.f));
	}

	float ScorchScale = bHeadshot ? 2.2f : 1.5f;
	BurnMark->SetRelativeScale3D(FVector(ScorchScale, ScorchScale, 0.005f));
	OuterRing->SetRelativeScale3D(FVector(ScorchScale * 1.8f, ScorchScale * 1.8f, 0.3f));

	FLinearColor ScorchColor = bHeadshot
		? FLinearColor(1.f, 0.15f, 0.02f)   // Orange-red for headshot
		: FLinearColor(0.8f, 0.25f, 0.05f);  // Warm orange for normal

	// Burn mark — dark charred surface
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat)
	{
		BurnMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!BurnMat) { return; }
		BurnMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.01f, 0.01f, 0.01f));
		BurnMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ScorchColor.R * 3.f, ScorchColor.G * 3.f, ScorchColor.B * 3.f));
		BurnMat->SetScalarParameterValue(TEXT("Roughness"), 0.95f);
		BurnMark->SetMaterial(0, BurnMat);
	}

	// Outer ring — expanding emissive bloom
	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (AddMat)
	{
		RingMat = UMaterialInstanceDynamic::Create(AddMat, this);
		if (!RingMat) { return; }
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ScorchColor.R * 80.f, ScorchColor.G * 80.f, ScorchColor.B * 80.f));
		OuterRing->SetMaterial(0, RingMat);
	}

	// Ember core — hot center glow
	UMaterialInterface* OpaqueMat = FExoMaterialFactory::GetEmissiveOpaque();
	if (OpaqueMat)
	{
		EmberMat = UMaterialInstanceDynamic::Create(OpaqueMat, this);
		if (!EmberMat) { return; }
		EmberMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ScorchColor.R * 60.f, ScorchColor.G * 60.f, ScorchColor.B * 60.f));
		EmberCore->SetMaterial(0, EmberMat);
	}

	// Headshot gets brighter initial light
	if (bHeadshot)
	{
		ScorchLight->SetIntensity(120000.f);
		ScorchLight->SetAttenuationRadius(900.f);
	}
}

void AExoKillScorch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;

	// Phase 1: Ember glow fades (0 to EmberDuration)
	if (Age < EmberDuration)
	{
		float T = Age / EmberDuration;
		float EmberFade = 1.f - T * T; // Quadratic falloff

		// Light fades
		float BaseLightIntensity = bIsHeadshot ? 120000.f : 50000.f;
		ScorchLight->SetIntensity(BaseLightIntensity * EmberFade);

		// Outer ring expands and fades
		float RingScale = (bIsHeadshot ? 2.2f : 1.5f) * 1.8f;
		float Expand = RingScale * (1.f + T * 0.6f);
		float RingFade = EmberFade;
		OuterRing->SetRelativeScale3D(FVector(Expand, Expand, 0.3f * RingFade));

		// Ember core shrinks
		float CoreScale = 0.3f * EmberFade;
		EmberCore->SetRelativeScale3D(FVector(CoreScale, CoreScale, 0.1f * EmberFade));

		// Burn mark emissive cools
		if (BurnMat)
		{
			FLinearColor ScorchColor = bIsHeadshot
				? FLinearColor(1.f, 0.15f, 0.02f)
				: FLinearColor(0.8f, 0.25f, 0.05f);
			float Em = EmberFade * 3.f;
			BurnMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(ScorchColor.R * Em, ScorchColor.G * Em, ScorchColor.B * Em));
		}
	}
	else
	{
		// After ember phase — static dark scorch, no light
		ScorchLight->SetIntensity(0.f);
		OuterRing->SetRelativeScale3D(FVector(0.001f));
		EmberCore->SetRelativeScale3D(FVector(0.001f));

		if (BurnMat && Age < EmberDuration + 0.1f)
		{
			BurnMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		}
	}

	// Phase 2: Scorch mark fades near end of life
	if (Age > FadeStart)
	{
		float FadeT = (Age - FadeStart) / (TotalLifetime - FadeStart);
		float Scale = BurnMark->GetRelativeScale3D().X * (1.f - FadeT * DeltaTime * 0.5f);
		BurnMark->SetRelativeScale3D(FVector(FMath::Max(Scale, 0.01f),
			FMath::Max(Scale, 0.01f), 0.005f));
	}

	if (Age >= TotalLifetime) Destroy();
}

void AExoKillScorch::SpawnScorch(UWorld* World, const FVector& Location, bool bHeadshot)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoKillScorch* Scorch = World->SpawnActor<AExoKillScorch>(
		AExoKillScorch::StaticClass(), FTransform(Location), Params);
	if (Scorch) Scorch->Init(bHeadshot);
}
