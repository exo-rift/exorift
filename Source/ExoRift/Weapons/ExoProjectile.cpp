// ExoProjectile.cpp — Unstable energy warhead with glowing trail and halo
#include "Weapons/ExoProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoTracerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

AExoProjectile::AExoProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 5.f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionSphere;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	// Core energy sphere — stretched along travel direction
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetRelativeScale3D(FVector(0.45f, 0.25f, 0.25f));
	ProjectileMesh->CastShadow = false;
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereFinder.Succeeded()) ProjectileMesh->SetStaticMesh(SphereFinder.Object);

	// Outer glow — large soft halo sphere
	OuterGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterGlow"));
	OuterGlow->SetupAttachment(CollisionSphere);
	OuterGlow->SetRelativeScale3D(FVector(0.8f, 0.5f, 0.5f));
	OuterGlow->CastShadow = false;
	OuterGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OuterGlow->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) OuterGlow->SetStaticMesh(SphereFinder.Object);

	// Trail wake — cylinder stretching behind the projectile
	TrailWake = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrailWake"));
	TrailWake->SetupAttachment(CollisionSphere);
	TrailWake->SetRelativeLocation(FVector(-200.f, 0.f, 0.f));
	TrailWake->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	TrailWake->SetRelativeScale3D(FVector(0.08f, 0.08f, 4.f));
	TrailWake->CastShadow = false;
	TrailWake->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrailWake->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) TrailWake->SetStaticMesh(CylFinder.Object);

	// Dynamic materials
	if (MatFinder.Succeeded())
	{
		ProjectileMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		ProjectileMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(1.f, 0.4f, 0.1f));
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(12.f, 5.f, 1.2f));
		ProjectileMesh->SetMaterial(0, ProjectileMat);

		OuterGlowMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		OuterGlowMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.6f, 0.25f, 0.06f));
		OuterGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(3.f, 1.2f, 0.3f));
		OuterGlow->SetMaterial(0, OuterGlowMat);

		TrailMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		TrailMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.4f, 0.15f, 0.04f));
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(2.f, 0.8f, 0.2f));
		TrailWake->SetMaterial(0, TrailMat);
	}

	// Main glow light — bright and dramatic
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(CollisionSphere);
	GlowLight->SetIntensity(40000.f);
	GlowLight->SetAttenuationRadius(1200.f);
	GlowLight->SetLightColor(FLinearColor(1.f, 0.4f, 0.1f));
	GlowLight->CastShadows = false;

	// Trail light — follows behind
	TrailLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TrailLight"));
	TrailLight->SetupAttachment(CollisionSphere);
	TrailLight->SetRelativeLocation(FVector(-250.f, 0.f, 0.f));
	TrailLight->SetIntensity(15000.f);
	TrailLight->SetAttenuationRadius(600.f);
	TrailLight->SetLightColor(FLinearColor(1.f, 0.3f, 0.05f));
	TrailLight->CastShadows = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
		TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.5f;

	CollisionSphere->OnComponentHit.AddDynamic(this, &AExoProjectile::OnHit);
}

void AExoProjectile::SetProjectileColor(const FLinearColor& Color)
{
	GlowColor = Color;
	if (ProjectileMat)
	{
		ProjectileMat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 12.f, Color.G * 12.f, Color.B * 12.f));
	}
	if (OuterGlowMat)
	{
		OuterGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
	}
	if (TrailMat)
	{
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 2.f, Color.G * 2.f, Color.B * 2.f));
	}
	if (GlowLight) GlowLight->SetLightColor(Color);
	if (TrailLight) TrailLight->SetLightColor(Color);
}

void AExoProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float Time = GetWorld()->GetTimeSeconds();

	// Multi-frequency flicker — unstable energy containment
	float F1 = FMath::Sin(Time * 25.f) * 0.15f;
	float F2 = FMath::Sin(Time * 41.f) * 0.10f;
	float F3 = FMath::Sin(Time * 67.f) * 0.06f;
	float F4 = FMath::Sin(Time * 113.f) * 0.04f;
	float Flicker = 1.f + F1 + F2 + F3 + F4;

	if (GlowLight)
	{
		GlowLight->SetIntensity(40000.f * Flicker);
		float WhiteShift = FMath::Max(0.f, Flicker - 1.15f) * 4.f;
		GlowLight->SetLightColor(FLinearColor(
			GlowColor.R + WhiteShift,
			GlowColor.G + WhiteShift * 0.5f,
			GlowColor.B + WhiteShift * 0.3f));
	}
	if (TrailLight)
	{
		TrailLight->SetIntensity(15000.f * (0.7f + 0.3f * Flicker));
	}

	// Pulsing emissive with high-frequency shimmer
	if (ProjectileMat)
	{
		float Em = (8.f + 4.f * FMath::Sin(Time * 18.f))
			* (1.f + 0.3f * FMath::Sin(Time * 55.f));
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * Em, GlowColor.G * Em, GlowColor.B * Em));
	}

	// Outer glow breathes
	if (OuterGlowMat)
	{
		float OG = 2.f + 1.f * FMath::Sin(Time * 12.f);
		OuterGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * OG, GlowColor.G * OG, GlowColor.B * OG));
	}

	// Trail emissive fades toward tail
	if (TrailMat)
	{
		float TE = 1.5f + 0.5f * FMath::Sin(Time * 20.f);
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * TE, GlowColor.G * TE, GlowColor.B * TE));
	}

	// Wobble — unstable warhead
	float Wx = 1.f + 0.10f * FMath::Sin(Time * 30.f);
	float Wy = 1.f + 0.10f * FMath::Sin(Time * 30.f + 2.1f);
	float Wz = 1.f + 0.08f * FMath::Sin(Time * 35.f + 4.2f);
	if (ProjectileMesh)
		ProjectileMesh->SetRelativeScale3D(FVector(0.45f * Wx, 0.25f * Wy, 0.25f * Wz));
	if (OuterGlow)
	{
		float GP = 1.f + 0.12f * FMath::Sin(Time * 15.f);
		OuterGlow->SetRelativeScale3D(FVector(0.8f * GP, 0.5f * GP, 0.5f * GP));
	}

	// Trail stretches with speed variation
	if (TrailWake)
	{
		float TrailStretch = 4.f + 1.f * Flicker;
		TrailWake->SetRelativeScale3D(FVector(0.06f * Flicker, 0.06f * Flicker, TrailStretch));
	}
}

void AExoProjectile::InitProjectile(const FVector& LaunchVelocity, float InDamage,
	AController* InInstigator)
{
	ProjectileDamage = InDamage;
	InstigatorController = InInstigator;
	ProjectileMovement->Velocity = LaunchVelocity;
}

void AExoProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

void AExoProjectile::Explode()
{
	TArray<AActor*> IgnoredActors;
	if (GetInstigator()) IgnoredActors.Add(GetInstigator());

	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		ProjectileDamage,
		ProjectileDamage * MinDamageFalloff,
		GetActorLocation(),
		ExplosionRadius * 0.3f,
		ExplosionRadius,
		1.f,
		nullptr,
		IgnoredActors,
		this,
		InstigatorController
	);

	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	UE_LOG(LogExoRift, Verbose, TEXT("Projectile exploded at %s"),
		*GetActorLocation().ToString());
	Destroy();
}
