#include "Weapons/ExoSmokeGrenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "Core/ExoAudioManager.h"
#include "UObject/ConstructorHelpers.h"
#include "ExoRift.h"

AExoSmokeGrenade::AExoSmokeGrenade()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(8.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetRelativeScale3D(FVector(0.12f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
		MeshComp->SetStaticMesh(SphereMesh.Object);

	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat)
	{
		auto* BodyMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!BodyMat) { return; }
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.12f, 0.13f, 0.11f));
		BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.f, 0.3f, 0.1f));
		MeshComp->SetMaterial(0, BodyMat);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
		TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.25f;
	ProjectileMovement->ProjectileGravityScale = 1.f;

	CloudLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CloudLight"));
	CloudLight->SetupAttachment(CollisionComp);
	CloudLight->SetIntensity(0.f);
	CloudLight->SetAttenuationRadius(900.f);
	CloudLight->SetLightColor(FLinearColor(0.4f, 0.45f, 0.6f));
	CloudLight->CastShadows = false;
	CloudLight->SetVisibility(false);
}

AExoSmokeGrenade* AExoSmokeGrenade::SpawnSmoke(UWorld* World,
	const FVector& Location, const FVector& ThrowVelocity)
{
	if (!World) return nullptr;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	auto* Grenade = World->SpawnActor<AExoSmokeGrenade>(
		AExoSmokeGrenade::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Grenade) Grenade->Throw(ThrowVelocity);
	return Grenade;
}

void AExoSmokeGrenade::Throw(const FVector& ThrowVelocity)
{
	bFuseLit = true;
	FuseElapsed = 0.f;
	ProjectileMovement->Velocity = ThrowVelocity;
}

void AExoSmokeGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFuseLit && !bCloudActive)
	{
		FuseElapsed += DeltaTime;
		if (FuseElapsed >= FuseTime) Detonate();
		return;
	}

	if (!bCloudActive) return;
	CloudElapsed += DeltaTime;
	if (CloudElapsed >= SmokeDuration - FadeOutTime) bFadingOut = true;
	if (CloudElapsed >= SmokeDuration) { Destroy(); return; }
	bFadingOut ? FadeOutCloud(DeltaTime) : UpdateCloud(DeltaTime);
}

void AExoSmokeGrenade::Detonate()
{
	bCloudActive = true;
	CloudElapsed = 0.f;
	CloudCenter = GetActorLocation();
	UE_LOG(LogExoRift, Log, TEXT("Smoke grenade detonated at %s"),
		*CloudCenter.ToString());

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;
	MeshComp->SetVisibility(false);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CloudLight->SetVisibility(true);
	CloudLight->SetIntensity(3000.f);
	SpawnCloudPuffs();

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		Audio->PlayExplosionSound(CloudCenter);
}

void AExoSmokeGrenade::SpawnCloudPuffs()
{
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	UMaterialInterface* SmokeMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!SphereMesh || !SmokeMat) return;

	Puffs.Reserve(NumSmokePuffs);
	for (int32 i = 0; i < NumSmokePuffs; i++)
	{
		FSmokePuff Puff;
		FVector RandDir = FMath::VRand();
		float RandDist = FMath::Pow(FMath::RandRange(0.1f, 1.f), 0.6f);
		Puff.TargetOffset = RandDir * CloudRadius * RandDist;
		Puff.TargetOffset.Z = FMath::Abs(Puff.TargetOffset.Z) * 0.6f
			+ FMath::RandRange(20.f, 150.f);
		Puff.TargetScale = FMath::RandRange(2.5f, 5.5f);
		Puff.Delay = FMath::RandRange(0.f, 0.8f);

		Puff.Mesh = NewObject<UStaticMeshComponent>(this);
		Puff.Mesh->SetupAttachment(RootComponent);
		Puff.Mesh->SetStaticMesh(SphereMesh);
		Puff.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Puff.Mesh->CastShadow = false;
		Puff.Mesh->SetWorldLocation(CloudCenter);
		Puff.Mesh->SetWorldScale3D(FVector(0.01f));

		Puff.Mat = UMaterialInstanceDynamic::Create(SmokeMat, this);
		if (!Puff.Mat) { continue; }
		float Grey = FMath::RandRange(0.5f, 0.9f);
		float BlueTint = FMath::RandRange(0.f, 0.08f);
		Puff.Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Grey * 0.8f, Grey * 0.85f, Grey + BlueTint));
		Puff.Mesh->SetMaterial(0, Puff.Mat);
		Puff.Mesh->RegisterComponent();
		Puffs.Add(Puff);
	}
}

void AExoSmokeGrenade::UpdateCloud(float DeltaTime)
{
	constexpr float ExpandTime = 2.f;
	for (FSmokePuff& P : Puffs)
	{
		if (!P.Mesh) continue;
		float Elapsed = CloudElapsed - P.Delay;
		if (Elapsed < 0.f) continue;

		float Eased = 1.f - FMath::Pow(
			1.f - FMath::Clamp(Elapsed / ExpandTime, 0.f, 1.f), 2.5f);
		FVector Pos = CloudCenter + P.TargetOffset * Eased;
		Pos.Z += CloudElapsed * 15.f;
		P.Mesh->SetWorldLocation(Pos);

		float Scale = FMath::Lerp(0.3f, P.TargetScale, Eased);
		P.Mesh->SetWorldScale3D(FVector(Scale, Scale, Scale * 0.7f));
	}

	float LightPulse = 0.85f + 0.15f * FMath::Sin(CloudElapsed * 1.5f);
	CloudLight->SetIntensity(3000.f * LightPulse);
}

void AExoSmokeGrenade::FadeOutCloud(float DeltaTime)
{
	float FadeFrac = FMath::Clamp(
		(CloudElapsed - (SmokeDuration - FadeOutTime)) / FadeOutTime, 0.f, 1.f);
	float Opacity = 1.f - FadeFrac;

	for (FSmokePuff& P : Puffs)
	{
		if (!P.Mesh || !P.Mat) continue;
		float CurScale = P.TargetScale * FMath::Lerp(1.f, 0.3f, FadeFrac);
		P.Mesh->SetWorldScale3D(FVector(CurScale, CurScale, CurScale * 0.7f));

		float Grey = FMath::RandRange(0.5f, 0.9f) * Opacity;
		P.Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Grey * 0.8f, Grey * 0.85f, Grey));

		FVector Loc = P.Mesh->GetComponentLocation();
		Loc.Z += DeltaTime * 40.f * FadeFrac;
		P.Mesh->SetWorldLocation(Loc);
	}
	CloudLight->SetIntensity(3000.f * Opacity);
}
