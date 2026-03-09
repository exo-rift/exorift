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

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetRelativeScale3D(FVector(0.3f, 0.15f, 0.15f));
	ProjectileMesh->CastShadow = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(SphereFinder.Object);
	}

	// Emissive projectile material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		ProjectileMat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		ProjectileMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(1.f, 0.4f, 0.1f));
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(3.f, 1.2f, 0.3f));
		ProjectileMesh->SetMaterial(0, ProjectileMat);
	}

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(CollisionSphere);
	GlowLight->SetIntensity(8000.f);
	GlowLight->SetAttenuationRadius(500.f);
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
			FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
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

	// Flickering glow — energy weapon feel
	float Flicker = 1.f + 0.1f * FMath::Sin(Time * 30.f) + 0.08f * FMath::Sin(Time * 47.f);
	if (GlowLight)
	{
		GlowLight->SetIntensity(8000.f * Flicker);
	}

	// Pulsing emissive
	if (ProjectileMat)
	{
		float EmissivePulse = 2.5f + 0.5f * FMath::Sin(Time * 20.f);
		ProjectileMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(GlowColor.R * EmissivePulse, GlowColor.G * EmissivePulse,
				GlowColor.B * EmissivePulse));
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
