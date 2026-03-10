#include "Visual/ExoAmbientParticles.h"
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
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));

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

	// Apply default dust mote material
	if (MatFinder.Succeeded())
	{
		for (int32 i = 0; i < NUM_MOTES; i++)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
				MatFinder.Object, this);
			FLinearColor DustCol(0.5f, 0.45f, 0.4f, 1.f);
			// Slight emissive so they're visible in shadow
			FLinearColor EmissiveCol(0.3f, 0.28f, 0.25f, 1.f);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), DustCol);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), EmissiveCol);
			MoteMeshes[i]->SetMaterial(0, Mat);
		}
	}
}

void AExoAmbientParticles::SetStyle(bool bEnergyWisps)
{
	bIsEnergyStyle = bEnergyWisps;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;

	for (int32 i = 0; i < MoteMeshes.Num(); i++)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
			MatFinder.Object, this);
		if (bEnergyWisps)
		{
			// Glowing cyan/blue energy wisps
			float Hue = FMath::RandRange(0.f, 1.f);
			FLinearColor Col(
				0.1f + Hue * 0.3f,
				0.5f + Hue * 0.3f,
				2.f + (1.f - Hue) * 3.f, 1.f);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Col);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Col);
			Motes[i].BaseScale = FMath::RandRange(0.02f, 0.05f);
		}
		else
		{
			// Dust motes
			FLinearColor DustCol(0.5f, 0.45f, 0.4f, 1.f);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), DustCol);
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
