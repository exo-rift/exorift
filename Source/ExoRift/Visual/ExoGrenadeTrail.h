#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoGrenadeTrail.generated.h"

/**
 * Attaches to a grenade actor and spawns fading trail spheres
 * along the flight path for visual tracking.
 */
UCLASS()
class AExoGrenadeTrail : public AActor
{
	GENERATED_BODY()

public:
	AExoGrenadeTrail();

	void InitTrail(AActor* Parent, EGrenadeType Type);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> TrackedActor;

	FLinearColor TrailColor;
	float SpawnInterval = 0.04f;
	float SpawnTimer = 0.f;
	float Age = 0.f;

	// Trail dots — ring buffer of spawned spheres
	static constexpr int32 MAX_DOTS = 30;
	UPROPERTY()
	TArray<UStaticMeshComponent*> Dots;
	TArray<float> DotAges;
	int32 NextDotIndex = 0;

	UPROPERTY()
	class UMaterialInstanceDynamic* DotMatTemplate;

	UStaticMesh* SphereMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
