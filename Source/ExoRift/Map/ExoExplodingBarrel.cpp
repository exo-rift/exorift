#include "Map/ExoExplodingBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoTracerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoExplodingBarrel::AExoExplodingBarrel()
{
	PrimaryActorTick.bCanEverTick = false;

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	RootComponent = BarrelMesh;
	BarrelMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	BarrelMesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 1.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFinder.Succeeded())
	{
		BarrelMesh->SetStaticMesh(CylFinder.Object);
	}

	// Warning glow
	UPointLightComponent* WarnLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("WarnLight"));
	WarnLight->SetupAttachment(BarrelMesh);
	WarnLight->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	WarnLight->SetIntensity(2000.f);
	WarnLight->SetAttenuationRadius(300.f);
	WarnLight->SetLightColor(FLinearColor(1.f, 0.3f, 0.05f));
	WarnLight->CastShadows = false;
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

	// Explosion VFX
	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	Destroy();
}
