// ExoGrenadeTrail.cpp — Fading trail spheres along grenade flight path
#include "Visual/ExoGrenadeTrail.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoGrenadeTrail::AExoGrenadeTrail()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SF.Succeeded()) SphereMesh = SF.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MF.Succeeded()) BaseMaterial = MF.Object;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

void AExoGrenadeTrail::InitTrail(AActor* Parent, EGrenadeType Type)
{
	TrackedActor = Parent;

	switch (Type)
	{
	case EGrenadeType::Frag:  TrailColor = FLinearColor(1.f, 0.4f, 0.1f); break;
	case EGrenadeType::EMP:   TrailColor = FLinearColor(0.2f, 0.5f, 1.f); break;
	case EGrenadeType::Smoke: TrailColor = FLinearColor(0.6f, 0.6f, 0.6f); break;
	default:                  TrailColor = FLinearColor(1.f, 0.4f, 0.1f); break;
	}

	if (!SphereMesh || !BaseMaterial) return;

	// Pre-allocate dot pool
	Dots.SetNum(MAX_DOTS);
	DotAges.SetNum(MAX_DOTS);
	for (int32 i = 0; i < MAX_DOTS; i++)
	{
		UStaticMeshComponent* D = NewObject<UStaticMeshComponent>(this);
		D->SetupAttachment(RootComponent);
		D->SetStaticMesh(SphereMesh);
		D->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		D->CastShadow = false;
		D->SetRelativeScale3D(FVector(0.06f));
		D->SetVisibility(false);
		D->RegisterComponent();

		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		FLinearColor Em(TrailColor.R * 8.f, TrailColor.G * 8.f, TrailColor.B * 8.f);
		M->SetVectorParameterValue(TEXT("BaseColor"), TrailColor);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		D->SetMaterial(0, M);

		Dots[i] = D;
		DotAges[i] = -1.f; // Inactive
	}
}

void AExoGrenadeTrail::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;

	// Spawn new trail dots while grenade exists
	bool bParentAlive = TrackedActor.IsValid() && !TrackedActor->IsActorBeingDestroyed();
	if (bParentAlive)
	{
		SpawnTimer += DeltaTime;
		while (SpawnTimer >= SpawnInterval)
		{
			SpawnTimer -= SpawnInterval;

			// Place a dot at the grenade's current position
			int32 Idx = NextDotIndex;
			NextDotIndex = (NextDotIndex + 1) % MAX_DOTS;

			FVector Pos = TrackedActor->GetActorLocation();
			Dots[Idx]->SetWorldLocation(Pos);
			Dots[Idx]->SetVisibility(true);
			Dots[Idx]->SetRelativeScale3D(FVector(0.06f));
			DotAges[Idx] = 0.f;
		}
	}

	// Update existing dots — fade and shrink
	float DotLifetime = 0.8f;
	bool bAnyActive = false;
	for (int32 i = 0; i < MAX_DOTS; i++)
	{
		if (DotAges[i] < 0.f) continue;
		DotAges[i] += DeltaTime;

		float T = FMath::Clamp(DotAges[i] / DotLifetime, 0.f, 1.f);
		float Alpha = 1.f - T * T;

		if (Alpha <= 0.01f)
		{
			Dots[i]->SetVisibility(false);
			DotAges[i] = -1.f;
			continue;
		}

		bAnyActive = true;
		float S = 0.06f * Alpha;
		Dots[i]->SetRelativeScale3D(FVector(S));

		UMaterialInstanceDynamic* M = Cast<UMaterialInstanceDynamic>(Dots[i]->GetMaterial(0));
		if (M)
		{
			FLinearColor Em(TrailColor.R * 8.f * Alpha, TrailColor.G * 8.f * Alpha,
				TrailColor.B * 8.f * Alpha);
			M->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		}
	}

	// Self-destruct when parent gone and all dots faded
	if (!bParentAlive && !bAnyActive)
	{
		Destroy();
	}
}
