#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoMeleeSlash.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Plasma blade slash VFX — arc sweep + afterglow trail on melee swing.
 */
UCLASS()
class AExoMeleeSlash : public AActor
{
	GENERATED_BODY()

public:
	AExoMeleeSlash();
	virtual void Tick(float DeltaTime) override;

	void InitSlash(const FVector& Direction);

	static void SpawnSlash(UWorld* World, const FVector& Origin,
		const FVector& Direction);

private:
	static constexpr int32 ARC_SEGMENTS = 6;

	UPROPERTY()
	TArray<UStaticMeshComponent*> ArcSegments;

	UPROPERTY()
	UStaticMeshComponent* CoreFlash = nullptr;

	UPROPERTY()
	UPointLightComponent* SlashLight = nullptr;

	float Age = 0.f;
	float Lifetime = 0.35f;
	FVector SlashDir = FVector::ForwardVector;
};
