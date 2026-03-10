#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoRotatingProp.generated.h"

/**
 * Animated rotating prop — radar dishes, ventilation fans, energy coils.
 * Continuously rotates around a configurable axis with optional bobbing.
 */
UCLASS()
class AExoRotatingProp : public AActor
{
	GENERATED_BODY()

public:
	AExoRotatingProp();
	virtual void Tick(float DeltaTime) override;

	/** Init with a specific prop type: Fan, RadarDish, EnergyCoil. */
	void InitProp(int32 PropType, const FLinearColor& AccentColor,
		float RotSpeed = 60.f, float PropScale = 1.f);

private:
	UPROPERTY()
	UStaticMeshComponent* BaseMesh;

	UPROPERTY()
	UStaticMeshComponent* SpinningPart;

	UPROPERTY()
	class UPointLightComponent* AccentLight;

	float RotationSpeed = 60.f; // Degrees per second
	float CurrentAngle = 0.f;
	float BobAmplitude = 0.f;
	float BobFrequency = 0.f;
	int32 Type = 0; // 0=Fan, 1=Radar, 2=Coil
};
