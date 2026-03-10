#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoTracerWake.generated.h"

/**
 * Lingering energy droplets left behind a tracer as it travels.
 * Creates a dotted glowing trail that fades after a short time.
 */
UCLASS()
class AExoTracerWake : public AActor
{
	GENERATED_BODY()

public:
	AExoTracerWake();

	/** Spawn a chain of wake droplets along the given path. */
	static void SpawnWake(UWorld* World, const FVector& Start, const FVector& End,
		const FLinearColor& Color, float Spacing = 300.f);

	void InitWake(const FLinearColor& Color, float Scale);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* DotMesh;

	UPROPERTY()
	UStaticMeshComponent* HaloMesh;

	UPROPERTY()
	class UPointLightComponent* DotLight;

	UPROPERTY()
	class UMaterialInstanceDynamic* DotMat;

	UPROPERTY()
	class UMaterialInstanceDynamic* HaloMat;

	float Age = 0.f;
	float Lifetime = 0.7f;
	float BaseScale = 0.1f;
	FLinearColor BaseColor;
};
