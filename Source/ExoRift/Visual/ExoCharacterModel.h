#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExoCharacterModel.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Procedural third-person character model built from basic shapes.
 * Gives characters a visible sci-fi silhouette without real art assets.
 */
UCLASS()
class UExoCharacterModel : public USceneComponent
{
	GENERATED_BODY()

public:
	UExoCharacterModel();

	/** Build the character model. Call after registration. */
	void BuildModel(bool bIsBot);

	/** Set team/ID color accent. */
	void SetAccentColor(const FLinearColor& Color);

private:
	UStaticMeshComponent* AddPart(const FVector& Offset, const FVector& Scale,
		const FLinearColor& Color, UStaticMesh* Mesh = nullptr);

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;

	UPROPERTY()
	UStaticMeshComponent* AccentPart;

	UPROPERTY()
	UPointLightComponent* IdentityLight;
};
