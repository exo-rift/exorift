#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShellCasing.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Ejected energy shell casing with ballistic trajectory.
 * Spawned by weapon fire — flies out, tumbles, glows, fades.
 */
UCLASS()
class AExoShellCasing : public AActor
{
	GENERATED_BODY()

public:
	AExoShellCasing();
	virtual void Tick(float DeltaTime) override;

	void InitCasing(const FVector& EjectDirection, const FLinearColor& Color);

private:
	UPROPERTY()
	UStaticMeshComponent* CasingMesh;

	UPROPERTY()
	UStaticMeshComponent* HotTip;

	UPROPERTY()
	UPointLightComponent* CasingLight;

	FVector Velocity = FVector::ZeroVector;
	FRotator TumbleRate = FRotator::ZeroRotator;
	float Age = 0.f;
	float Lifetime = 0.7f;
	FLinearColor TipColor;
};
