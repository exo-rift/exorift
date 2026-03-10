#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExoFPArms.generated.h"

/**
 * Procedural first-person arms built from basic shapes.
 * Attaches to the weapon view model to make it look held.
 * Simple forearm + hand geometry with glove and suit accents.
 */
UCLASS()
class UExoFPArms : public USceneComponent
{
	GENERATED_BODY()

public:
	UExoFPArms();

	/** Build arm geometry. Call once after registering. */
	void BuildArms(const FLinearColor& SuitColor);

private:
	UStaticMeshComponent* AddPart(const FVector& Offset, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator,
		UStaticMesh* Mesh = nullptr);

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
};
