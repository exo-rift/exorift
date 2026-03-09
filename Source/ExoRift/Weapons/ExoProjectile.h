#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class EXORIFT_API AExoProjectile : public AActor
{
	GENERATED_BODY()

public:
	AExoProjectile();

	void InitProjectile(const FVector& LaunchVelocity, float InDamage, AController* InInstigator);

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode();

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float MinDamageFalloff = 0.2f;

	float ProjectileDamage = 80.f;

	UPROPERTY()
	AController* InstigatorController = nullptr;
};
