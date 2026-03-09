#include "Weapons/ExoGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoTracerManager.h"
#include "Core/ExoAudioManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoGrenade::AExoGrenade()
{
	PrimaryActorTick.bCanEverTick = true;

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

	// Apply grenade body material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatF.Succeeded())
	{
		BodyMat = UMaterialInstanceDynamic::Create(MatF.Object, this);
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.08f, 0.08f, 0.08f));
		MeshComp->SetMaterial(0, BodyMat);
	}

	FuseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FuseLight"));
	FuseLight->SetupAttachment(CollisionComp);
	FuseLight->SetIntensity(2000.f);
	FuseLight->SetAttenuationRadius(300.f);
	FuseLight->SetLightColor(FLinearColor(1.f, 0.3f, 0.05f));
	FuseLight->CastShadows = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
		TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->ProjectileGravityScale = 1.f;
}

void AExoGrenade::BuildGrenadeVisuals()
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
		const FLinearColor& Color) -> UStaticMeshComponent*
	{
		if (!Mesh) return nullptr;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(CollisionComp);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->RegisterComponent();
		auto* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		C->SetMaterial(0, Mat);
		return C;
	};

	// Color by grenade type
	FLinearColor TypeColor;
	switch (GrenadeType)
	{
	case EGrenadeType::Frag:    TypeColor = FLinearColor(0.9f, 0.3f, 0.05f); break;
	case EGrenadeType::EMP:     TypeColor = FLinearColor(0.2f, 0.5f, 1.f);   break;
	case EGrenadeType::Smoke:   TypeColor = FLinearColor(0.7f, 0.7f, 0.7f);  break;
	default:                    TypeColor = FLinearColor(0.9f, 0.3f, 0.05f); break;
	}

	// Set body color based on type (dark tinted)
	if (BodyMat)
	{
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(TypeColor.R * 0.15f, TypeColor.G * 0.15f, TypeColor.B * 0.15f));
	}

	// Set fuse light to type color
	if (FuseLight) FuseLight->SetLightColor(TypeColor);

	// Equator ring (type-colored)
	MakePart(CylMesh, FVector(0.f, 0.f, 0.f), FVector(1.2f, 1.2f, 0.15f), TypeColor);

	// Top pin / fuse cap
	FLinearColor Metal(0.15f, 0.15f, 0.15f);
	MakePart(CylMesh, FVector(0.f, 0.f, 10.f), FVector(0.3f, 0.3f, 0.2f), Metal);

	// Pin lever (safety handle)
	MakePart(CubeMesh, FVector(4.f, 0.f, 12.f), FVector(0.6f, 0.15f, 0.08f), Metal);
}

void AExoGrenade::Ignite(FVector Direction)
{
	bIgnited = true;
	FuseElapsed = 0.f;
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * InitialSpeed;

	BuildGrenadeVisuals();

	GetWorldTimerManager().SetTimer(
		FuseTimerHandle, this, &AExoGrenade::Explode, FuseTime, false);
}

void AExoGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bIgnited) return;

	FuseElapsed += DeltaTime;
	float FuseFrac = FMath::Clamp(FuseElapsed / FuseTime, 0.f, 1.f);
	float Time = GetWorld()->GetTimeSeconds();

	// Fuse light pulses faster as detonation approaches
	float PulseRate = FMath::Lerp(2.f, 12.f, FuseFrac);
	float PulseStrength = 0.5f + 0.5f * FMath::Abs(FMath::Sin(Time * PulseRate));

	// Intensity ramps up toward detonation
	float BaseIntensity = FMath::Lerp(2000.f, 10000.f, FuseFrac);

	if (FuseLight)
	{
		FuseLight->SetIntensity(BaseIntensity * PulseStrength);
		FuseLight->SetAttenuationRadius(FMath::Lerp(300.f, 600.f, FuseFrac));
	}

	// Body emissive flash near detonation
	if (BodyMat && FuseFrac > 0.7f)
	{
		float EmissivePulse = (FuseFrac - 0.7f) / 0.3f;
		EmissivePulse *= FMath::Abs(FMath::Sin(Time * PulseRate));
		FLinearColor Emit(EmissivePulse * 2.f, EmissivePulse * 0.5f, 0.f);
		BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), Emit);
	}
}

void AExoGrenade::Explode()
{
	UE_LOG(LogExoRift, Log, TEXT("Grenade exploded at %s (Type=%d)"),
		*GetActorLocation().ToString(), static_cast<uint8>(GrenadeType));

	TArray<AActor*> IgnoreActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		ExplosionDamage,
		ExplosionDamage * 0.2f,
		GetActorLocation(),
		ExplosionRadius * 0.3f,
		ExplosionRadius,
		1.f,
		nullptr,
		IgnoreActors,
		this,
		GetInstigatorController()
	);

	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayExplosionSound(GetActorLocation());
	}

	Destroy();
}
