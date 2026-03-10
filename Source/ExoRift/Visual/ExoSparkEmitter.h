#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoSparkEmitter.generated.h"

/**
 * Periodic electrical spark burst from a damaged panel or exposed wire.
 * Spawns small bright fragments that fly outward with gravity, plus a flash light.
 * Place at compounds for battle-damage atmosphere.
 */
UCLASS()
class AExoSparkEmitter : public AActor
{
	GENERATED_BODY()

public:
	AExoSparkEmitter();
	virtual void Tick(float DeltaTime) override;

	void InitSparks(const FLinearColor& Color, float BurstInterval = 3.f);

private:
	static constexpr int32 MAX_SPARKS = 8;

	struct FSpark
	{
		FVector Velocity = FVector::ZeroVector;
		float Age = -1.f;
	};

	UPROPERTY()
	TArray<UStaticMeshComponent*> SparkMeshes;

	UPROPERTY()
	class UPointLightComponent* FlashLight;

	TArray<FSpark> Sparks;
	FVector SparkOrigin = FVector::ZeroVector;
	FLinearColor SparkColor;

	float Interval = 3.f;
	float Timer = 0.f;
	float SparkLifetime = 0.4f;
	float FlashDecay = 0.f;

	UStaticMesh* CubeMesh = nullptr;
	UMaterialInterface* BaseMat = nullptr;
};
