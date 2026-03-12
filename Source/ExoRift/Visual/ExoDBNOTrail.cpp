#include "Visual/ExoDBNOTrail.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AExoDBNOTrail::AExoDBNOTrail()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = Lifetime + 1.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));

	Mark = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mark"));
	RootComponent = Mark;
	Mark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mark->CastShadow = false;
	if (CylF.Succeeded()) Mark->SetStaticMesh(CylF.Object);

	float S = FMath::RandRange(0.15f, 0.3f);
	Mark->SetRelativeScale3D(FVector(S, S, 0.003f));
}

void AExoDBNOTrail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;

	if (Age > FadeStart)
	{
		float FadeT = (Age - FadeStart) / (Lifetime - FadeStart);
		float Scale = Mark->GetRelativeScale3D().X * (1.f - FadeT * DeltaTime);
		Mark->SetRelativeScale3D(FVector(FMath::Max(Scale, 0.001f),
			FMath::Max(Scale, 0.001f), 0.003f));
	}

	if (Age >= Lifetime) Destroy();
}

void AExoDBNOTrail::SpawnMark(UWorld* World, const FVector& Location)
{
	if (!World) return;

	// Trace down to find ground
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
	AExoDBNOTrail* Trail = World->SpawnActor<AExoDBNOTrail>(
		AExoDBNOTrail::StaticClass(), FTransform(SpawnLoc), Params);
	if (Trail)
	{
		// Red-orange energy bleed
		UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();
		if (EmMat && Trail->Mark)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmMat, Trail);
			if (!Mat) { return; }
			float R = FMath::RandRange(0.7f, 1.f);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(R * 6.f, R * 1.5f, 0.3f));
			Trail->Mark->SetMaterial(0, Mat);
		}
	}
}
