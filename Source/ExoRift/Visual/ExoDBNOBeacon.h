#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDBNOBeacon.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Pulsing vertical distress beacon above DBNO players — orange-yellow beam
 *  with a rotating diamond cap and pulsing light for teammate visibility. */
UCLASS()
class AExoDBNOBeacon : public AActor
{
	GENERATED_BODY()
public:
	AExoDBNOBeacon();
	virtual void Tick(float DeltaTime) override;
	static AExoDBNOBeacon* SpawnBeacon(UWorld* World, AActor* AttachTo);
	void DestroyBeacon();
private:
	UPROPERTY() UStaticMeshComponent* BeamMesh;
	UPROPERTY() UStaticMeshComponent* DiamondCap;
	UPROPERTY() UPointLightComponent* PulseLight;
	UPROPERTY() UMaterialInstanceDynamic* BeamMat = nullptr;
	UPROPERTY() UMaterialInstanceDynamic* CapMat = nullptr;
	float Age = 0.f;
	static constexpr float PulseFreq = 1.5f;
	static constexpr float BeamHeight = 400.f;
	static constexpr float BeamRadius = 0.06f;
	static constexpr float CapSpinRate = 45.f;
};
