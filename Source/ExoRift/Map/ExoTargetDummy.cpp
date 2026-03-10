// ExoTargetDummy.cpp — Destructible target for warmup / firing range
#include "Map/ExoTargetDummy.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoTargetDummy::AExoTargetDummy()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	UStaticMesh* Sphere = SphereFinder.Succeeded() ? SphereFinder.Object : nullptr;
	UStaticMesh* Cyl = CylFinder.Succeeded() ? CylFinder.Object : nullptr;

	// Base platform
	BasePlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	if (Cyl) BasePlateMesh->SetStaticMesh(Cyl);
	BasePlateMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.02f));
	BasePlateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BasePlateMesh->CastShadow = false;
	RootComponent = BasePlateMesh;

	// Torso — tall rectangular block
	TorsoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Torso"));
	if (Cube) TorsoMesh->SetStaticMesh(Cube);
	TorsoMesh->SetupAttachment(BasePlateMesh);
	TorsoMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	TorsoMesh->SetRelativeScale3D(FVector(0.3f, 0.15f, 0.55f));
	TorsoMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TorsoMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TorsoMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	TorsoMesh->CastShadow = false;

	// Head — sphere on top
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	if (Sphere) HeadMesh->SetStaticMesh(Sphere);
	HeadMesh->SetupAttachment(TorsoMesh);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 65.f));
	HeadMesh->SetRelativeScale3D(FVector(0.6f, 0.9f, 0.45f));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeadMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HeadMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	HeadMesh->CastShadow = false;

	// Glow light
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	GlowLight->SetupAttachment(TorsoMesh);
	GlowLight->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	GlowLight->SetIntensity(3000.f);
	GlowLight->SetAttenuationRadius(350.f);
	GlowLight->CastShadows = false;

	AccentColor = FLinearColor(1.f, 0.3f, 0.1f); // Default orange-red
}

void AExoTargetDummy::InitDummy(const FLinearColor& Color, float InHealth)
{
	AccentColor = Color;
	MaxHealth = InHealth;
	CurrentHealth = InHealth;
	BuildVisuals();
}

void AExoTargetDummy::BuildVisuals()
{
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	// Dark body with colored emissive edges
	TorsoMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	FLinearColor Em(AccentColor.R * 2.f, AccentColor.G * 2.f, AccentColor.B * 2.f);
	TorsoMat->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
	TorsoMesh->SetMaterial(0, TorsoMat);

	HeadMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	FLinearColor HeadEm(AccentColor.R * 3.f, AccentColor.G * 3.f, AccentColor.B * 3.f);
	HeadMat->SetVectorParameterValue(TEXT("EmissiveColor"), HeadEm);
	HeadMesh->SetMaterial(0, HeadMat);

	// Base plate
	UMaterialInstanceDynamic* PlateMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	FLinearColor PlateEm(AccentColor.R * 0.5f, AccentColor.G * 0.5f, AccentColor.B * 0.5f);
	PlateMat->SetVectorParameterValue(TEXT("EmissiveColor"), PlateEm);
	BasePlateMesh->SetMaterial(0, PlateMat);

	GlowLight->SetLightColor(AccentColor);
}

float AExoTargetDummy::TakeDamage(float Damage, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!bAlive) return 0.f;

	float Applied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= Applied;
	HitFlashTimer = 0.15f;

	// Bright flash on hit
	if (TorsoMat)
	{
		FLinearColor Flash(AccentColor.R * 15.f, AccentColor.G * 15.f, AccentColor.B * 15.f);
		TorsoMat->SetVectorParameterValue(TEXT("EmissiveColor"), Flash);
	}

	if (CurrentHealth <= 0.f)
	{
		Shatter();
	}

	return Applied;
}

void AExoTargetDummy::Shatter()
{
	bAlive = false;
	RespawnTimer = 3.f;

	// Hide the dummy
	TorsoMesh->SetVisibility(false);
	HeadMesh->SetVisibility(false);
	GlowLight->SetIntensity(0.f);
}

void AExoTargetDummy::Respawn()
{
	bAlive = true;
	CurrentHealth = MaxHealth;

	TorsoMesh->SetVisibility(true);
	HeadMesh->SetVisibility(true);
	GlowLight->SetIntensity(3000.f);

	if (TorsoMat)
	{
		FLinearColor Em(AccentColor.R * 2.f, AccentColor.G * 2.f, AccentColor.B * 2.f);
		TorsoMat->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
	}
}

void AExoTargetDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bAlive)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f)
		{
			Respawn();
		}
		return;
	}

	// Hit flash recovery
	if (HitFlashTimer > 0.f)
	{
		HitFlashTimer -= DeltaTime;
		if (HitFlashTimer <= 0.f && TorsoMat)
		{
			FLinearColor Em(AccentColor.R * 2.f, AccentColor.G * 2.f, AccentColor.B * 2.f);
			TorsoMat->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		}
	}

	// Subtle idle glow pulse
	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = 0.8f + 0.2f * FMath::Sin(Time * 2.5f);
	GlowLight->SetIntensity(3000.f * Pulse);

	// Health-based color shift toward red as damaged
	float HealthPct = CurrentHealth / MaxHealth;
	if (HealthPct < 0.5f && TorsoMat)
	{
		float RedShift = 1.f - (HealthPct * 2.f);
		FLinearColor DmgColor = FMath::Lerp(AccentColor,
			FLinearColor(1.f, 0.1f, 0.05f), RedShift);
		FLinearColor Em(DmgColor.R * 2.f, DmgColor.G * 2.f, DmgColor.B * 2.f);
		TorsoMat->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
	}
}
