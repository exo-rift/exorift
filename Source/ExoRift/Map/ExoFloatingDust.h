#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoFloatingDust.generated.h"

/**
 * Spawns a cluster of small floating dust/debris motes around the player camera.
 * They drift slowly, catching light from the environment for ambient atmosphere.
 */
UCLASS()
class AExoFloatingDust : public AActor
{
	GENERATED_BODY()

public:
	AExoFloatingDust();

	virtual void Tick(float DeltaTime) override;

private:
	static constexpr int32 NUM_MOTES = 60;

	struct FMote
	{
		UStaticMeshComponent* Mesh = nullptr;
		FVector LocalOffset; // Offset from camera
		FVector Drift;       // Slow drift velocity
		float Phase;         // Animation phase
	};

	TArray<FMote> Motes;
	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
};
