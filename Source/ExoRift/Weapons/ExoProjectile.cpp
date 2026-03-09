#include "Weapons/ExoProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

AExoProjectile::AExoProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 5.f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionSphere;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(CollisionSphere);
	ProjectileMesh->SetRelativeScale3D(FVector(0.2f));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.5f;

	CollisionSphere->OnComponentHit.AddDynamic(this, &AExoProjectile::OnHit);
}

void AExoProjectile::InitProjectile(const FVector& LaunchVelocity, float InDamage, AController* InInstigator)
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
		ExplosionRadius * 0.3f, // Inner radius (full damage)
		ExplosionRadius,         // Outer radius
		1.f,                     // Falloff exponent
		nullptr,                 // Damage type
		IgnoredActors,
		this,                    // Damage causer
		InstigatorController
	);

	UE_LOG(LogExoRift, Verbose, TEXT("Projectile exploded at %s"), *GetActorLocation().ToString());
	Destroy();
}
