#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoReactorCore.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Large animated energy reactor centerpiece.
 * Tall glowing structure with rotating rings, pulsing core,
 * and energy conduit beams. Visible from across the map.
 */
UCLASS()
class AExoReactorCore : public AActor
{
	GENERATED_BODY()

public:
	AExoReactorCore();

	virtual void Tick(float DeltaTime) override;

	void InitReactor();

private:
	UPROPERTY()
	UStaticMeshComponent* CoreSphere;

	UPROPERTY()
	UStaticMeshComponent* InnerRing;

	UPROPERTY()
	UStaticMeshComponent* OuterRing;

	UPROPERTY()
	UStaticMeshComponent* Pillar;

	UPROPERTY()
	UStaticMeshComponent* BasePlate;

	static constexpr int32 NUM_CONDUITS = 4;
	UPROPERTY()
	TArray<UStaticMeshComponent*> Conduits;

	UPROPERTY()
	UPointLightComponent* CoreLight;

	UPROPERTY()
	UPointLightComponent* AmbientLight;

	UPROPERTY()
	UMaterialInstanceDynamic* CoreMat;

	UPROPERTY()
	UMaterialInstanceDynamic* InnerRingMat;

	UPROPERTY()
	UMaterialInstanceDynamic* OuterRingMat;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> ConduitMats;

	UStaticMesh* SphereMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* CubeMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
