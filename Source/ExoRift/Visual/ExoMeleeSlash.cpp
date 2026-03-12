// ExoMeleeSlash.cpp — Plasma blade arc sweep VFX
#include "Visual/ExoMeleeSlash.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMeleeSlash::AExoMeleeSlash()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.5f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;

	auto MakeMesh = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Cube) C->SetStaticMesh(Cube);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		return C;
	};

	// Core flash at origin
	CoreFlash = MakeMesh(TEXT("CoreFlash"));
	RootComponent = CoreFlash;

	// Arc segments — thin blades arranged in a sweep pattern
	for (int32 i = 0; i < ARC_SEGMENTS; i++)
	{
		FName Name = *FString::Printf(TEXT("Arc_%d"), i);
		UStaticMeshComponent* Seg = MakeMesh(*Name.ToString());
		Seg->SetupAttachment(CoreFlash);
		ArcSegments.Add(Seg);
	}

	SlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SlashLight"));
	SlashLight->SetupAttachment(CoreFlash);
	SlashLight->SetIntensity(200000.f);
	SlashLight->SetAttenuationRadius(400.f);
	SlashLight->SetLightColor(FLinearColor(0.3f, 0.6f, 1.f));
	SlashLight->CastShadows = false;
}

void AExoMeleeSlash::InitSlash(const FVector& Direction)
{
	SlashDir = Direction.IsNearlyZero() ? FVector::ForwardVector : Direction.GetSafeNormal();

	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!AddMat) return;

	// Core flash — brief bright burst at origin
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(AddMat, this);
		if (!Mat) { return; }
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(60.f, 120.f, 250.f)); // Bright blue-white
		CoreFlash->SetMaterial(0, Mat);
		CoreFlash->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.05f));
	}

	// Arc segments — arranged in a ~120-degree sweep
	FVector Right, Up;
	SlashDir.FindBestAxisVectors(Right, Up);

	FLinearColor BladeCol(30.f, 80.f, 200.f);
	for (int32 i = 0; i < ARC_SEGMENTS; i++)
	{
		float Angle = -60.f + (120.f * i) / (ARC_SEGMENTS - 1); // -60 to +60 degrees
		float Rad = FMath::DegreesToRadians(Angle);

		// Direction of this segment in the sweep arc
		FVector SegDir = SlashDir * FMath::Cos(Rad) + Right * FMath::Sin(Rad);
		FVector SegPos = SegDir * (80.f + i * 15.f); // Fan outward

		ArcSegments[i]->SetRelativeLocation(SegPos);
		ArcSegments[i]->SetWorldRotation(SegDir.Rotation());

		// Thin elongated blade
		float Len = FMath::RandRange(0.8f, 1.2f);
		ArcSegments[i]->SetRelativeScale3D(FVector(Len, 0.02f, 0.005f));

		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(AddMat, this);
		if (!Mat) { continue; }
		float Brightness = 1.f - FMath::Abs(Angle) / 80.f; // Brighter at center
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			BladeCol * FMath::Max(Brightness, 0.4f));
		ArcSegments[i]->SetMaterial(0, Mat);
	}
}

void AExoMeleeSlash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;

	// Core flash shrinks quickly
	float CoreS = 0.3f * (1.f - T * T);
	CoreFlash->SetRelativeScale3D(FVector(FMath::Max(CoreS, 0.001f)));

	// Arc segments expand outward and fade
	for (int32 i = 0; i < ArcSegments.Num(); i++)
	{
		FVector Pos = ArcSegments[i]->GetRelativeLocation();
		// Push outward
		if (!Pos.IsNearlyZero())
		{
			FVector Dir = Pos.GetSafeNormal();
			ArcSegments[i]->SetRelativeLocation(Pos + Dir * 600.f * DeltaTime);
		}

		// Stretch and thin as they fly
		FVector Scale = ArcSegments[i]->GetRelativeScale3D();
		float Stretch = 1.f + T * 2.f;
		float Thin = FMath::Max(Alpha, 0.01f);
		ArcSegments[i]->SetRelativeScale3D(
			FVector(Scale.X * Stretch / FMath::Max(Stretch, 0.01f),
				Scale.Y * Thin, Scale.Z * Thin));
	}

	// Light fades
	SlashLight->SetIntensity(200000.f * Alpha * Alpha);

	if (Age >= Lifetime) Destroy();
}

void AExoMeleeSlash::SpawnSlash(UWorld* World, const FVector& Origin,
	const FVector& Direction)
{
	if (!World) return;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoMeleeSlash* FX = World->SpawnActor<AExoMeleeSlash>(
		AExoMeleeSlash::StaticClass(), Origin, FRotator::ZeroRotator, Params);
	if (FX) FX->InitSlash(Direction);
}
