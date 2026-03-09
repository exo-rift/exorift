#include "Map/ExoExplodingBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoExplodingBarrel::AExoExplodingBarrel()
{
	PrimaryActorTick.bCanEverTick = false;

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	RootComponent = BarrelMesh;
	BarrelMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

float AExoExplodingBarrel::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bHasExploded) return 0.f;

	float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= Actual;

	UE_LOG(LogExoRift, Verbose, TEXT("Barrel hit for %.0f, HP=%.0f"), Actual, Health);

	if (Health <= 0.f)
	{
		Explode(EventInstigator);
	}

	return Actual;
}

void AExoExplodingBarrel::Explode(AController* InstigatorController)
{
	if (bHasExploded) return;
	bHasExploded = true;

	UE_LOG(LogExoRift, Log, TEXT("Barrel exploding at %s"), *GetActorLocation().ToString());

	// Radial damage with linear falloff
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		ExplosionDamage,             // BaseDamage
		ExplosionDamage * 0.2f,      // MinimumDamage (20% at edge)
		GetActorLocation(),          // Origin
		ExplosionRadius * 0.3f,      // DamageInnerRadius (full damage)
		ExplosionRadius,             // DamageOuterRadius
		1.f,                         // DamageFalloff exponent
		nullptr,                     // DamageTypeClass
		TArray<AActor*>{this},       // IgnoreActors (don't damage self)
		this,                        // DamageCauser
		InstigatorController         // InstigatedBy
	);

	Destroy();
}
