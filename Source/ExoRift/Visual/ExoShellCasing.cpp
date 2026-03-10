// ExoShellCasing.cpp — Ejected energy shell casing with hot glowing tip
#include "Visual/ExoShellCasing.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoShellCasing::AExoShellCasing()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));

	// Main casing body
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	RootComponent = CasingMesh;
	if (CubeFind.Succeeded()) CasingMesh->SetStaticMesh(CubeFind.Object);
	CasingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CasingMesh->CastShadow = false;
	CasingMesh->SetGenerateOverlapEvents(false);
	CasingMesh->SetWorldScale3D(FVector(0.018f, 0.007f, 0.007f));

	// Hot glowing tip — the business end that was just fired
	HotTip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HotTip"));
	HotTip->SetupAttachment(CasingMesh);
	if (CylFind.Succeeded()) HotTip->SetStaticMesh(CylFind.Object);
	HotTip->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HotTip->CastShadow = false;
	HotTip->SetGenerateOverlapEvents(false);
	HotTip->SetRelativeLocation(FVector(45.f, 0.f, 0.f));
	HotTip->SetRelativeScale3D(FVector(0.3f, 1.2f, 1.2f));

	// Tiny point light for the hot tip
	CasingLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CasingLight"));
	CasingLight->SetupAttachment(CasingMesh);
	CasingLight->SetRelativeLocation(FVector(45.f, 0.f, 0.f));
	CasingLight->SetIntensity(800.f);
	CasingLight->SetAttenuationRadius(80.f);
	CasingLight->CastShadows = false;
}

void AExoShellCasing::InitCasing(const FVector& EjectDirection, const FLinearColor& Color)
{
	TipColor = Color;

	// Randomized ejection: mostly to the right and up
	Velocity = EjectDirection * FMath::RandRange(140.f, 250.f);
	Velocity.Z += FMath::RandRange(80.f, 160.f);
	Velocity += FVector(
		FMath::RandRange(-30.f, 30.f),
		FMath::RandRange(-30.f, 30.f),
		0.f);

	// Random tumble spin
	TumbleRate = FRotator(
		FMath::RandRange(500.f, 1000.f),
		FMath::RandRange(300.f, 700.f),
		FMath::RandRange(150.f, 500.f));

	// Brass-metallic body with weapon tint
	FLinearColor Brass(0.55f, 0.42f, 0.15f);
	FLinearColor Tinted = FMath::Lerp(Brass, Color, 0.2f);
	UMaterialInstanceDynamic* BodyMat = UMaterialInstanceDynamic::Create(
		FExoMaterialFactory::GetLitEmissive(), this);
	BodyMat->SetVectorParameterValue(TEXT("BaseColor"), Tinted);
	BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), Tinted * 3.f);
	CasingMesh->SetMaterial(0, BodyMat);

	// Hot tip — weapon-colored emissive glow
	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (EmMat)
	{
		UMaterialInstanceDynamic* TipMat = UMaterialInstanceDynamic::Create(EmMat, this);
		TipMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 8.f, Color.G * 8.f, Color.B * 8.f));
		HotTip->SetMaterial(0, TipMat);
	}

	CasingLight->SetLightColor(Color);
	CasingLight->SetIntensity(1200.f);
}

void AExoShellCasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Gravity
	Velocity.Z -= 700.f * DeltaTime;

	// Air drag
	Velocity *= (1.f - 0.5f * DeltaTime);

	// Move
	FVector NewLoc = GetActorLocation() + Velocity * DeltaTime;
	SetActorLocation(NewLoc);

	// Tumble
	FRotator NewRot = GetActorRotation() + TumbleRate * DeltaTime;
	SetActorRotation(NewRot);

	// Shrink as it fades
	float Scale = FMath::Lerp(1.f, 0.3f, T * T);
	CasingMesh->SetWorldScale3D(FVector(0.018f * Scale, 0.007f * Scale, 0.007f * Scale));

	// Hot tip cools down — emissive fades
	float HeatFade = (1.f - T) * (1.f - T);
	UMaterialInstanceDynamic* TipMat = Cast<UMaterialInstanceDynamic>(HotTip->GetMaterial(0));
	if (TipMat)
	{
		TipMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(TipColor.R * 8.f * HeatFade,
				TipColor.G * 8.f * HeatFade, TipColor.B * 8.f * HeatFade));
	}

	// Light dims as tip cools
	CasingLight->SetIntensity(1200.f * HeatFade);

	if (Age >= Lifetime) Destroy();
}
