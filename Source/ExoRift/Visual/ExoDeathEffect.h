#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDeathEffect.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/** Energy burst + scattering fragments when a character is eliminated. */
UCLASS()
class AExoDeathEffect : public AActor
{
	GENERATED_BODY()

public:
	AExoDeathEffect();

	void Init(const FLinearColor& AccentColor);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* CoreFlash;

	UPROPERTY()
	UStaticMeshComponent* ShockRing;

	UPROPERTY()
	UPointLightComponent* BurstLight;

	UPROPERTY()
	TArray<UStaticMeshComponent*> Fragments;

	TArray<FVector> FragVelocities;

	float Age = 0.f;
	static constexpr float Lifetime = 0.6f;
	static constexpr int32 NumFragments = 10;
};
