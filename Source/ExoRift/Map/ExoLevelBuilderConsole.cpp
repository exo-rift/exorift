// ExoLevelBuilderConsole.cpp — SpawnConsole helper for interior furniture
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoEnvironmentAnimator.h"
#include "Visual/ExoMaterialFactory.h"

void AExoLevelBuilder::SpawnConsole(const FVector& Pos, float Yaw)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FLinearColor DarkMetal(0.06f, 0.06f, 0.08f);
	FLinearColor ScreenGlow(0.05f, 0.3f, 0.6f);

	// Console body — angled desk
	SpawnStaticMesh(Pos, FVector(1.5f, 0.8f, 0.8f), Rot, CubeMesh, DarkMetal);

	// Screen panel — thin glowing rectangle on top
	FVector ScreenOffset = Rot.RotateVector(FVector(0.f, 0.f, 85.f));
	UStaticMeshComponent* Screen = SpawnStaticMesh(
		Pos + ScreenOffset,
		FVector(1.2f, 0.6f, 0.03f),
		Rot + FRotator(20.f, 0.f, 0.f),
		CubeMesh, ScreenGlow);
	if (Screen)
	{
		UMaterialInterface* ScrEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* ScrMat = UMaterialInstanceDynamic::Create(ScrEmissiveMat, this);
		if (!ScrMat) { return; }
		ScrMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.1f, 0.5f, 1.0f));
		Screen->SetMaterial(0, ScrMat);
	}

	// Small light above the console
	UPointLightComponent* ConLight = NewObject<UPointLightComponent>(this);
	ConLight->SetupAttachment(RootComponent);
	ConLight->SetWorldLocation(Pos + FVector(0.f, 0.f, 120.f));
	ConLight->SetIntensity(1800.f);
	ConLight->SetAttenuationRadius(420.f);
	ConLight->SetLightColor(FLinearColor(0.45f, 1.1f, 1.8f));
	ConLight->CastShadows = false;
	ConLight->RegisterComponent();

	// Register screen for flicker animation
	if (Screen)
	{
		UMaterialInstanceDynamic* ScrMatDyn = Cast<UMaterialInstanceDynamic>(Screen->GetMaterial(0));
		if (AExoEnvironmentAnimator* Anim = AExoEnvironmentAnimator::Get(GetWorld()))
		{
			Anim->RegisterConsoleScreen(ScrMatDyn, ConLight);
		}
	}
}
