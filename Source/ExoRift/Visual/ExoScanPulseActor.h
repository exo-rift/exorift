#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoScanPulseActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Expanding multi-ring scan pulse visual for the AreaScan ability. */
UCLASS()
class AExoScanPulseActor : public AActor
{
	GENERATED_BODY()

public:
	AExoScanPulseActor();

	void Init(float Radius);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* RingMesh;

	UPROPERTY()
	UStaticMeshComponent* SecondaryRing;

	UPROPERTY()
	UStaticMeshComponent* TertiaryRing;

	UPROPERTY()
	UStaticMeshComponent* InnerFlash;

	UPROPERTY()
	UStaticMeshComponent* GroundWave;

	UPROPERTY()
	UPointLightComponent* PulseLight;

	UPROPERTY()
	UPointLightComponent* TrailingLight;

	float Age = 0.f;
	float ScanRadius = 5000.f;
	static constexpr float Lifetime = 1.0f;
};
