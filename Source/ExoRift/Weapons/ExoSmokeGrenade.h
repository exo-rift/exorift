#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoSmokeGrenade.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Standalone smoke grenade. Thrown with arc trajectory (2s fuse),
 * deploys expanding cloud of 18 emissive spheres (~800cm radius, 8s).
 */
UCLASS()
class EXORIFT_API AExoSmokeGrenade : public AActor
{
	GENERATED_BODY()

public:
	AExoSmokeGrenade();
	virtual void Tick(float DeltaTime) override;

	void Throw(const FVector& ThrowVelocity);

	/** Convenience spawner — spawns, throws, and returns the grenade. */
	static AExoSmokeGrenade* SpawnSmoke(UWorld* World,
		const FVector& Location, const FVector& ThrowVelocity);

	UPROPERTY(EditDefaultsOnly, Category = "Smoke")
	float FuseTime = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smoke")
	float SmokeDuration = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smoke")
	float CloudRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smoke")
	int32 NumSmokePuffs = 18;

	UPROPERTY(EditDefaultsOnly, Category = "Smoke")
	float InitialSpeed = 1800.f;

protected:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* CloudLight;

private:
	void Detonate();
	void SpawnCloudPuffs();
	void UpdateCloud(float DeltaTime);
	void FadeOutCloud(float DeltaTime);

	struct FSmokePuff
	{
		UStaticMeshComponent* Mesh = nullptr;
		UMaterialInstanceDynamic* Mat = nullptr;
		FVector TargetOffset;
		float TargetScale;
		float Delay;
	};

	TArray<FSmokePuff> Puffs;
	FVector CloudCenter = FVector::ZeroVector;
	float FuseElapsed = 0.f;
	float CloudElapsed = 0.f;
	bool bFuseLit = false;
	bool bCloudActive = false;
	bool bFadingOut = false;
	static constexpr float FadeOutTime = 2.f;
};
