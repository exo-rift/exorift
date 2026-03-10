#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoAmbientParticles.generated.h"

class UStaticMeshComponent;

/**
 * Ambient floating particle cloud around the player.
 * Spawns small glowing motes that drift lazily — dust indoors, energy wisps outdoors.
 * Follows the local player for constant atmosphere without covering the whole map.
 */
UCLASS()
class AExoAmbientParticles : public AActor
{
	GENERATED_BODY()

public:
	AExoAmbientParticles();

	virtual void Tick(float DeltaTime) override;

	void SetStyle(bool bEnergyWisps);

	static AExoAmbientParticles* Get(UWorld* World);

private:
	static constexpr int32 NUM_MOTES = 30;

	struct FMote
	{
		FVector Offset = FVector::ZeroVector;
		FVector Drift = FVector::ZeroVector;
		float Phase = 0.f;
		float BaseScale = 0.02f;
	};

	UPROPERTY()
	TArray<UStaticMeshComponent*> MoteMeshes;

	TArray<FMote> Motes;

	float SpawnRadius = 2500.f;
	float DriftSpeed = 40.f;
	bool bIsEnergyStyle = false;
};
