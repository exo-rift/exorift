#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

UCLASS()
class EXORIFT_API AExoProjectile : public AActor
{
	GENERATED_BODY()

public:
	AExoProjectile();

	virtual void Tick(float DeltaTime) override;

	void InitProjectile(const FVector& LaunchVelocity, float InDamage, AController* InInstigator);

	/** Set the projectile's glow color (weapon-dependent). */
	void SetProjectileColor(const FLinearColor& Color);

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

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* GlowLight;

	UPROPERTY()
	UMaterialInstanceDynamic* ProjectileMat = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float MinDamageFalloff = 0.2f;

	float ProjectileDamage = 80.f;
	FLinearColor GlowColor = FLinearColor(1.f, 0.4f, 0.1f);

	UPROPERTY()
	AController* InstigatorController = nullptr;
};
