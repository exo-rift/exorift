#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoScanPulseActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Expanding ring visual for the AreaScan ability. */
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
	UStaticMeshComponent* InnerFlash;

	UPROPERTY()
	UPointLightComponent* PulseLight;

	float Age = 0.f;
	float ScanRadius = 5000.f;
	static constexpr float Lifetime = 0.8f;
};
