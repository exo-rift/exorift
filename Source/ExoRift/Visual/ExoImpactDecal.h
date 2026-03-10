#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoImpactDecal.generated.h"

class UStaticMeshComponent;

/**
 * Persistent scorch mark left at bullet impact locations.
 * Fades out after a few seconds.
 */
UCLASS()
class AExoImpactDecal : public AActor
{
	GENERATED_BODY()

public:
	AExoImpactDecal();

	virtual void Tick(float DeltaTime) override;

	void InitDecal(const FVector& HitNormal, const FLinearColor& Color);

	/** Spawn a decal at impact point. */
	static void SpawnDecal(UWorld* World, const FVector& Location,
		const FVector& HitNormal, const FLinearColor& WeaponColor);

private:
	UPROPERTY()
	UStaticMeshComponent* ScorchMesh;

	UPROPERTY()
	UStaticMeshComponent* GlowRing;

	float Age = 0.f;
	static constexpr float GlowDuration = 0.5f;
	static constexpr float FadeDuration = 4.f;
	static constexpr float TotalLifetime = 5.f;
};
