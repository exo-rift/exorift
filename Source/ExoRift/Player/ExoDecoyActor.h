#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDecoyActor.generated.h"

class UAIPerceptionStimuliSourceComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Holographic decoy that draws bot aggro via AI perception stimuli.
 * Flickers and bobs at spawn location. Auto-destroys after lifespan.
 */
UCLASS()
class EXORIFT_API AExoDecoyActor : public AActor
{
	GENERATED_BODY()

public:
	AExoDecoyActor();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseDisk;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* HoloLight;

	UPROPERTY(VisibleAnywhere)
	UAIPerceptionStimuliSourceComponent* StimuliSource;

	UPROPERTY()
	UMaterialInstanceDynamic* HoloMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* BaseMat = nullptr;

	// Runtime detail parts
	UPROPERTY()
	TArray<UStaticMeshComponent*> DetailParts;

	UPROPERTY()
	UStaticMeshComponent* ScanLineMesh = nullptr;

	UPROPERTY()
	UPointLightComponent* ScanLight = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* ScanMat = nullptr;

private:
	float Age = 0.f;
	float FlickerSeed = 0.f;
	FVector SpawnLocation = FVector::ZeroVector;

	void UpdateHologram(float DeltaTime);
	void BuildDetailParts();
};
