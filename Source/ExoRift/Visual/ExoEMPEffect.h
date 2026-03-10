#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoEMPEffect.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Blue-white expanding EMP pulse with electrical arc fragments. */
UCLASS()
class AExoEMPEffect : public AActor
{
	GENERATED_BODY()

public:
	AExoEMPEffect();

	void InitPulse(float Radius);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* CoreSphere;

	UPROPERTY()
	UStaticMeshComponent* PulseRing;

	UPROPERTY()
	UStaticMeshComponent* PulseRing2;

	UPROPERTY()
	UPointLightComponent* PulseLight;

	UPROPERTY()
	TArray<UStaticMeshComponent*> ArcFragments;

	TArray<FVector> ArcDirections;

	float Age = 0.f;
	float Lifetime = 0.7f;
	float PulseRadius = 500.f;
};
