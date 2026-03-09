#include "Weapons/ExoGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Visual/ExoTracerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoGrenade::AExoGrenade()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(8.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetRelativeScale3D(FVector(0.15f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
	}

	// Pulsing fuse light — warns players it's live
	FuseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FuseLight"));
	FuseLight->SetupAttachment(CollisionComp);
	FuseLight->SetIntensity(5000.f);
	FuseLight->SetAttenuationRadius(400.f);
	FuseLight->SetLightColor(FLinearColor(1.f, 0.3f, 0.05f));
	FuseLight->CastShadows = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->ProjectileGravityScale = 1.f;
}

void AExoGrenade::Ignite(FVector Direction)
{
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * InitialSpeed;

	GetWorldTimerManager().SetTimer(
		FuseTimerHandle, this, &AExoGrenade::Explode, FuseTime, false);
}

void AExoGrenade::Explode()
{
	UE_LOG(LogExoRift, Log, TEXT("Grenade exploded at %s (Type=%d)"),
		*GetActorLocation().ToString(), static_cast<uint8>(GrenadeType));

	// Radial damage with falloff
	TArray<AActor*> IgnoreActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		ExplosionDamage,
		ExplosionDamage * 0.2f,  // MinDamage at edge
		GetActorLocation(),
		ExplosionRadius * 0.3f,  // Inner radius (full damage)
		ExplosionRadius,         // Outer radius
		1.f,                     // Falloff exponent
		nullptr,                 // DamageTypeClass
		IgnoreActors,
		this,                    // DamageCauser
		GetInstigatorController()
	);

	// Explosion VFX
	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	Destroy();
}
