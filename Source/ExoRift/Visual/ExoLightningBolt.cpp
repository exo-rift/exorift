// ExoLightningBolt.cpp — Procedural jagged lightning bolt from sky to ground
#include "Visual/ExoLightningBolt.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoLightningBolt::AExoLightningBolt()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFinder.Succeeded()) CylinderMesh = CylFinder.Object;

	StrikeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StrikeLight"));
	StrikeLight->SetupAttachment(RootComponent);
	StrikeLight->SetIntensity(0.f);
	StrikeLight->SetAttenuationRadius(15000.f);
	StrikeLight->SetLightColor(FLinearColor(0.7f, 0.8f, 1.f));
	StrikeLight->CastShadows = false;

	SkyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->SetIntensity(0.f);
	SkyLight->SetAttenuationRadius(25000.f);
	SkyLight->SetLightColor(FLinearColor(0.6f, 0.7f, 1.f));
	SkyLight->CastShadows = false;
}

void AExoLightningBolt::InitBolt(const FVector& StrikePos, float BoltHeight)
{
	SetActorLocation(StrikePos);
	StrikeLight->SetWorldLocation(StrikePos + FVector(0.f, 0.f, 200.f));
	SkyLight->SetWorldLocation(StrikePos + FVector(0.f, 0.f, BoltHeight * 0.5f));

	FVector Top = StrikePos + FVector(
		FMath::RandRange(-2000.f, 2000.f),
		FMath::RandRange(-2000.f, 2000.f),
		BoltHeight);

	BuildBoltSegments(Top, StrikePos, 0);

	StrikeLight->SetIntensity(BaseIntensity);
	SkyLight->SetIntensity(BaseIntensity * 0.6f);
}

void AExoLightningBolt::BuildBoltSegments(const FVector& Start, const FVector& End,
	int32 Depth)
{
	if (Segments.Num() >= MAX_SEGMENTS || !CylinderMesh) return;

	FVector Dir = End - Start;
	float Length = Dir.Size();

	// Subdivide: add random displacement at midpoint for jagged look
	if (Length > 800.f && Depth < 4)
	{
		FVector Mid = (Start + End) * 0.5f;
		float Jag = Length * 0.25f;
		Mid += FVector(
			FMath::RandRange(-Jag, Jag),
			FMath::RandRange(-Jag, Jag),
			FMath::RandRange(-Jag * 0.3f, Jag * 0.3f));

		BuildBoltSegments(Start, Mid, Depth + 1);
		BuildBoltSegments(Mid, End, Depth + 1);

		// Branch: 30% chance to spawn a side bolt from midpoint
		if (Depth < 2 && FMath::FRand() < 0.3f && Segments.Num() < MAX_SEGMENTS - 2)
		{
			FVector BranchEnd = Mid + FVector(
				FMath::RandRange(-1500.f, 1500.f),
				FMath::RandRange(-1500.f, 1500.f),
				FMath::RandRange(-2000.f, -500.f));
			BuildBoltSegments(Mid, BranchEnd, Depth + 2);
		}
		return;
	}

	// Create a single bolt segment (cylinder connecting Start to End)
	UStaticMeshComponent* Seg = NewObject<UStaticMeshComponent>(this);
	Seg->SetupAttachment(RootComponent);
	Seg->SetStaticMesh(CylinderMesh);
	Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Seg->CastShadow = false;
	Seg->SetGenerateOverlapEvents(false);

	FVector MidPt = (Start + End) * 0.5f;
	FRotator SegRot = Dir.Rotation();
	SegRot.Pitch += 90.f;

	Seg->SetWorldLocation(MidPt);
	Seg->SetWorldRotation(SegRot);

	// Thicker for main bolt, thinner for branches
	float Thickness = (Depth < 2) ? 0.12f : 0.06f;
	float ZScale = Length / 100.f;
	Seg->SetWorldScale3D(FVector(Thickness, Thickness, ZScale));

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (BaseMat)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (!Mat) { return; }
		float Bright = (Depth < 2) ? 55.f : 28.f;
		FLinearColor EmColor(
			BoltColor.R * Bright, BoltColor.G * Bright, BoltColor.B * Bright);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), EmColor);
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"), EmColor);
		Seg->SetMaterial(0, Mat);
		SegmentMats.Add(Mat);
	}

	Seg->RegisterComponent();
	Segments.Add(Seg);
}

void AExoLightningBolt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;

	float Alpha = 1.f - FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	// Sharp initial flash then rapid decay (cubic falloff)
	float Brightness = Alpha * Alpha * Alpha;

	StrikeLight->SetIntensity(BaseIntensity * Brightness);
	SkyLight->SetIntensity(BaseIntensity * 0.6f * Brightness);

	// Flicker the bolt segments (rapid on/off for electric feel)
	float Flicker = (FMath::Sin(Age * 120.f) > -0.2f) ? 1.f : 0.3f;

	for (int32 i = 0; i < SegmentMats.Num(); i++)
	{
		float Bright = Brightness * Flicker * ((i < Segments.Num() / 2) ? 55.f : 28.f);
		FLinearColor EmColor(
			BoltColor.R * Bright, BoltColor.G * Bright, BoltColor.B * Bright);
		SegmentMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), EmColor);
	}

	// Scale segments down as bolt fades
	for (UStaticMeshComponent* Seg : Segments)
	{
		FVector S = Seg->GetComponentScale();
		float ThickFade = FMath::Max(S.X * Alpha, 0.01f);
		Seg->SetWorldScale3D(FVector(ThickFade, ThickFade, S.Z));
	}

	if (Age >= Lifetime) Destroy();
}
