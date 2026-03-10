// ExoWeatherRain.cpp — Mesh-based rain drop rendering
#include "Visual/ExoWeatherSystem.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

void AExoWeatherSystem::InitRainMeshes()
{
	if (bRainMeshesInitialized) return;
	bRainMeshesInitialized = true;

	static UStaticMesh* CylMesh = nullptr;
	if (!CylMesh)
	{
		CylMesh = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Engine/BasicShapes/Cylinder"));
	}
	if (!CylMesh) return;

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();

	RainMeshes.Reserve(RAIN_MESH_POOL);
	for (int32 i = 0; i < RAIN_MESH_POOL; i++)
	{
		UStaticMeshComponent* M = NewObject<UStaticMeshComponent>(this);
		M->SetupAttachment(RootComponent);
		M->SetStaticMesh(CylMesh);
		M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		M->CastShadow = false;
		M->SetGenerateOverlapEvents(false);
		M->SetVisibility(false);
		// Thin, elongated streak — rain drop visual
		M->SetRelativeScale3D(FVector(0.02f, 0.02f, 1.2f));

		if (BaseMat)
		{
			UMaterialInstanceDynamic* RM = UMaterialInstanceDynamic::Create(BaseMat, this);
			// Cool blue-white rain streaks
			RM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.4f, 0.5f, 0.8f));
			M->SetMaterial(0, RM);
		}

		M->RegisterComponent();
		RainMeshes.Add(M);
	}
}

void AExoWeatherSystem::UpdateRainMeshes()
{
	if (!bRainMeshesInitialized)
	{
		// Only init meshes when rain first starts
		bool bAnyRain = bCurrentRaining || bTargetRaining;
		if (!bAnyRain) return;
		InitRainMeshes();
	}

	int32 ActiveDrops = RainDrops.Num();
	int32 MeshCount = RainMeshes.Num();

	for (int32 i = 0; i < MeshCount; i++)
	{
		if (i < ActiveDrops)
		{
			const FRainDrop& D = RainDrops[i];
			RainMeshes[i]->SetWorldLocation(D.Position);
			// Orient streak along velocity direction
			FRotator DropRot = D.Velocity.Rotation();
			DropRot.Pitch += 90.f;
			RainMeshes[i]->SetWorldRotation(DropRot);
			// Scale streak length based on fall speed (faster = longer)
			float Speed = D.Velocity.Size();
			float StretchZ = FMath::Clamp(Speed / 3000.f, 0.8f, 2.0f);
			RainMeshes[i]->SetRelativeScale3D(
				FVector(0.02f, 0.02f, StretchZ));
			// Fade as drop ages
			float Alpha = FMath::Clamp(1.f - D.Life / 1.5f, 0.f, 1.f);
			RainMeshes[i]->SetVisibility(Alpha > 0.05f);
		}
		else
		{
			RainMeshes[i]->SetVisibility(false);
		}
	}
}
