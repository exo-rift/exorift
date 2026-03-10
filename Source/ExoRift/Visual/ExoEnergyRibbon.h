#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoEnergyRibbon.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Persistent energy ribbon that lingers after a tracer passes.
 * Creates a glowing trail of segments that fade over 1-2 seconds,
 * leaving visible energy contrails across the battlefield.
 */
UCLASS()
class AExoEnergyRibbon : public AActor
{
	GENERATED_BODY()

public:
	AExoEnergyRibbon();

	void InitRibbon(const FVector& Start, const FVector& End,
		const FLinearColor& Color, float Thickness = 1.f);

	virtual void Tick(float DeltaTime) override;

	static void SpawnRibbon(UWorld* World, const FVector& Start, const FVector& End,
		const FLinearColor& Color, float Thickness = 1.f);

private:
	static constexpr int32 NUM_SEGMENTS = 12;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Segments;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> SegmentMats;

	TArray<FVector> SegmentDrifts;

	FLinearColor RibbonColor;
	float RibbonThickness = 1.f;
	float Age = 0.f;
	float Lifetime = 1.6f;

	UStaticMesh* CylinderMesh = nullptr;
};
