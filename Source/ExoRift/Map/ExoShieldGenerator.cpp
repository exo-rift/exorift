// ExoShieldGenerator.cpp — Destructible energy dome at strategic positions
#include "Map/ExoShieldGenerator.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoTracerManager.h"
#include "Visual/ExoScreenShake.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoShieldGenerator::AExoShieldGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFind(
		TEXT("/Engine/BasicShapes/Sphere"));

	// Base platform
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	RootComponent = BaseMesh;
	if (CylFind.Succeeded()) BaseMesh->SetStaticMesh(CylFind.Object);
	BaseMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.2f));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BaseMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Central pylon
	PylonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PylonMesh"));
	PylonMesh->SetupAttachment(BaseMesh);
	if (CylFind.Succeeded()) PylonMesh->SetStaticMesh(CylFind.Object);
	PylonMesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	PylonMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 2.f));
	PylonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PylonMesh->CastShadow = false;

	// Shield dome (sphere scaled to radius)
	ShieldDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldDome"));
	ShieldDome->SetupAttachment(BaseMesh);
	if (SphFind.Succeeded()) ShieldDome->SetStaticMesh(SphFind.Object);
	ShieldDome->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ShieldDome->SetRelativeScale3D(FVector(ShieldRadius / 50.f));
	ShieldDome->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ShieldDome->SetCollisionResponseToAllChannels(ECR_Block);
	ShieldDome->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ShieldDome->CastShadow = false;

	// Core light
	CoreLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoreLight"));
	CoreLight->SetupAttachment(BaseMesh);
	CoreLight->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	CoreLight->SetIntensity(8000.f);
	CoreLight->SetAttenuationRadius(ShieldRadius * 1.5f);
	CoreLight->CastShadows = false;
}

void AExoShieldGenerator::BeginPlay()
{
	Super::BeginPlay();

	// Apply materials
	if (BaseMesh->GetStaticMesh())
	{
		UMaterialInstanceDynamic* BM = UMaterialInstanceDynamic::Create(
			BaseMesh->GetMaterial(0), this);
		BM->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.06f, 0.065f, 0.08f));
		BaseMesh->SetMaterial(0, BM);
	}
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	if (PylonMesh->GetStaticMesh())
	{
		UMaterialInstanceDynamic* PM = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		PM->SetVectorParameterValue(TEXT("EmissiveColor"),
			ShieldColor * 3.f);
		PylonMesh->SetMaterial(0, PM);
	}
	if (ShieldDome->GetStaticMesh())
	{
		DomeMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		DomeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			ShieldColor * 2.f);
		ShieldDome->SetMaterial(0, DomeMat);
	}
	CoreLight->SetLightColor(ShieldColor);
}

void AExoShieldGenerator::InitGenerator(float InShieldRadius, const FLinearColor& Color)
{
	ShieldRadius = InShieldRadius;
	ShieldColor = Color;
	ShieldDome->SetRelativeScale3D(FVector(ShieldRadius / 50.f));
	CoreLight->SetAttenuationRadius(ShieldRadius * 1.5f);
	CoreLight->SetLightColor(Color);
}

void AExoShieldGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bShieldActive) return;

	float Time = GetWorld()->GetTimeSeconds();

	// Dome emissive pulse — faster as health depletes
	float HealthPct = ShieldHealth / MaxShieldHealth;
	float PulseRate = FMath::Lerp(4.f, 12.f, 1.f - HealthPct);
	float Pulse = 0.6f + 0.4f * FMath::Sin(Time * PulseRate);

	if (DomeMat)
	{
		DomeMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			ShieldColor * 2.f * Pulse * HealthPct);
	}
	CoreLight->SetIntensity(8000.f * Pulse * HealthPct);
}

float AExoShieldGenerator::TakeDamage(float Damage, const FDamageEvent& Event,
	AController* EventInstigator, AActor* Causer)
{
	if (!bShieldActive) return 0.f;

	float Actual = Super::TakeDamage(Damage, Event, EventInstigator, Causer);
	ShieldHealth -= Damage;

	if (ShieldHealth <= 0.f)
	{
		DestroyShield();
	}
	return Actual;
}

void AExoShieldGenerator::DestroyShield()
{
	bShieldActive = false;
	ShieldDome->SetVisibility(false);
	ShieldDome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreLight->SetIntensity(0.f);

	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ShieldRadius * 0.5f);
	FExoScreenShake::AddExplosionShake(GetActorLocation(), GetActorLocation(), ShieldRadius, 0.3f);

	// Self-destruct after a delay
	SetLifeSpan(5.f);
}
