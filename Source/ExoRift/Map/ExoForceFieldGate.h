#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoForceFieldGate.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Animated energy force field gate. Purely visual — players walk through.
 * Features shimmering barrier, frame pillars, and pulsing accent lights.
 */
UCLASS()
class AExoForceFieldGate : public AActor
{
	GENERATED_BODY()

public:
	AExoForceFieldGate();

	void InitGate(float Width, float Height, const FLinearColor& Color);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* LeftPillar;

	UPROPERTY()
	UStaticMeshComponent* RightPillar;

	UPROPERTY()
	UStaticMeshComponent* TopBeam;

	UPROPERTY()
	UStaticMeshComponent* BarrierMesh;

	UPROPERTY()
	UPointLightComponent* LeftLight;

	UPROPERTY()
	UPointLightComponent* RightLight;

	UPROPERTY()
	UMaterialInstanceDynamic* BarrierMat;

	FLinearColor GateColor;
	float GateWidth = 600.f;
	float GateHeight = 400.f;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
