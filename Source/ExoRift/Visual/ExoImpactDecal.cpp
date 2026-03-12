#include "Visual/ExoImpactDecal.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoImpactDecal::AExoImpactDecal()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = TotalLifetime + 1.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	ScorchMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScorchMesh"));
	RootComponent = ScorchMesh;
	ScorchMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScorchMesh->CastShadow = false;
	ScorchMesh->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) ScorchMesh->SetStaticMesh(CylFinder.Object);
	ScorchMesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.005f));

	GlowRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowRing"));
	GlowRing->SetupAttachment(ScorchMesh);
	GlowRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GlowRing->CastShadow = false;
	GlowRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) GlowRing->SetStaticMesh(CylFinder.Object);
	GlowRing->SetRelativeScale3D(FVector(1.8f, 1.8f, 0.5f));
}

void AExoImpactDecal::InitDecal(const FVector& HitNormal, const FLinearColor& Color)
{
	// Orient flat against surface
	FRotator SurfaceRot = HitNormal.Rotation();
	SurfaceRot.Pitch -= 90.f;
	SetActorRotation(SurfaceRot);

	// Random size variation
	float S = FMath::RandRange(0.15f, 0.30f);
	ScorchMesh->SetRelativeScale3D(FVector(S, S, 0.005f));

	// Dark scorch mark (opaque PBR)
	UMaterialInterface* ScorchLitMat = FExoMaterialFactory::GetLitEmissive();
	if (ScorchLitMat)
	{
		UMaterialInstanceDynamic* ScorchMat = UMaterialInstanceDynamic::Create(ScorchLitMat, this);
		if (!ScorchMat) { return; }
		ScorchMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.02f, 0.02f, 0.02f));
		ScorchMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		ScorchMat->SetScalarParameterValue(TEXT("Metallic"), 0.1f);
		ScorchMat->SetScalarParameterValue(TEXT("Roughness"), 0.9f);
		ScorchMesh->SetMaterial(0, ScorchMat);
	}

	// Glowing ring (emissive additive for bloom)
	UMaterialInterface* GlowBaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (GlowBaseMat)
	{
		UMaterialInstanceDynamic* GlowMat = UMaterialInstanceDynamic::Create(GlowBaseMat, this);
		if (!GlowMat) { return; }
		FLinearColor GlowCol(Color.R * 105.f, Color.G * 105.f, Color.B * 105.f, 1.f);
		GlowMat->SetVectorParameterValue(TEXT("EmissiveColor"), GlowCol);
		GlowRing->SetMaterial(0, GlowMat);
	}
}

void AExoImpactDecal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;

	// Glow ring fades quickly
	if (Age < GlowDuration)
	{
		float T = Age / GlowDuration;
		float GlowAlpha = 1.f - T;
		float RingExpand = 1.8f + T * 0.5f;
		GlowRing->SetRelativeScale3D(FVector(RingExpand, RingExpand, 0.5f * GlowAlpha));
	}
	else
	{
		GlowRing->SetRelativeScale3D(FVector(0.001f));
	}

	// Scorch mark fades near end of life
	float FadeStart = TotalLifetime - FadeDuration;
	if (Age > FadeStart)
	{
		float FadeT = (Age - FadeStart) / FadeDuration;
		float ScorchScale = ScorchMesh->GetRelativeScale3D().X;
		float ShrinkScale = ScorchScale * (1.f - FadeT);
		ScorchMesh->SetRelativeScale3D(FVector(ShrinkScale, ShrinkScale, 0.005f));
	}

	if (Age >= TotalLifetime) Destroy();
}

void AExoImpactDecal::SpawnDecal(UWorld* World, const FVector& Location,
	const FVector& HitNormal, const FLinearColor& WeaponColor)
{
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Offset slightly from surface to prevent z-fighting
	FVector SpawnLoc = Location + HitNormal * 1.f;
	AExoImpactDecal* Decal = World->SpawnActor<AExoImpactDecal>(
		AExoImpactDecal::StaticClass(), FTransform(SpawnLoc), Params);
	if (Decal) Decal->InitDecal(HitNormal, WeaponColor);
}
