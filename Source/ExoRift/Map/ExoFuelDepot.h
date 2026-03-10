#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoFuelDepot.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Industrial fuel/energy depot — storage tanks, pipes, and warning lights.
 * A medium-sized POI for loot and cover. Call BuildDepot() after spawn.
 */
UCLASS()
class AExoFuelDepot : public AActor
{
	GENERATED_BODY()

public:
	AExoFuelDepot();

	void BuildDepot();

	virtual void Tick(float DeltaTime) override;

private:
	UStaticMeshComponent* AddPart(const FVector& Pos, const FVector& Scale,
		const FRotator& Rot, UStaticMesh* Mesh, const FLinearColor& Color);

	UPROPERTY()
	TArray<UStaticMeshComponent*> DepotParts;

	UPROPERTY()
	TArray<UPointLightComponent*> WarnLights;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> TankGlowMats;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
