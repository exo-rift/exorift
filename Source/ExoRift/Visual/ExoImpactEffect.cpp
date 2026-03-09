#include "Visual/ExoImpactEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"

static UStaticMeshComponent* CreateSparkSubobject(AActor* Owner, USceneComponent* Parent, FName Name)
{
	UStaticMeshComponent* Spark = Owner->CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Spark->SetupAttachment(Parent);
	Spark->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Spark->CastShadow = false;
	Spark->SetGenerateOverlapEvents(false);
	return Spark;
}

AExoImpactEffect::AExoImpactEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.3f;

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
	RootComponent = CoreMesh;
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreMesh->CastShadow = false;
	CoreMesh->SetGenerateOverlapEvents(false);
	CoreMesh->SetWorldScale3D(FVector(0.08f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		CoreMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Three spark shards (stretched cubes flying outward)
	SparkMesh1 = CreateSparkSubobject(this, CoreMesh, TEXT("Spark1"));
	SparkMesh2 = CreateSparkSubobject(this, CoreMesh, TEXT("Spark2"));
	SparkMesh3 = CreateSparkSubobject(this, CoreMesh, TEXT("Spark3"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded())
	{
		SparkMesh1->SetStaticMesh(CubeFinder.Object);
		SparkMesh2->SetStaticMesh(CubeFinder.Object);
		SparkMesh3->SetStaticMesh(CubeFinder.Object);
	}

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(CoreMesh);
	FlashLight->SetIntensity(8000.f);
	FlashLight->SetAttenuationRadius(300.f);
	FlashLight->CastShadows = false;
}

void AExoImpactEffect::InitEffect(const FVector& HitNormal, bool bHitCharacter)
{
	// Color: orange sparks for surfaces, red for characters
	FLinearColor SparkColor = bHitCharacter
		? FLinearColor(6.f, 0.5f, 0.3f, 1.f)
		: FLinearColor(5.f, 3.f, 1.f, 1.f);

	FLinearColor LightColor = bHitCharacter
		? FLinearColor(1.f, 0.2f, 0.1f)
		: FLinearColor(1.f, 0.7f, 0.3f);

	FlashLight->SetLightColor(LightColor);
	BaseIntensity = FlashLight->Intensity;

	// Build a tangent frame from the hit normal
	FVector Tangent, Bitangent;
	HitNormal.FindBestAxisVectors(Tangent, Bitangent);

	// Random spark velocities along the surface normal hemisphere
	auto RandHemisphere = [&]() -> FVector
	{
		FVector Base = HitNormal * FMath::RandRange(80.f, 200.f);
		Base += Tangent * FMath::RandRange(-100.f, 100.f);
		Base += Bitangent * FMath::RandRange(-100.f, 100.f);
		return Base;
	};

	SparkVel1 = RandHemisphere();
	SparkVel2 = RandHemisphere();
	SparkVel3 = RandHemisphere();

	// Thin elongated spark shards
	float SparkScale = 0.02f;
	SparkMesh1->SetWorldScale3D(FVector(0.06f, SparkScale, SparkScale));
	SparkMesh2->SetWorldScale3D(FVector(0.05f, SparkScale, SparkScale));
	SparkMesh3->SetWorldScale3D(FVector(0.04f, SparkScale, SparkScale));
}

void AExoImpactEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Fade flash light
	if (FlashLight)
	{
		// Fast initial falloff
		FlashLight->SetIntensity(BaseIntensity * Alpha * Alpha * Alpha);
	}

	// Shrink core
	if (CoreMesh)
	{
		float S = 0.08f * Alpha;
		CoreMesh->SetWorldScale3D(FVector(S));
	}

	// Move sparks outward and shrink
	auto MoveSpark = [&](UStaticMeshComponent* Spark, const FVector& Vel)
	{
		if (!Spark) return;
		FVector Offset = Vel * (Age);
		// Add gravity
		Offset.Z -= 200.f * Age * Age;
		Spark->SetRelativeLocation(Offset);
		float S = 0.04f * Alpha;
		Spark->SetWorldScale3D(FVector(S * 1.5f, S * 0.5f, S * 0.5f));
		// Orient spark along velocity
		FVector Dir = Vel + FVector(0.f, 0.f, -400.f * Age);
		if (!Dir.IsNearlyZero())
		{
			Spark->SetWorldRotation(Dir.Rotation());
		}
	};

	MoveSpark(SparkMesh1, SparkVel1);
	MoveSpark(SparkMesh2, SparkVel2);
	MoveSpark(SparkMesh3, SparkVel3);

	if (Age >= Lifetime)
	{
		Destroy();
	}
}
