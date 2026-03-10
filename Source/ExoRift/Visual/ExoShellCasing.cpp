// ExoShellCasing.cpp — Ejected shell casing with tumbling trajectory
#include "Visual/ExoShellCasing.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoShellCasing::AExoShellCasing()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.8f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	RootComponent = CasingMesh;
	if (CubeFind.Succeeded()) CasingMesh->SetStaticMesh(CubeFind.Object);
	CasingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CasingMesh->CastShadow = false;
	CasingMesh->SetGenerateOverlapEvents(false);
	CasingMesh->SetWorldScale3D(FVector(0.015f, 0.006f, 0.006f));
}

void AExoShellCasing::InitCasing(const FVector& EjectDirection, const FLinearColor& Color)
{
	// Randomized ejection: mostly to the right and up
	Velocity = EjectDirection * FMath::RandRange(120.f, 220.f);
	Velocity.Z += FMath::RandRange(60.f, 140.f);
	Velocity += FVector(
		FMath::RandRange(-30.f, 30.f),
		FMath::RandRange(-30.f, 30.f),
		0.f);

	// Random tumble spin
	TumbleRate = FRotator(
		FMath::RandRange(400.f, 900.f),
		FMath::RandRange(200.f, 600.f),
		FMath::RandRange(100.f, 400.f));

	// Brass-metallic material with weapon tint
	{
		FLinearColor Brass(0.6f, 0.45f, 0.15f);
		FLinearColor Tinted = FMath::Lerp(Brass, Color, 0.2f);
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
			FExoMaterialFactory::GetLitEmissive(), this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Tinted);
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Tinted * 2.f);
		CasingMesh->SetMaterial(0, Mat);
	}
}

void AExoShellCasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Gravity
	Velocity.Z -= 600.f * DeltaTime;

	// Move
	FVector NewLoc = GetActorLocation() + Velocity * DeltaTime;
	SetActorLocation(NewLoc);

	// Tumble
	FRotator NewRot = GetActorRotation() + TumbleRate * DeltaTime;
	SetActorRotation(NewRot);

	// Shrink as it fades
	float Scale = FMath::Lerp(1.f, 0.3f, T * T);
	CasingMesh->SetWorldScale3D(FVector(0.015f * Scale, 0.006f * Scale, 0.006f * Scale));

	if (Age >= Lifetime) Destroy();
}
