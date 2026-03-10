// ExoFloatingDust.cpp — Ambient floating debris motes near the player camera
#include "Map/ExoFloatingDust.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"
#include "Kismet/GameplayStatics.h"

AExoFloatingDust::AExoFloatingDust()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	// BaseMaterial no longer needed — all motes use emissive material
}

void AExoFloatingDust::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Lazy init motes on first tick (after world is ready)
	if (Motes.Num() == 0 && CubeMesh)
	{
		for (int32 i = 0; i < NUM_MOTES; i++)
		{
			FMote M;
			M.Mesh = NewObject<UStaticMeshComponent>(this);
			M.Mesh->SetStaticMesh(CubeMesh);
			M.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			M.Mesh->CastShadow = false;
			M.Mesh->SetGenerateOverlapEvents(false);

			// Tiny scale — barely visible motes
			float S = FMath::RandRange(0.02f, 0.06f);
			M.Mesh->SetWorldScale3D(FVector(S, S * 0.5f, S * 0.3f));
			M.Mesh->RegisterComponent();

			// Subtle emissive tint — cool-blue dust catching environment light
			float Brightness = FMath::RandRange(0.08f, 0.35f);
			FLinearColor Col(Brightness, Brightness * 1.1f, Brightness * 1.3f);
			UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			float Glow = FMath::RandRange(0.15f, 0.5f);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), Col * Glow);
			M.Mesh->SetMaterial(0, Mat);

			// Random position in sphere around origin
			float Radius = 3000.f;
			M.LocalOffset = FVector(
				FMath::RandRange(-Radius, Radius),
				FMath::RandRange(-Radius, Radius),
				FMath::RandRange(-Radius * 0.3f, Radius * 0.5f));

			// Very slow drift
			M.Drift = FVector(
				FMath::RandRange(-30.f, 30.f),
				FMath::RandRange(-30.f, 30.f),
				FMath::RandRange(-10.f, 10.f));

			M.Phase = FMath::RandRange(0.f, 2.f * PI);
			Motes.Add(M);
		}
	}

	// Follow camera
	APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!Cam) return;

	FVector CamLoc = Cam->GetCameraLocation();
	float Time = GetWorld()->GetTimeSeconds();
	float Radius = 3000.f;

	for (FMote& M : Motes)
	{
		// Drift the local offset slowly
		M.LocalOffset += M.Drift * DeltaTime;

		// Wrap around the camera sphere
		for (int32 Axis = 0; Axis < 3; Axis++)
		{
			float Limit = (Axis == 2) ? Radius * 0.5f : Radius;
			if (M.LocalOffset[Axis] > Limit) M.LocalOffset[Axis] -= Limit * 2.f;
			if (M.LocalOffset[Axis] < -Limit) M.LocalOffset[Axis] += Limit * 2.f;
		}

		// Gentle sinusoidal bob
		float Bob = FMath::Sin(Time * 0.5f + M.Phase) * 20.f;
		FVector WorldPos = CamLoc + M.LocalOffset + FVector(0.f, 0.f, Bob);
		M.Mesh->SetWorldLocation(WorldPos);

		// Slow tumble
		float R = Time * 15.f + M.Phase * 100.f;
		M.Mesh->SetWorldRotation(FRotator(R, R * 0.7f, R * 0.4f));
	}
}
