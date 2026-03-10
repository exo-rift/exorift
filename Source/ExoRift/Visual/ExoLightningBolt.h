#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoLightningBolt.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Procedural lightning bolt from sky to ground.
 * Spawned by the weather system during storms.
 * Generates a jagged path with branches and emissive glow.
 */
UCLASS()
class AExoLightningBolt : public AActor
{
	GENERATED_BODY()

public:
	AExoLightningBolt();

	void InitBolt(const FVector& StrikePos, float BoltHeight = 15000.f);

	virtual void Tick(float DeltaTime) override;

private:
	void BuildBoltSegments(const FVector& Start, const FVector& End, int32 Depth);

	static constexpr int32 MAX_SEGMENTS = 24;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Segments;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> SegmentMats;

	UPROPERTY()
	UPointLightComponent* StrikeLight;

	UPROPERTY()
	UPointLightComponent* SkyLight;

	float Age = 0.f;
	float Lifetime = 0.4f;
	float BaseIntensity = 500000.f;
	FLinearColor BoltColor = FLinearColor(0.7f, 0.8f, 1.f);

	UStaticMesh* CylinderMesh = nullptr;
};
