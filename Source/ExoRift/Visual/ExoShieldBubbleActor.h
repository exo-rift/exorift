#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShieldBubbleActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Protective energy dome with surface arcs for the ShieldBubble ability. */
UCLASS()
class AExoShieldBubbleActor : public AActor
{
	GENERATED_BODY()

public:
	AExoShieldBubbleActor();

	void Init();
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* DomeMesh;

	UPROPERTY()
	UStaticMeshComponent* InnerDome;

	UPROPERTY()
	UStaticMeshComponent* BaseRing;

	UPROPERTY()
	UStaticMeshComponent* TopCrown;

	UPROPERTY()
	UPointLightComponent* ShieldLight;

	UPROPERTY()
	UPointLightComponent* TopLight;

	static constexpr int32 NUM_ARCS = 6;

	UPROPERTY()
	UStaticMeshComponent* SurfaceArcs[NUM_ARCS];

	UPROPERTY()
	UMaterialInstanceDynamic* ArcMats[NUM_ARCS];

	UPROPERTY()
	UMaterialInstanceDynamic* DomeMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* InnerMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* RingMat = nullptr;

	float Age = 0.f;
	static constexpr float Lifetime = 1.2f;
};
