#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoGrenade.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

UCLASS()
class EXORIFT_API AExoGrenade : public AActor
{
	GENERATED_BODY()

public:
	AExoGrenade();

	virtual void Tick(float DeltaTime) override;

	/** Launch the grenade in the given direction. */
	void Ignite(FVector Direction);

	/** True if the fuse is lit and ticking. */
	bool IsIgnited() const { return bIgnited; }

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	EGrenadeType GrenadeType = EGrenadeType::Frag;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float FuseTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionDamage = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float ExplosionRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	float InitialSpeed = 2000.f;

protected:
	void BuildGrenadeVisuals();

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* FuseLight;

	UPROPERTY()
	UMaterialInstanceDynamic* BodyMat = nullptr;

	float FuseElapsed = 0.f;
	float NextBeepTime = 0.f;
	bool bIgnited = false;

private:
	void Explode();
	void ExplodeFrag();
	void ExplodeEMP();
	void ExplodeSmoke();

	FTimerHandle FuseTimerHandle;
};
