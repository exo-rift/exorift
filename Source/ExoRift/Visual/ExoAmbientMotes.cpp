// ExoAmbientMotes.cpp — Floating energy particles for sci-fi atmosphere
#include "Visual/ExoAmbientMotes.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AExoAmbientMotes::AExoAmbientMotes()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // 20Hz is plenty for ambient drift

	SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ClusterLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ClusterLight"));
	ClusterLight->SetupAttachment(RootComponent);
	ClusterLight->SetIntensity(3000.f);
	ClusterLight->SetAttenuationRadius(2500.f);
	ClusterLight->CastShadows = false;
}

void AExoAmbientMotes::InitMotes(const FLinearColor& Color, float InRadius, int32 Count)
{
	MoteColor = Color;
	ClusterRadius = InRadius;
	ClusterLight->SetLightColor(Color);

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!BaseMat || !SphereMesh) return;

	for (int32 i = 0; i < Count; i++)
	{
		FMote M;

		// Random position within sphere
		FVector Offset(
			FMath::RandRange(-ClusterRadius, ClusterRadius),
			FMath::RandRange(-ClusterRadius, ClusterRadius),
			FMath::RandRange(-ClusterRadius * 0.3f, ClusterRadius * 0.5f));
		M.BaseOffset = Offset;

		// Slow drift velocity
		M.DriftVelocity = FVector(
			FMath::RandRange(-30.f, 30.f),
			FMath::RandRange(-30.f, 30.f),
			FMath::RandRange(-8.f, 15.f));

		// Vertical bob parameters — each mote has unique rhythm
		M.BobPhase = FMath::RandRange(0.f, PI * 2.f);
		M.BobFreq = FMath::RandRange(0.3f, 0.8f);
		M.BobAmp = FMath::RandRange(20.f, 60.f);
		M.PulsePhase = FMath::RandRange(0.f, PI * 2.f);

		M.Mesh = NewObject<UStaticMeshComponent>(this);
		M.Mesh->SetupAttachment(RootComponent);
		M.Mesh->SetStaticMesh(SphereMesh);
		M.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		M.Mesh->CastShadow = false;
		M.Mesh->SetGenerateOverlapEvents(false);
		M.Mesh->SetRelativeLocation(Offset);

		// Random size: tiny glowing orbs
		float S = FMath::RandRange(0.03f, 0.08f);
		M.BaseSize = S;
		M.Mesh->SetRelativeScale3D(FVector(S));

		// Emissive material with brightness variation
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (!Mat) { continue; }
		float Brightness = FMath::RandRange(40.f, 120.f);
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * Brightness, Color.G * Brightness, Color.B * Brightness));
		M.Mesh->SetMaterial(0, Mat);
		M.Mesh->RegisterComponent();

		Motes.Add(M);
	}
}

void AExoAmbientMotes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	for (FMote& M : Motes)
	{
		if (!M.Mesh) continue;

		// Drift position over time (wraps within cluster bounds)
		M.BaseOffset += M.DriftVelocity * DeltaTime;

		// Wrap drift to keep within cluster radius
		for (int32 Axis = 0; Axis < 3; Axis++)
		{
			float Limit = (Axis == 2) ? ClusterRadius * 0.5f : ClusterRadius;
			if (M.BaseOffset[Axis] > Limit) M.BaseOffset[Axis] = -Limit;
			if (M.BaseOffset[Axis] < -Limit) M.BaseOffset[Axis] = Limit;
		}

		// Vertical bob
		float Bob = FMath::Sin(Time * M.BobFreq + M.BobPhase) * M.BobAmp;
		FVector Pos = M.BaseOffset + FVector(0.f, 0.f, Bob);
		M.Mesh->SetRelativeLocation(Pos);

		// Gentle scale pulse
		float Pulse = 0.8f + 0.2f * FMath::Sin(Time * 1.5f + M.PulsePhase);
		M.Mesh->SetRelativeScale3D(FVector(M.BaseSize * Pulse));
	}

	// Cluster light gentle breathe
	float LightPulse = 0.7f + 0.3f * FMath::Sin(Time * 0.8f);
	ClusterLight->SetIntensity(3000.f * LightPulse);
}

void AExoAmbientMotes::SpawnClusters(UWorld* World)
{
	if (!World) return;

	// Cluster definitions: position, color, radius, count
	struct FClusterDef
	{
		FVector Pos;
		FLinearColor Color;
		float Radius;
		int32 Count;
	};

	TArray<FClusterDef> Clusters = {
		// Central hub — cyan energy
		{FVector(0.f, 0.f, 400.f), FLinearColor(0.15f, 0.55f, 1.f), 3000.f, 30},
		// North industrial — amber sparks
		{FVector(0.f, 16000.f, 300.f), FLinearColor(1.f, 0.65f, 0.15f), 2500.f, 25},
		// South research — teal
		{FVector(-8000.f, -16000.f, 350.f), FLinearColor(0.1f, 0.8f, 0.6f), 2800.f, 28},
		// East power — red warning particles
		{FVector(16000.f, 0.f, 300.f), FLinearColor(1.f, 0.2f, 0.1f), 2200.f, 22},
		// West barracks — green
		{FVector(-16000.f, 0.f, 280.f), FLinearColor(0.15f, 0.9f, 0.3f), 2000.f, 20},
		// Open field clusters — soft purple
		{FVector(8000.f, 8000.f, 500.f), FLinearColor(0.5f, 0.2f, 0.9f), 4000.f, 35},
		{FVector(-12000.f, 12000.f, 400.f), FLinearColor(0.4f, 0.25f, 0.85f), 3500.f, 30},
		// Near the obelisk — bright white-cyan
		{FVector(0.f, 0.f, 1200.f), FLinearColor(0.6f, 0.8f, 1.f), 1500.f, 40},
	};

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FClusterDef& Def : Clusters)
	{
		AExoAmbientMotes* Cluster = World->SpawnActor<AExoAmbientMotes>(
			AExoAmbientMotes::StaticClass(), Def.Pos, FRotator::ZeroRotator, Params);
		if (Cluster)
		{
			Cluster->InitMotes(Def.Color, Def.Radius, Def.Count);
		}
	}
}
