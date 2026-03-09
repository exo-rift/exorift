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
	PrimaryActorTick.bCanEverTick = true;

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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(MatFinder.Object, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.04f, 0.02f));
		BarrelMesh->SetMaterial(0, Mat);
	}

	WarnLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("WarnLight"));
	WarnLight->SetupAttachment(BarrelMesh);
	WarnLight->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	WarnLight->SetIntensity(3000.f);
	WarnLight->SetAttenuationRadius(400.f);
	WarnLight->SetLightColor(FLinearColor(1.f, 0.3f, 0.05f));
	WarnLight->CastShadows = false;
}

void AExoExplodingBarrel::BeginPlay()
{
	Super::BeginPlay();
	BuildBarrelVisuals();
}

void AExoExplodingBarrel::BuildBarrelVisuals()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

	UStaticMesh* CubeMesh = CubeF.Succeeded() ? CubeF.Object : nullptr;
	UStaticMesh* CylMesh = CylF.Succeeded() ? CylF.Object : nullptr;
	UMaterialInterface* BaseMat = MatF.Succeeded() ? MatF.Object : nullptr;

	if (!BaseMat) return;

	auto MakePart = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator)
		-> UStaticMeshComponent*
	{
		if (!Mesh) return nullptr;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = true;
		C->RegisterComponent();
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		C->SetMaterial(0, Mat);
		return C;
	};

	FLinearColor Yellow(0.9f, 0.7f, 0.05f);
	FLinearColor DarkMetal(0.08f, 0.08f, 0.08f);
	FLinearColor Orange(0.9f, 0.3f, 0.05f);

	// Top cap ring
	MakePart(CylMesh, FVector(0.f, 0.f, 78.f), FVector(1.05f, 1.05f, 0.04f), DarkMetal);
	// Bottom cap ring
	MakePart(CylMesh, FVector(0.f, 0.f, -78.f), FVector(1.05f, 1.05f, 0.04f), DarkMetal);

	// Hazard stripes (two opposing yellow bands around mid-section)
	auto* Stripe1 = MakePart(CubeMesh, FVector(60.f, 0.f, 20.f),
		FVector(0.02f, 0.8f, 0.08f), Yellow);
	auto* Stripe2 = MakePart(CubeMesh, FVector(-60.f, 0.f, 20.f),
		FVector(0.02f, 0.8f, 0.08f), Yellow);
	MakePart(CubeMesh, FVector(0.f, 60.f, 20.f),
		FVector(0.8f, 0.02f, 0.08f), Yellow);
	MakePart(CubeMesh, FVector(0.f, -60.f, 20.f),
		FVector(0.8f, 0.02f, 0.08f), Yellow);

	// Second hazard ring lower
	MakePart(CubeMesh, FVector(60.f, 0.f, -20.f),
		FVector(0.02f, 0.8f, 0.08f), Yellow);
	MakePart(CubeMesh, FVector(-60.f, 0.f, -20.f),
		FVector(0.02f, 0.8f, 0.08f), Yellow);

	// Warning symbol: triangle on front face (3 bars approximation)
	MakePart(CubeMesh, FVector(62.f, 0.f, 50.f),
		FVector(0.01f, 0.12f, 0.02f), Orange);
	MakePart(CubeMesh, FVector(62.f, 0.f, 42.f),
		FVector(0.01f, 0.18f, 0.02f), Orange);
	MakePart(CubeMesh, FVector(62.f, 0.f, 34.f),
		FVector(0.01f, 0.24f, 0.02f), Orange);
	// Exclamation dot
	MakePart(CubeMesh, FVector(62.f, 0.f, 55.f),
		FVector(0.01f, 0.03f, 0.03f), Orange);

	// Valve handle on top
	MakePart(CylMesh, FVector(0.f, 0.f, 82.f), FVector(0.15f, 0.15f, 0.03f), DarkMetal);
	MakePart(CubeMesh, FVector(0.f, 0.f, 86.f), FVector(0.25f, 0.03f, 0.02f), DarkMetal);

	// Store stripe material for damage flash
	if (Stripe1) HazardStripeMat = Cast<UMaterialInstanceDynamic>(Stripe1->GetMaterial(0));
}

void AExoExplodingBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bHasExploded) return;

	float Time = GetWorld()->GetTimeSeconds();

	// Pulsing warning light
	if (WarnLight)
	{
		float Pulse = 0.6f + 0.4f * FMath::Abs(FMath::Sin(Time * 1.5f));
		WarnLight->SetIntensity(3000.f * Pulse);
	}

	// Flash hazard stripes brighter when damaged
	if (HazardStripeMat && Health < 50.f)
	{
		float DmgFrac = 1.f - (Health / 50.f);
		float Flash = 1.f + DmgFrac * 2.f * FMath::Abs(FMath::Sin(Time * 6.f));
		HazardStripeMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.9f * Flash, 0.7f * Flash, 0.05f));
	}
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

	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayExplosionSound(GetActorLocation());
	}

	Destroy();
}
