// ExoProjectile.cpp — Unstable energy warhead with glowing trail and halo
#include "Weapons/ExoProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
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
	// Core energy sphere — stretched along travel direction, larger for visibility
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetRelativeScale3D(FVector(2.5f, 1.4f, 1.4f));
	ProjectileMesh->CastShadow = false;
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereFinder.Succeeded()) ProjectileMesh->SetStaticMesh(SphereFinder.Object);

	// Outer glow — large soft halo sphere
	OuterGlow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterGlow"));
	OuterGlow->SetupAttachment(CollisionSphere);
	OuterGlow->SetRelativeScale3D(FVector(5.f, 3.2f, 3.2f));
	OuterGlow->CastShadow = false;
	OuterGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OuterGlow->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) OuterGlow->SetStaticMesh(SphereFinder.Object);

	// Trail wake — cylinder stretching behind the projectile
	TrailWake = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrailWake"));
	TrailWake->SetupAttachment(CollisionSphere);
	TrailWake->SetRelativeLocation(FVector(-600.f, 0.f, 0.f));
	TrailWake->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	TrailWake->SetRelativeScale3D(FVector(0.45f, 0.45f, 12.f));
	TrailWake->CastShadow = false;
	TrailWake->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrailWake->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) TrailWake->SetStaticMesh(CylFinder.Object);

	// Dynamic materials — extreme emissive for massive bloom and visibility
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (EmissiveMat)
	{
		ProjectileMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!ProjectileMat) { return; }
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(270.f, 112.f, 27.f));
		ProjectileMesh->SetMaterial(0, ProjectileMat);

		OuterGlowMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!OuterGlowMat) { return; }
		OuterGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(80.f, 32.f, 9.f));
		OuterGlow->SetMaterial(0, OuterGlowMat);

		TrailMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!TrailMat) { return; }
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(56.f, 22.f, 5.5f));
		TrailWake->SetMaterial(0, TrailMat);
	}

	// Main glow light — intense, far-reaching for dramatic environment lighting
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(CollisionSphere);
	GlowLight->SetIntensity(1150000.f);
	GlowLight->SetAttenuationRadius(8400.f);
	GlowLight->SetLightColor(FLinearColor(2.3f, 0.92f, 0.23f));
	GlowLight->CastShadows = false;

	// Trail light — follows behind
	TrailLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TrailLight"));
	TrailLight->SetupAttachment(CollisionSphere);
	TrailLight->SetRelativeLocation(FVector(-700.f, 0.f, 0.f));
	TrailLight->SetIntensity(460000.f);
	TrailLight->SetAttenuationRadius(5600.f);
	TrailLight->SetLightColor(FLinearColor(2.3f, 0.69f, 0.115f));
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
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 270.f, Color.G * 270.f, Color.B * 270.f));
	}
	if (OuterGlowMat)
	{
		OuterGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 80.f, Color.G * 80.f, Color.B * 80.f));
	}
	if (TrailMat)
	{
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 56.f, Color.G * 56.f, Color.B * 56.f));
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
		GlowLight->SetIntensity(460000.f * Flicker);
		float WhiteShift = FMath::Max(0.f, Flicker - 1.15f) * 8.f;
		GlowLight->SetLightColor(FLinearColor(
			GlowColor.R + WhiteShift,
			GlowColor.G + WhiteShift * 0.5f,
			GlowColor.B + WhiteShift * 0.3f));
	}
	if (TrailLight)
	{
		TrailLight->SetIntensity(184000.f * (0.7f + 0.3f * Flicker));
	}

	// Pulsing emissive with high-frequency shimmer
	if (ProjectileMat)
	{
		float Em = (92.f + 46.f * FMath::Sin(Time * 18.f))
			* (1.f + 0.3f * FMath::Sin(Time * 55.f));
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * Em, GlowColor.G * Em, GlowColor.B * Em));
	}

	// Outer glow breathes
	if (OuterGlowMat)
	{
		float OG = 27.f + 14.f * FMath::Sin(Time * 12.f);
		OuterGlowMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * OG, GlowColor.G * OG, GlowColor.B * OG));
	}

	// Trail emissive fades toward tail
	if (TrailMat)
	{
		float TE = 18.f + 9.f * FMath::Sin(Time * 20.f);
		TrailMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * TE, GlowColor.G * TE, GlowColor.B * TE));
	}

	// Wobble — unstable warhead, more dramatic deformation
	float Wx = 1.f + 0.15f * FMath::Sin(Time * 30.f);
	float Wy = 1.f + 0.15f * FMath::Sin(Time * 30.f + 2.1f);
	float Wz = 1.f + 0.12f * FMath::Sin(Time * 35.f + 4.2f);
	if (ProjectileMesh)
		ProjectileMesh->SetRelativeScale3D(FVector(2.5f * Wx, 1.4f * Wy, 1.4f * Wz));
	if (OuterGlow)
	{
		float GP = 1.f + 0.22f * FMath::Sin(Time * 15.f);
		OuterGlow->SetRelativeScale3D(FVector(5.f * GP, 3.2f * GP, 3.2f * GP));
	}

	// Trail stretches with speed variation
	if (TrailWake)
	{
		float TrailStretch = 12.f + 4.f * Flicker;
		TrailWake->SetRelativeScale3D(FVector(0.4f * Flicker, 0.4f * Flicker, TrailStretch));
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
