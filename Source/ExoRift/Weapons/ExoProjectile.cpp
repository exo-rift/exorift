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

	// Core energy sphere — stretched along travel direction
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetRelativeScale3D(FVector(0.35f, 0.2f, 0.2f));
	ProjectileMesh->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(SphereFinder.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		ProjectileMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		ProjectileMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(1.f, 0.4f, 0.1f));
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(6.f, 2.4f, 0.6f));
		ProjectileMesh->SetMaterial(0, ProjectileMat);
	}

	// Main glow light — bright and warm
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(CollisionSphere);
	GlowLight->SetIntensity(12000.f);
	GlowLight->SetAttenuationRadius(700.f);
	GlowLight->SetLightColor(FLinearColor(1.f, 0.4f, 0.1f));
	GlowLight->CastShadows = false;

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
			FLinearColor(Color.R * 6.f, Color.G * 6.f, Color.B * 6.f));
	}
	if (GlowLight)
	{
		GlowLight->SetLightColor(Color);
	}
}

void AExoProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Multi-frequency flicker for unstable energy warhead feel
	float Flicker = 1.f
		+ 0.12f * FMath::Sin(Time * 25.f)
		+ 0.08f * FMath::Sin(Time * 41.f)
		+ 0.05f * FMath::Sin(Time * 67.f);

	if (GlowLight)
	{
		GlowLight->SetIntensity(12000.f * Flicker);
		// Subtle color shift toward white at peak intensity
		float WhiteShift = FMath::Max(0.f, Flicker - 1.1f) * 3.f;
		GlowLight->SetLightColor(FLinearColor(
			GlowColor.R + WhiteShift,
			GlowColor.G + WhiteShift * 0.5f,
			GlowColor.B + WhiteShift * 0.3f));
	}

	// Pulsing emissive with rapid flicker
	if (ProjectileMat)
	{
		float EmPulse = 4.f + 2.f * FMath::Sin(Time * 18.f);
		float EmFlicker = 1.f + 0.3f * FMath::Sin(Time * 55.f);
		float Em = EmPulse * EmFlicker;
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * Em, GlowColor.G * Em, GlowColor.B * Em));
	}

	// Wobble the projectile scale slightly — unstable energy containment
	float Wobble = 1.f + 0.08f * FMath::Sin(Time * 30.f);
	float WobbleY = 1.f + 0.08f * FMath::Sin(Time * 30.f + 2.1f);
	if (ProjectileMesh)
	{
		ProjectileMesh->SetRelativeScale3D(FVector(
			0.35f * Wobble, 0.2f * WobbleY, 0.2f * Wobble));
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
