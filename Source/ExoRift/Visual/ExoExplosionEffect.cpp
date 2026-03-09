#include "Visual/ExoExplosionEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"

static constexpr int32 NUM_DEBRIS = 6;

AExoExplosionEffect::AExoExplosionEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.0f;

	// Fireball — scaled sphere
	FireballMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fireball"));
	RootComponent = FireballMesh;
	FireballMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FireballMesh->CastShadow = false;
	FireballMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		FireballMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Shockwave ring — flattened cylinder
	ShockwaveRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShockwaveRing"));
	ShockwaveRing->SetupAttachment(FireballMesh);
	ShockwaveRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShockwaveRing->CastShadow = false;
	ShockwaveRing->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		ShockwaveRing->SetStaticMesh(CylinderFinder.Object);
	}

	// Explosion light
	ExplosionLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ExplosionLight"));
	ExplosionLight->SetupAttachment(FireballMesh);
	ExplosionLight->SetIntensity(50000.f);
	ExplosionLight->SetAttenuationRadius(2000.f);
	ExplosionLight->SetLightColor(FLinearColor(1.f, 0.5f, 0.1f));
	ExplosionLight->CastShadows = false;

	// Debris chunks — cubes flying outward
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	for (int32 i = 0; i < NUM_DEBRIS; i++)
	{
		FName Name = *FString::Printf(TEXT("Debris_%d"), i);
		UStaticMeshComponent* Debris = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Debris->SetupAttachment(FireballMesh);
		Debris->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Debris->CastShadow = false;
		Debris->SetGenerateOverlapEvents(false);
		if (CubeFinder.Succeeded())
		{
			Debris->SetStaticMesh(CubeFinder.Object);
		}
		DebrisMeshes.Add(Debris);
	}
}

void AExoExplosionEffect::InitExplosion(float Radius)
{
	ExpRadius = Radius;

	// Start fireball small
	float InitScale = Radius / 5000.f;
	FireballMesh->SetWorldScale3D(FVector(InitScale));

	// Shockwave ring starts invisible (tiny)
	ShockwaveRing->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	BaseIntensity = ExplosionLight->Intensity;

	// Random debris velocities
	DebrisVelocities.SetNum(DebrisMeshes.Num());
	for (int32 i = 0; i < DebrisMeshes.Num(); i++)
	{
		FVector Vel = FMath::VRand() * FMath::RandRange(300.f, 800.f);
		Vel.Z = FMath::Abs(Vel.Z) + 200.f; // Bias upward
		DebrisVelocities[i] = Vel;

		float S = FMath::RandRange(0.04f, 0.12f);
		DebrisMeshes[i]->SetWorldScale3D(FVector(S, S * 0.6f, S * 0.4f));
	}
}

void AExoExplosionEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Fireball: rapid expand then fade
	if (FireballMesh)
	{
		float ExpandT = FMath::Clamp(Age / (Lifetime * 0.3f), 0.f, 1.f);
		float MaxScale = ExpRadius / 800.f;
		float Scale = FMath::Lerp(MaxScale * 0.1f, MaxScale, FMath::Sqrt(ExpandT));
		// Shrink after peak
		float FadeT = FMath::Clamp((Age - Lifetime * 0.3f) / (Lifetime * 0.7f), 0.f, 1.f);
		Scale *= (1.f - FadeT * FadeT);
		FireballMesh->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));
	}

	// Shockwave ring: expand outward, flatten
	if (ShockwaveRing)
	{
		float RingT = FMath::Clamp(Age / (Lifetime * 0.5f), 0.f, 1.f);
		float RingScale = ExpRadius / 400.f * RingT;
		float RingAlpha = 1.f - RingT;
		ShockwaveRing->SetWorldScale3D(
			FVector(RingScale, RingScale, 0.005f * RingAlpha));
	}

	// Light: sharp falloff
	if (ExplosionLight)
	{
		float LightAlpha = 1.f - T;
		ExplosionLight->SetIntensity(BaseIntensity * LightAlpha * LightAlpha * LightAlpha);
	}

	// Debris: fly outward with gravity
	for (int32 i = 0; i < DebrisMeshes.Num(); i++)
	{
		if (!DebrisMeshes[i]) continue;
		FVector Pos = DebrisVelocities[i] * Age;
		Pos.Z -= 490.f * Age * Age; // Gravity
		DebrisMeshes[i]->SetRelativeLocation(Pos);

		// Tumble rotation
		FRotator Rot = FRotator(Age * 360.f * (i + 1), Age * 200.f * (i + 2), 0.f);
		DebrisMeshes[i]->SetRelativeRotation(Rot);

		// Shrink over time
		float S = FMath::Lerp(0.08f, 0.01f, T);
		DebrisMeshes[i]->SetWorldScale3D(FVector(S, S * 0.6f, S * 0.4f));
	}

	if (Age >= Lifetime)
	{
		Destroy();
	}
}
