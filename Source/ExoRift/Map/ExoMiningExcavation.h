#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoMiningExcavation.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Mining excavation site — a quarry-like depression with exposed mineral
 * veins, drilling equipment, and conveyors. Call BuildSite() after spawn.
 */
UCLASS()
class AExoMiningExcavation : public AActor
{
	GENERATED_BODY()

public:
	AExoMiningExcavation();

	void BuildSite();

	virtual void Tick(float DeltaTime) override;

private:
	UStaticMeshComponent* AddPart(const FVector& Pos, const FVector& Scale,
		const FRotator& Rot, UStaticMesh* Mesh, const FLinearColor& Color);

	UPROPERTY()
	TArray<UStaticMeshComponent*> SiteParts;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> MineralMats;

	UPROPERTY()
	TArray<UPointLightComponent*> MineralLights;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
