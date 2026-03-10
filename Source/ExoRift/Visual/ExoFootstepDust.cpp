#include "Visual/ExoFootstepDust.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoFootstepDust::AExoFootstepDust()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.f;

	PuffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PuffMesh"));
	RootComponent = PuffMesh;
	PuffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PuffMesh->CastShadow = false;
	PuffMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		PuffMesh->SetStaticMesh(SphereFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		UMaterialInstanceDynamic* DustMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		FLinearColor DustColor(0.35f, 0.3f, 0.25f, 1.f);
		DustMat->SetVectorParameterValue(TEXT("BaseColor"), DustColor);
		PuffMesh->SetMaterial(0, DustMat);
	}

	PuffMesh->SetWorldScale3D(FVector(0.01f));
}

void AExoFootstepDust::InitDust(bool bLanding)
{
	if (bLanding)
	{
		Lifetime = 0.7f;
		MaxScale = 1.2f;
		RiseSpeed = 50.f;
	}
	else
	{
		Lifetime = 0.4f;
		MaxScale = 0.25f;
		RiseSpeed = 25.f;
	}
}

void AExoFootstepDust::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Expand quickly, then slow
	float Scale = MaxScale * FMath::Sqrt(T);
	PuffMesh->SetWorldScale3D(FVector(Scale, Scale, Scale * 0.5f));

	// Rise gently
	FVector Loc = GetActorLocation();
	Loc.Z += RiseSpeed * DeltaTime;
	SetActorLocation(Loc);

	// Fade by shrinking toward end of life
	if (T > 0.6f)
	{
		float FadeT = (T - 0.6f) / 0.4f;
		float FadeScale = Scale * (1.f - FadeT);
		PuffMesh->SetWorldScale3D(FVector(FadeScale, FadeScale, FadeScale * 0.5f));
	}

	if (Age >= Lifetime) Destroy();
}

void AExoFootstepDust::SpawnFootstepDust(UWorld* World, const FVector& Location, bool bSprinting)
{
	if (!World) return;
	// Only spawn for sprinting to keep it subtle
	if (!bSprinting) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoFootstepDust* Dust = World->SpawnActor<AExoFootstepDust>(
		AExoFootstepDust::StaticClass(), FTransform(Location), Params);
	if (Dust) Dust->InitDust(false);
}

void AExoFootstepDust::SpawnLandingDust(UWorld* World, const FVector& Location, float FallSpeed)
{
	if (!World) return;
	// Only spawn dust for meaningful falls (not stepping off small ledges)
	if (FallSpeed < 400.f) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoFootstepDust* Dust = World->SpawnActor<AExoFootstepDust>(
		AExoFootstepDust::StaticClass(), FTransform(Location), Params);
	if (Dust) Dust->InitDust(true);
}
