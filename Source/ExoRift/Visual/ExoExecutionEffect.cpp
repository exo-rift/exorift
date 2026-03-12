// ExoExecutionEffect.cpp — Dramatic execution finisher VFX with vortex and converging beams
#include "Visual/ExoExecutionEffect.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoExecutionEffect::AExoExecutionEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = Lifetime + 0.5f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	UStaticMesh* SphereMesh = SphereFinder.Succeeded() ? SphereFinder.Object : nullptr;
	UStaticMesh* CylMesh = CylFinder.Succeeded() ? CylFinder.Object : nullptr;
	UStaticMesh* CubeMesh = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;

	auto MakeMesh = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) C->SetStaticMesh(Mesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		return C;
	};

	CoreVortex = MakeMesh(TEXT("CoreVortex"), SphereMesh);
	RootComponent = CoreVortex;

	InnerRing = MakeMesh(TEXT("InnerRing"), CylMesh);
	InnerRing->SetupAttachment(CoreVortex);

	OuterRing = MakeMesh(TEXT("OuterRing"), CylMesh);
	OuterRing->SetupAttachment(CoreVortex);

	EnergyColumn = MakeMesh(TEXT("EnergyColumn"), CylMesh);
	EnergyColumn->SetupAttachment(CoreVortex);

	GroundBurn = MakeMesh(TEXT("GroundBurn"), CylMesh);
	GroundBurn->SetupAttachment(CoreVortex);

	for (int32 i = 0; i < NumShards; i++)
	{
		FName SName = *FString::Printf(TEXT("Shard_%d"), i);
		UStaticMeshComponent* S = MakeMesh(*SName.ToString(), CubeMesh);
		S->SetupAttachment(CoreVortex);
		Shards.Add(S);
	}

	for (int32 i = 0; i < NumBeams; i++)
	{
		FName BName = *FString::Printf(TEXT("Beam_%d"), i);
		UStaticMeshComponent* B = MakeMesh(*BName.ToString(), CylMesh);
		B->SetupAttachment(CoreVortex);
		Beams.Add(B);
	}

	FlashLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FlashLight"));
	FlashLight->SetupAttachment(CoreVortex);
	FlashLight->SetIntensity(0.f);
	FlashLight->SetAttenuationRadius(6000.f);
	FlashLight->CastShadows = false;

	ColumnLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ColumnLight"));
	ColumnLight->SetupAttachment(CoreVortex);
	ColumnLight->SetRelativeLocation(FVector(0.f, 0.f, 400.f));
	ColumnLight->SetIntensity(0.f);
	ColumnLight->SetAttenuationRadius(3000.f);
	ColumnLight->CastShadows = false;
}

void AExoExecutionEffect::Init(const FLinearColor& AccentColor, const FVector& ExecutorPos)
{
	Accent = AccentColor;
	ExecutorDirection = (GetActorLocation() - ExecutorPos).GetSafeNormal2D();

	UMaterialInterface* AdditiveMat = FExoMaterialFactory::GetEmissiveAdditive();
	UMaterialInterface* OpaqueMat = FExoMaterialFactory::GetEmissiveOpaque();
	if (!AdditiveMat) return;

	FLinearColor BrightAccent(Accent.R * 40.f, Accent.G * 40.f, Accent.B * 40.f);
	FLinearColor HotWhite(80.f, 75.f, 60.f);

	// Core vortex — intensely bright sphere
	VortexMat = UMaterialInstanceDynamic::Create(AdditiveMat, this);
	if (!VortexMat) { return; }
	VortexMat->SetVectorParameterValue(TEXT("EmissiveColor"), HotWhite);
	CoreVortex->SetMaterial(0, VortexMat);
	CoreVortex->SetRelativeScale3D(FVector(0.1f));

	// Inner ring — tight spinning halo
	RingMat = UMaterialInstanceDynamic::Create(AdditiveMat, this);
	if (!RingMat) { return; }
	RingMat->SetVectorParameterValue(TEXT("EmissiveColor"), BrightAccent);
	InnerRing->SetMaterial(0, RingMat);
	InnerRing->SetRelativeScale3D(FVector(2.f, 2.f, 0.02f));

	// Outer ring — wider, dimmer
	UMaterialInstanceDynamic* OuterMat = UMaterialInstanceDynamic::Create(AdditiveMat, this);
	if (!OuterMat) { return; }
	OuterMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(Accent.R * 15.f, Accent.G * 15.f, Accent.B * 15.f));
	OuterRing->SetMaterial(0, OuterMat);
	OuterRing->SetRelativeScale3D(FVector(4.f, 4.f, 0.015f));

	// Energy column — rising pillar
	UMaterialInstanceDynamic* ColMat = UMaterialInstanceDynamic::Create(AdditiveMat, this);
	if (!ColMat) { return; }
	ColMat->SetVectorParameterValue(TEXT("EmissiveColor"), BrightAccent * 0.6f);
	EnergyColumn->SetMaterial(0, ColMat);
	EnergyColumn->SetRelativeLocation(FVector(0.f, 0.f, 300.f));
	EnergyColumn->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.01f));

	// Ground burn mark
	if (OpaqueMat)
	{
		UMaterialInstanceDynamic* BurnMat = UMaterialInstanceDynamic::Create(OpaqueMat, this);
		if (!BurnMat) { return; }
		BurnMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Accent.R * 3.f, Accent.G * 3.f, Accent.B * 3.f));
		GroundBurn->SetMaterial(0, BurnMat);
	}
	GroundBurn->SetRelativeLocation(FVector(0.f, 0.f, -45.f));
	GroundBurn->SetRelativeScale3D(FVector(3.f, 3.f, 0.01f));

	// Orbiting shards
	ShardAngles.SetNum(NumShards);
	ShardHeights.SetNum(NumShards);
	for (int32 i = 0; i < NumShards; i++)
	{
		ShardAngles[i] = (float)i / NumShards * 2.f * PI;
		ShardHeights[i] = FMath::RandRange(-50.f, 200.f);
		float S = FMath::RandRange(0.15f, 0.35f);
		Shards[i]->SetRelativeScale3D(FVector(S, S * 0.4f, S * 3.f));
		UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(AdditiveMat, this);
		if (!SM) { continue; }
		FLinearColor ShardCol = BrightAccent * FMath::RandRange(0.5f, 1.5f);
		SM->SetVectorParameterValue(TEXT("EmissiveColor"), ShardCol);
		Shards[i]->SetMaterial(0, SM);
	}

	// Converging beams — energy streaks from executor direction
	for (int32 i = 0; i < NumBeams; i++)
	{
		float Angle = (float)i / NumBeams * 2.f * PI;
		float BeamS = FMath::RandRange(0.03f, 0.06f);
		Beams[i]->SetRelativeScale3D(FVector(BeamS, BeamS, 4.f));
		UMaterialInstanceDynamic* BM = UMaterialInstanceDynamic::Create(AdditiveMat, this);
		if (!BM) { continue; }
		BM->SetVectorParameterValue(TEXT("EmissiveColor"), BrightAccent * 0.8f);
		Beams[i]->SetMaterial(0, BM);
		float Dist = 500.f;
		FVector BeamPos(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 100.f);
		Beams[i]->SetRelativeLocation(BeamPos);
		FRotator BeamRot = (-BeamPos).Rotation();
		BeamRot.Pitch += 90.f;
		Beams[i]->SetRelativeRotation(BeamRot);
	}

	// Lights
	FlashLight->SetLightColor(Accent);
	FlashLight->SetIntensity(600000.f);
	ColumnLight->SetLightColor(Accent);
	ColumnLight->SetIntensity(200000.f);
}

void AExoExecutionEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;
	if (Age > Lifetime) { Destroy(); return; }

	float T = Age / Lifetime;
	float Alpha = 1.f - FMath::Clamp((T - 0.7f) / 0.3f, 0.f, 1.f);

	// Core vortex: expand then collapse
	float CoreScale;
	if (T < 0.3f)
		CoreScale = FMath::Lerp(0.1f, 3.5f, T / 0.3f);
	else if (T < 0.7f)
		CoreScale = 3.5f + FMath::Sin((T - 0.3f) / 0.4f * PI * 4.f) * 0.3f;
	else
		CoreScale = FMath::Lerp(3.5f, 0.f, (T - 0.7f) / 0.3f);
	CoreVortex->SetRelativeScale3D(FVector(CoreScale));

	// Rings: spin and expand
	float RingYaw = Age * 720.f;
	float InnerExpand = FMath::Lerp(2.f, 6.f, FMath::Min(T * 2.f, 1.f));
	float OuterExpand = FMath::Lerp(4.f, 12.f, FMath::Min(T * 1.5f, 1.f));
	InnerRing->SetRelativeScale3D(FVector(InnerExpand * Alpha, InnerExpand * Alpha, 0.02f));
	InnerRing->SetRelativeRotation(FRotator(0.f, RingYaw, 0.f));
	OuterRing->SetRelativeScale3D(FVector(OuterExpand * Alpha, OuterExpand * Alpha, 0.015f));
	OuterRing->SetRelativeRotation(FRotator(0.f, -RingYaw * 0.6f, 0.f));

	// Energy column: rises and widens
	float ColH = FMath::Lerp(0.01f, 15.f, FMath::Min(T * 3.f, 1.f)) * Alpha;
	float ColW = FMath::Lerp(0.5f, 0.8f, T) * Alpha;
	EnergyColumn->SetRelativeScale3D(FVector(ColW, ColW, ColH));
	EnergyColumn->SetRelativeLocation(FVector(0.f, 0.f, ColH * 50.f * 0.5f));

	// Ground burn: expands slightly
	float BurnScale = FMath::Lerp(3.f, 5.f, T);
	GroundBurn->SetRelativeScale3D(FVector(BurnScale, BurnScale, 0.01f));

	// Orbiting shards: spiral inward then explode outward
	for (int32 i = 0; i < NumShards; i++)
	{
		ShardAngles[i] += DeltaTime * (8.f + T * 20.f);
		float Radius;
		if (T < 0.6f)
			Radius = FMath::Lerp(300.f, 50.f, T / 0.6f);
		else
			Radius = FMath::Lerp(50.f, 800.f, (T - 0.6f) / 0.4f);

		float SH = ShardHeights[i] + T * 200.f;
		FVector Pos(FMath::Cos(ShardAngles[i]) * Radius,
			FMath::Sin(ShardAngles[i]) * Radius, SH);
		Shards[i]->SetRelativeLocation(Pos);
		Shards[i]->SetRelativeRotation(FRotator(Age * 400.f, ShardAngles[i] * 57.3f, 0.f));

		float ShardAlpha = (T > 0.85f) ? FMath::Clamp(1.f - (T - 0.85f) / 0.15f, 0.f, 1.f) : 1.f;
		float S = Shards[i]->GetRelativeScale3D().X * ShardAlpha;
		Shards[i]->SetRelativeScale3D(FVector(S, S * 0.4f, S * 3.f));
	}

	// Converging beams: rush inward
	for (int32 i = 0; i < NumBeams; i++)
	{
		float BeamT = FMath::Clamp(T * 2.f, 0.f, 1.f);
		float Dist = FMath::Lerp(500.f, 0.f, BeamT);
		float Angle = (float)i / NumBeams * 2.f * PI + Age * 3.f;
		FVector Pos(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 100.f * Alpha);
		Beams[i]->SetRelativeLocation(Pos);
		float BeamAlpha = (T > 0.5f) ? FMath::Max(0.f, 1.f - (T - 0.5f) * 2.f) : 1.f;
		float BS = Beams[i]->GetRelativeScale3D().X;
		float BH = FMath::Lerp(4.f, 0.5f, BeamT) * BeamAlpha;
		Beams[i]->SetRelativeScale3D(FVector(BS, BS, BH));
	}

	// Lights: flash then decay
	float FlashI = (T < 0.1f)
		? FMath::Lerp(600000.f, 900000.f, T / 0.1f)
		: 900000.f * FMath::Max(0.f, 1.f - (T - 0.1f) / 0.9f);
	FlashLight->SetIntensity(FlashI);

	float ColI = (T < 0.2f)
		? FMath::Lerp(200000.f, 400000.f, T / 0.2f)
		: 400000.f * FMath::Max(0.f, 1.f - (T - 0.2f) / 0.8f);
	ColumnLight->SetIntensity(ColI);
}
