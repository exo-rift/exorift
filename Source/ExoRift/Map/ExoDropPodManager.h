#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDropPodManager.generated.h"

class AExoDropPod;

UCLASS()
class EXORIFT_API AExoDropPodManager : public AActor
{
	GENERATED_BODY()

public:
	AExoDropPodManager();

	void StartDropPhase(const TArray<AController*>& Players);
	void OnPodLanded(AExoDropPod* Pod);
	int32 GetLandedCount() const { return LandedCount; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropAltitude = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropSpreadRadius = 25000.f;

	UPROPERTY()
	TArray<AExoDropPod*> ActivePods;

	int32 TotalPods = 0;
	int32 LandedCount = 0;
};
