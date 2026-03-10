#include "Visual/ExoAmbientParticles.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AExoAmbientParticles::AExoAmbientParticles()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	for (int32 i = 0; i < NUM_MOTES; i++)
	{
		FName Name = *FString::Printf(TEXT("Mote_%d"), i);
		UStaticMeshComponent* M = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		M->SetupAttachment(RootComponent);
		M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		M->CastShadow = false;
		M->SetGenerateOverlapEvents(false);
		if (SphereFinder.Succeeded()) M->SetStaticMesh(SphereFinder.Object);
		MoteMeshes.Add(M);
	}

	Motes.SetNum(NUM_MOTES);
	for (int32 i = 0; i < NUM_MOTES; i++)
	{
		FMote& Mt = Motes[i];
		Mt.Offset = FVector(
			FMath::RandRange(-SpawnRadius, SpawnRadius),
			FMath::RandRange(-SpawnRadius, SpawnRadius),
			FMath::RandRange(-200.f, 400.f));
		Mt.Drift = FVector(
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(-1.f, 1.f),
			FMath::RandRange(0.1f, 0.6f)).GetSafeNormal() * DriftSpeed;
		Mt.Phase = FMath::RandRange(0.f, PI * 2.f);
		Mt.BaseScale = FMath::RandRange(0.015f, 0.035f);
	}

	// Apply default dust mote material — emissive opaque so glow is visible in shadow
	{
		UMaterialInterface* DustMat = FExoMaterialFactory::GetEmissiveOpaque();
		for (int32 i = 0; i < NUM_MOTES; i++)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(DustMat, this);
			FLinearColor EmissiveCol(0.3f, 0.28f, 0.25f, 1.f);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), EmissiveCol);
			MoteMeshes[i]->SetMaterial(0, Mat);
		}
	}
}

void AExoAmbientParticles::SetStyle(bool bEnergyWisps)
{
	bIsEnergyStyle = bEnergyWisps;

	UMaterialInterface* ParentMat = bEnergyWisps
		? FExoMaterialFactory::GetEmissiveAdditive()
		: FExoMaterialFactory::GetEmissiveOpaque();

	for (int32 i = 0; i < MoteMeshes.Num(); i++)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ParentMat, this);
		if (bEnergyWisps)
		{
			// Varied energy wisps: cyan, teal, violet, white — some bright, some dim
			FLinearColor Col;
			int32 Style = i % 5;
			if (Style < 2) // Cyan
				Col = FLinearColor(0.1f, 0.6f, 3.5f + FMath::RandRange(0.f, 2.f));
			else if (Style < 3) // Teal-green
				Col = FLinearColor(0.05f, 1.5f + FMath::RandRange(0.f, 1.f), 1.0f);
			else if (Style < 4) // Violet
				Col = FLinearColor(0.8f + FMath::RandRange(0.f, 0.5f), 0.15f, 2.5f);
			else // Warm white
				Col = FLinearColor(1.5f, 1.3f, 1.0f);

			// Random brightness variance
			float Bright = FMath::RandRange(0.6f, 1.4f);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Col * Bright);
			Motes[i].BaseScale = FMath::RandRange(0.015f, 0.06f);
		}
		else
		{
			// Dust motes
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.3f, 0.28f, 0.25f, 1.f));
		}
		MoteMeshes[i]->SetMaterial(0, Mat);
	}
}

void AExoAmbientParticles::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Follow local player
	APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (LocalPawn)
	{
		SetActorLocation(LocalPawn->GetActorLocation());
	}

	float Time = GetWorld()->GetTimeSeconds();

	for (int32 i = 0; i < NUM_MOTES; i++)
	{
		FMote& Mt = Motes[i];

		// Drift
		Mt.Offset += Mt.Drift * DeltaTime;
		// Gentle sinusoidal wander
		Mt.Offset.X += FMath::Sin(Time * 0.3f + Mt.Phase) * 5.f * DeltaTime;
		Mt.Offset.Y += FMath::Cos(Time * 0.25f + Mt.Phase * 1.3f) * 5.f * DeltaTime;

		// Wrap back into radius if too far
		float Dist2D = FVector2D(Mt.Offset.X, Mt.Offset.Y).Size();
		if (Dist2D > SpawnRadius)
		{
			Mt.Offset.X = FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f);
			Mt.Offset.Y = FMath::RandRange(-SpawnRadius * 0.5f, SpawnRadius * 0.5f);
			Mt.Offset.Z = FMath::RandRange(-100.f, 300.f);
		}
		if (Mt.Offset.Z > 500.f) Mt.Offset.Z = -100.f;

		MoteMeshes[i]->SetRelativeLocation(Mt.Offset);

		// Gentle scale pulse
		float Pulse = 1.f + 0.3f * FMath::Sin(Time * 1.5f + Mt.Phase);
		float S = Mt.BaseScale * Pulse;
		MoteMeshes[i]->SetRelativeScale3D(FVector(S));
	}
}

AExoAmbientParticles* AExoAmbientParticles::Get(UWorld* World)
{
	if (!World) return nullptr;
	for (TActorIterator<AExoAmbientParticles> It(World); It; ++It)
	{
		return *It;
	}
	return World->SpawnActor<AExoAmbientParticles>();
}
