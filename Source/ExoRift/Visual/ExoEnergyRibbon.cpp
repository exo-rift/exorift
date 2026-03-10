// ExoEnergyRibbon.cpp — Persistent glowing contrail left by energy tracers
#include "Visual/ExoEnergyRibbon.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoEnergyRibbon::AExoEnergyRibbon()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 3.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	// Create segment meshes
	for (int32 i = 0; i < NUM_SEGMENTS; i++)
	{
		FName Name = *FString::Printf(TEXT("Seg_%d"), i);
		UStaticMeshComponent* Seg = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Seg->CastShadow = false;
		Seg->SetGenerateOverlapEvents(false);
		if (CylinderMesh) Seg->SetStaticMesh(CylinderMesh);
		if (i == 0)
			RootComponent = Seg;
		else
			Seg->SetupAttachment(RootComponent);
		Segments.Add(Seg);
	}
}

void AExoEnergyRibbon::InitRibbon(const FVector& Start, const FVector& End,
	const FLinearColor& Color, float Thickness)
{
	RibbonColor = Color;
	RibbonThickness = Thickness;

	FVector Dir = (End - Start);
	float TotalLen = Dir.Size();
	Dir.Normalize();

	FRotator SegRot = Dir.Rotation();
	SegRot.Pitch += 90.f;

	float SegLen = TotalLen / NUM_SEGMENTS;

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!BaseMat) return;

	SegmentDrifts.SetNum(NUM_SEGMENTS);

	for (int32 i = 0; i < NUM_SEGMENTS; i++)
	{
		float T = (i + 0.5f) / NUM_SEGMENTS;
		FVector SegCenter = Start + Dir * (SegLen * (i + 0.5f));

		// Each segment gets a random drift direction for organic dispersal
		SegmentDrifts[i] = FVector(
			FMath::RandRange(-40.f, 40.f),
			FMath::RandRange(-40.f, 40.f),
			FMath::RandRange(5.f, 25.f));

		Segments[i]->SetWorldLocation(SegCenter);
		Segments[i]->SetWorldRotation(SegRot);

		// Thicker in the middle, thinner at ends
		float WidthMod = 1.f - FMath::Abs(T - 0.5f) * 1.2f;
		float R = 0.06f * Thickness * FMath::Max(WidthMod, 0.3f);
		float L = SegLen / 100.f;
		Segments[i]->SetWorldScale3D(FVector(R, R, L * 0.55f));

		// Emissive: bright core with some white blended in
		float Brightness = 25.f * (0.7f + 0.3f * WidthMod);
		FLinearColor SegColor(
			Color.R * Brightness + 5.f,
			Color.G * Brightness + 5.f,
			Color.B * Brightness + 5.f);

		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"), SegColor);
		Segments[i]->SetMaterial(0, Mat);
		SegmentMats.Add(Mat);
	}
}

void AExoEnergyRibbon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	// Slow start, fast end fade curve
	float Alpha = FMath::Pow(1.f - T, 1.5f);

	for (int32 i = 0; i < Segments.Num(); i++)
	{
		if (!Segments[i] || !SegmentMats.IsValidIndex(i)) continue;

		// Drift outward and upward as it disperses
		FVector Drift = SegmentDrifts[i] * DeltaTime;
		Segments[i]->AddWorldOffset(Drift);

		// Expand and fade — ribbon widens as it dissipates
		FVector Scale = Segments[i]->GetComponentScale();
		float Expand = 1.f + T * 2.5f;
		float Shrink = FMath::Max(Alpha, 0.01f);
		Segments[i]->SetWorldScale3D(FVector(
			Scale.X * Expand * 0.98f, Scale.Y * Expand * 0.98f,
			Scale.Z * Shrink));

		// Fade emissive
		float Brightness = 25.f * Alpha * Alpha;
		FLinearColor FadeColor(
			RibbonColor.R * Brightness + 3.f * Alpha,
			RibbonColor.G * Brightness + 3.f * Alpha,
			RibbonColor.B * Brightness + 3.f * Alpha);
		SegmentMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"), FadeColor);
	}

	if (Age >= Lifetime) Destroy();
}

void AExoEnergyRibbon::SpawnRibbon(UWorld* World, const FVector& Start, const FVector& End,
	const FLinearColor& Color, float Thickness)
{
	if (!World) return;
	if (FVector::Distance(Start, End) < 100.f) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoEnergyRibbon* Ribbon = World->SpawnActor<AExoEnergyRibbon>(
		AExoEnergyRibbon::StaticClass(), Start, FRotator::ZeroRotator, Params);
	if (Ribbon)
	{
		Ribbon->InitRibbon(Start, End, Color, Thickness);
	}
}
