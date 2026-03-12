#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoKillScorch.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Large persistent scorch mark spawned at kill locations.
 * Features expanding burn ring, residual embers, and slow fade.
 */
UCLASS()
class AExoKillScorch : public AActor
{
	GENERATED_BODY()

public:
	AExoKillScorch();
	virtual void Tick(float DeltaTime) override;

	void Init(bool bHeadshot);

	static void SpawnScorch(UWorld* World, const FVector& Location, bool bHeadshot);

private:
	UPROPERTY()
	UStaticMeshComponent* BurnMark;

	UPROPERTY()
	UStaticMeshComponent* OuterRing;

	UPROPERTY()
	UStaticMeshComponent* EmberCore;

	UPROPERTY()
	UPointLightComponent* ScorchLight;

	UPROPERTY()
	UMaterialInstanceDynamic* BurnMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* RingMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* EmberMat = nullptr;

	float Age = 0.f;
	bool bIsHeadshot = false;

	static constexpr float EmberDuration = 3.f;
	static constexpr float TotalLifetime = 30.f;
	static constexpr float FadeStart = 25.f;
};
