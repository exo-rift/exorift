#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoPickupFlash.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Brief visual flash when a pickup is collected.
 * Spawned at collection point, fades out and self-destructs.
 */
UCLASS()
class EXORIFT_API AExoPickupFlash : public AActor
{
	GENERATED_BODY()

public:
	AExoPickupFlash();

	virtual void Tick(float DeltaTime) override;

	/** Spawn a pickup flash at the given location with a color. */
	static void SpawnAt(UWorld* World, const FVector& Location, const FLinearColor& Color);

	void Init(const FLinearColor& Color);

private:
	UPROPERTY()
	UPointLightComponent* FlashLight = nullptr;

	UPROPERTY()
	UStaticMeshComponent* FlashSphere = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* FlashMat = nullptr;

	float Age = 0.f;
	float Lifetime = 0.25f;
	float BaseIntensity = 20000.f;
};
