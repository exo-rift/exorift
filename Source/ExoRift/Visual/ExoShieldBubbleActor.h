#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShieldBubbleActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Brief protective dome visual for the ShieldBubble ability. */
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
	UStaticMeshComponent* BaseRing;

	UPROPERTY()
	UPointLightComponent* ShieldLight;

	UPROPERTY()
	UMaterialInstanceDynamic* DomeMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* RingMat = nullptr;

	float Age = 0.f;
	static constexpr float Lifetime = 1.2f;
};
