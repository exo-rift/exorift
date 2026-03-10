#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoPatrolDrone.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Ambient flying patrol drone — purely visual, follows waypoint circuit.
 * Adds atmosphere and life to the map without gameplay impact.
 */
UCLASS()
class AExoPatrolDrone : public AActor
{
	GENERATED_BODY()

public:
	AExoPatrolDrone();
	virtual void Tick(float DeltaTime) override;

	void InitDrone(const TArray<FVector>& InWaypoints, const FLinearColor& Color,
		float InSpeed = 800.f, float InHoverHeight = 600.f);

private:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* BodyMesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* RotorL;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* RotorR;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* DroneLight;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* ScanLight;

	TArray<FVector> Waypoints;
	int32 CurrentWaypoint = 0;
	float Speed = 800.f;
	float HoverHeight = 600.f;
	float RotorAngle = 0.f;
	float BobPhase = 0.f;
};
