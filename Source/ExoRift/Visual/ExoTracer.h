#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoTracer.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/** Fast-fading beam actor for hitscan weapon tracers. */
UCLASS()
class AExoTracer : public AActor
{
	GENERATED_BODY()

public:
	AExoTracer();

	/** Set up the tracer between two points. Call immediately after spawn. */
	void InitTracer(const FVector& Start, const FVector& End, bool bIsHit);

	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* BeamMesh;

	UPROPERTY()
	UPointLightComponent* GlowLight;

	UPROPERTY()
	UMaterialInstanceDynamic* DynMat;

	float Age = 0.f;
	float Lifetime = 0.12f;
	FLinearColor BaseColor;
	float BaseIntensity = 0.f;
};
