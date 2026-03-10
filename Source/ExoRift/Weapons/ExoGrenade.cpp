#include "Weapons/ExoGrenade.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoTracerManager.h"
#include "Visual/ExoScreenShake.h"
#include "Visual/ExoEMPEffect.h"
#include "Core/ExoAudioManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
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

	// Apply grenade body material — lit surface with emissive for fuse glow
	UMaterialInterface* LitEmissive = FExoMaterialFactory::GetLitEmissive();
	if (LitEmissive)
	{
		BodyMat = UMaterialInstanceDynamic::Create(LitEmissive, this);
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
	UStaticMesh* CubeMesh = CubeF.Succeeded() ? CubeF.Object : nullptr;
	UStaticMesh* CylMesh = CylF.Succeeded() ? CylF.Object : nullptr;

	UMaterialInterface* LitMatG = FExoMaterialFactory::GetLitEmissive();
	UMaterialInterface* EmissiveOpaque = FExoMaterialFactory::GetEmissiveOpaque();

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
		float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Lum > 0.15f && EmissiveOpaque)
		{
			auto* Mat = UMaterialInstanceDynamic::Create(EmissiveOpaque, this);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
			C->SetMaterial(0, Mat);
		}
		else if (LitMatG)
		{
			auto* Mat = UMaterialInstanceDynamic::Create(LitMatG, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.85f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.28f);
			C->SetMaterial(0, Mat);
		}
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

	switch (GrenadeType)
	{
	case EGrenadeType::Frag:  ExplodeFrag();  break;
	case EGrenadeType::EMP:   ExplodeEMP();   break;
	case EGrenadeType::Smoke: ExplodeSmoke(); break;
	default: ExplodeFrag(); break;
	}

	Destroy();
}

void AExoGrenade::ExplodeFrag()
{
	TArray<AActor*> IgnoreActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(), ExplosionDamage, ExplosionDamage * 0.2f,
		GetActorLocation(), ExplosionRadius * 0.3f, ExplosionRadius,
		1.f, nullptr, IgnoreActors, this, GetInstigatorController());

	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		Audio->PlayExplosionSound(GetActorLocation());
}

void AExoGrenade::ExplodeEMP()
{
	FVector Origin = GetActorLocation();

	// Find all characters in radius
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Shape = FCollisionShape::MakeSphere(ExplosionRadius);
	GetWorld()->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity,
		ECC_Pawn, Shape);

	for (const FOverlapResult& O : Overlaps)
	{
		AExoCharacter* Char = Cast<AExoCharacter>(O.GetActor());
		if (!Char) continue;

		// Drain shields completely
		UExoShieldComponent* Shield = Char->GetShieldComponent();
		if (Shield) Shield->AbsorbDamage(999.f);

		// Slow movement for 4 seconds
		UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
		if (CMC)
		{
			float OrigSpeed = CMC->MaxWalkSpeed;
			CMC->MaxWalkSpeed *= 0.4f;
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle,
				[CMC, OrigSpeed]() { if (CMC) CMC->MaxWalkSpeed = OrigSpeed; },
				4.f, false);
		}

		// Small EMP damage
		FDamageEvent DmgEvent;
		Char->TakeDamage(15.f, DmgEvent, GetInstigatorController(), this);
	}

	FExoScreenShake::AddShake(0.4f, 0.3f);

	// Spawn EMP pulse VFX
	FActorSpawnParameters EMPParams;
	AExoEMPEffect* EMPFx = GetWorld()->SpawnActor<AExoEMPEffect>(
		AExoEMPEffect::StaticClass(), Origin, FRotator::ZeroRotator, EMPParams);
	if (EMPFx) EMPFx->InitPulse(ExplosionRadius);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		Audio->PlayExplosionSound(Origin);
}

void AExoGrenade::ExplodeSmoke()
{
	// Spawn a large smoke cloud using multiple mesh components on a temporary actor
	FVector Origin = GetActorLocation();

	// Spawn a smoke cloud actor
	FActorSpawnParameters Params;
	AActor* Cloud = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), Origin,
		FRotator::ZeroRotator, Params);
	if (!Cloud) return;
	Cloud->SetLifeSpan(8.f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (!SphereFinder.Succeeded()) return;

	UMaterialInterface* SmokeMat = FExoMaterialFactory::GetEmissiveOpaque();

	// Create cluster of smoke spheres
	for (int32 i = 0; i < 12; i++)
	{
		UStaticMeshComponent* Smoke = NewObject<UStaticMeshComponent>(Cloud);
		Smoke->SetupAttachment(Cloud->GetRootComponent());
		Smoke->SetStaticMesh(SphereFinder.Object);
		Smoke->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Smoke->CastShadow = false;

		FVector Offset = FMath::VRand() * FMath::RandRange(50.f, 250.f);
		Offset.Z = FMath::Abs(Offset.Z) + FMath::RandRange(0.f, 200.f);
		Smoke->SetWorldLocation(Origin + Offset);
		float S = FMath::RandRange(2.f, 4.f);
		Smoke->SetWorldScale3D(FVector(S, S, S * 0.7f));

		if (SmokeMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(SmokeMat, Cloud);
			float Grey = FMath::RandRange(0.15f, 0.3f);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Grey * 0.5f, Grey * 0.5f, Grey * 0.45f));
			Smoke->SetMaterial(0, Mat);
		}
		Smoke->RegisterComponent();
	}

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		Audio->PlayExplosionSound(Origin);
}
