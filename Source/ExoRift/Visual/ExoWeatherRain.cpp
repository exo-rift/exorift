// ExoWeatherRain.cpp — Rain rendering + lightning bolt integration
#include "Visual/ExoWeatherSystem.h"
#include "Visual/ExoLightningBolt.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoScreenShake.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
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
			if (!RM) { return; }
			// Cool blue-white rain streaks
			RM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.9f, 1.1f, 1.8f));
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

// ---------------------------------------------------------------------------
// Lightning — multi-flash bursts with bolt mesh spawning
// ---------------------------------------------------------------------------

void AExoWeatherSystem::UpdateLightning(float DeltaTime)
{
	bool bStormActive = (CurrentWeather == EExoWeatherState::Storm
		|| TargetWeather == EExoWeatherState::Storm);

	if (bStormActive)
	{
		if (LightningFlashesRemaining > 0)
		{
			MultiFlashDelay -= DeltaTime;
			if (MultiFlashDelay <= 0.f)
			{
				LightningAlpha = FMath::RandRange(0.6f, 1.f);
				LightningBoltAlpha = LightningAlpha;
				LightningFlashesRemaining--;
				MultiFlashDelay = FMath::RandRange(0.05f, 0.15f);

				if (WeatherLightComp)
				{
					WeatherLightComp->SetIntensity(18.f * LightningAlpha);
					WeatherLightComp->SetLightColor(FLinearColor(1.8f, 1.9f, 2.2f));
				}

				if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
				{
					FExoScreenShake::AddShake(0.3f * LightningAlpha, 0.15f);
				}

				// Spawn a visible lightning bolt mesh near the player
				if (LightningFlashesRemaining == 0)
				{
					APlayerController* LPC = GetWorld()->GetFirstPlayerController();
					if (LPC && LPC->GetPawn())
					{
						FVector PLoc = LPC->GetPawn()->GetActorLocation();
						FVector StrikePos = PLoc + FVector(
							FMath::RandRange(-8000.f, 8000.f),
							FMath::RandRange(-8000.f, 8000.f), 0.f);
						AExoLightningBolt* Bolt = GetWorld()->SpawnActor<AExoLightningBolt>(
							AExoLightningBolt::StaticClass(), StrikePos,
							FRotator::ZeroRotator);
						if (Bolt) Bolt->InitBolt(StrikePos);
					}
				}
			}
		}
		else if (LightningCooldown > 0.f)
		{
			LightningCooldown -= DeltaTime;
		}
		else if (LightningAlpha <= 0.01f)
		{
			LightningFlashesRemaining = FMath::RandRange(1, 3);
			MultiFlashDelay = 0.f;
			LightningCooldown = FMath::RandRange(3.f, 10.f);
		}
	}

	LightningAlpha = FMath::Max(LightningAlpha - DeltaTime * 5.f, 0.f);
	LightningBoltAlpha = FMath::Max(LightningBoltAlpha - DeltaTime * 8.f, 0.f);

	if (!bStormActive || LightningAlpha < 0.01f)
	{
		if (WeatherLightComp)
		{
			float FillIntensity = (1.f - CurrentVisibility) * 1.1f;
			WeatherLightComp->SetIntensity(
				FMath::FInterpTo(WeatherLightComp->Intensity, FillIntensity,
					DeltaTime, 5.f));
			WeatherLightComp->SetLightColor(FLinearColor(0.66f, 0.77f, 1.1f));
		}
	}
}
