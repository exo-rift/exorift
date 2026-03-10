#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/ExoTypes.h"
#include "ExoWeaponViewModel.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UExoFPArms;

/**
 * Procedural first-person weapon model built from basic shapes.
 * Attach to a weapon actor to give it a visible model.
 */
UCLASS()
class UExoWeaponViewModel : public USceneComponent
{
	GENERATED_BODY()

public:
	UExoWeaponViewModel();

	/** Build the model geometry for the given weapon type. Call once after creation. */
	void BuildModel(EWeaponType Type, const FLinearColor& RarityColor);

	/** Get the muzzle tip location for tracer/flash spawning. */
	FVector GetMuzzleLocation() const;

private:
	void BuildRifleModel(const FLinearColor& Accent);
	void BuildPistolModel(const FLinearColor& Accent);
	void BuildSMGModel(const FLinearColor& Accent);
	void BuildShotgunModel(const FLinearColor& Accent);
	void BuildSniperModel(const FLinearColor& Accent);
	void BuildLauncherModel(const FLinearColor& Accent);
	void BuildMeleeModel(const FLinearColor& Accent);

	UStaticMeshComponent* AddPart(const FVector& Offset, const FVector& Scale,
		const FLinearColor& Color, UStaticMesh* Mesh = nullptr);

	UPROPERTY()
	UStaticMeshComponent* MuzzleTip;

	UPROPERTY()
	UPointLightComponent* MuzzleReadyLight;

	UPROPERTY()
	UExoFPArms* Arms = nullptr;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
