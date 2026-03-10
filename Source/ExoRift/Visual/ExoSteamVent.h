#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoSteamVent.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Periodic steam/gas vent that bursts upward.
 * Placed near pipes and industrial areas for atmosphere.
 */
UCLASS()
class AExoSteamVent : public AActor
{
	GENERATED_BODY()

public:
	AExoSteamVent();

	virtual void Tick(float DeltaTime) override;

	/** Configure vent timing and color. */
	void InitVent(float InInterval, float InDuration, const FLinearColor& InColor);

private:
	static constexpr int32 NUM_PUFFS = 5;

	UPROPERTY()
	TArray<UStaticMeshComponent*> PuffMeshes;

	UPROPERTY()
	UPointLightComponent* VentLight;

	float Interval = 5.f;
	float BurstDuration = 0.8f;
	float Timer = 0.f;
	float BurstAge = 0.f;
	bool bIsBursting = false;
	FLinearColor VentColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.f);
};
