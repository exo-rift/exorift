#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoCrashedCapitalShip.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Massive crashed capital ship hull — a dramatic map landmark.
 * Built from multiple sections: hull, wing, engine nacelles,
 * bridge tower, and scattered debris with glowing damage.
 */
UCLASS()
class AExoCrashedCapitalShip : public AActor
{
	GENERATED_BODY()

public:
	AExoCrashedCapitalShip();

	void BuildShip();

	virtual void Tick(float DeltaTime) override;

private:
	UStaticMeshComponent* AddHullSection(const FVector& Pos, const FVector& Scale,
		const FRotator& Rot, const FLinearColor& Color);
	UStaticMeshComponent* AddDamageGlow(const FVector& Pos, const FVector& Scale,
		const FLinearColor& Color);

	UPROPERTY()
	TArray<UStaticMeshComponent*> HullParts;

	UPROPERTY()
	TArray<UStaticMeshComponent*> DamageGlows;

	UPROPERTY()
	TArray<UPointLightComponent*> DamageLights;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DamageMats;

	TArray<FLinearColor> DamageBaseColors;

	UStaticMesh* CubeMesh = nullptr;
	UStaticMesh* CylinderMesh = nullptr;
	UStaticMesh* SphereMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
