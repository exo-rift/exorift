#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoGuardTower.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Climbable guard tower — provides vantage points for sniping.
 * Has a ramp/stairway, observation deck, searchlight.
 * Call BuildTower() after spawn.
 */
UCLASS()
class AExoGuardTower : public AActor
{
	GENERATED_BODY()

public:
	AExoGuardTower();

	void BuildTower();

	virtual void Tick(float DeltaTime) override;

private:
	UStaticMeshComponent* AddPart(const FVector& Pos, const FVector& Scale,
		const FRotator& Rot, UStaticMesh* Mesh, const FLinearColor& Color);

	UPROPERTY()
	TArray<UStaticMeshComponent*> Parts;

	UPROPERTY()
	UPointLightComponent* SearchLight = nullptr;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;

	float SearchAngle = 0.f;
};
