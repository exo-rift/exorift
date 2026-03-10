#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoRelayTower.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Tall communication relay tower — visible landmark across the map.
 * Multi-section steel lattice with rotating dish, blinking lights,
 * and energy conduit glow. Call BuildTower() after spawn.
 */
UCLASS()
class AExoRelayTower : public AActor
{
	GENERATED_BODY()

public:
	AExoRelayTower();

	void BuildTower();

	virtual void Tick(float DeltaTime) override;

private:
	UStaticMeshComponent* AddSection(const FVector& Pos, const FVector& Scale,
		const FRotator& Rot, UStaticMesh* Mesh, const FLinearColor& Color);

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	UPROPERTY()
	TArray<UPointLightComponent*> BeaconLights;

	UPROPERTY()
	UStaticMeshComponent* DishArm = nullptr;

	UPROPERTY()
	UStaticMeshComponent* DishHead = nullptr;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;

	float DishAngle = 0.f;
};
