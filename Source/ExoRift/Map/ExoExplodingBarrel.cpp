#include "Map/ExoExplodingBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoTracerManager.h"
#include "Core/ExoAudioManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
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

	// Apply red-tinted barrel material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.04f, 0.02f));
		BarrelMesh->SetMaterial(0, Mat);
	}

	// Warning light
	UPointLightComponent* WarnLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("WarnLight"));
	WarnLight->SetupAttachment(BarrelMesh);
	WarnLight->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	WarnLight->SetIntensity(3000.f);
	WarnLight->SetAttenuationRadius(400.f);
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
		ExplosionDamage,
		ExplosionDamage * 0.2f,
		GetActorLocation(),
		ExplosionRadius * 0.3f,
		ExplosionRadius,
		1.f,
		nullptr,
		TArray<AActor*>{this},
		this,
		InstigatorController
	);

	// VFX
	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	// Audio
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayExplosionSound(GetActorLocation());
	}

	Destroy();
}
