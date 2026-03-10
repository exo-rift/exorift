#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShellCasing.generated.h"

class UStaticMeshComponent;

/**
 * Ejected shell casing with simple ballistic trajectory.
 * Spawned by weapon fire — flies out to the right, tumbles, fades.
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
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* CasingMesh;

	FVector Velocity = FVector::ZeroVector;
	FRotator TumbleRate = FRotator::ZeroRotator;
	float Age = 0.f;
	float Lifetime = 0.6f;
};
