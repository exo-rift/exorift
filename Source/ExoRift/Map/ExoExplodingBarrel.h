#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoExplodingBarrel.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

UCLASS()
class EXORIFT_API AExoExplodingBarrel : public AActor
{
	GENERATED_BODY()

public:
	AExoExplodingBarrel();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	void BuildBarrelVisuals();

	UPROPERTY(EditAnywhere, Category = "Barrel")
	float ExplosionDamage = 80.f;

	UPROPERTY(EditAnywhere, Category = "Barrel")
	float ExplosionRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Barrel")
	float Health = 50.f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BarrelMesh;

	UPROPERTY()
	UPointLightComponent* WarnLight = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* HazardStripeMat = nullptr;

private:
	void Explode(AController* InstigatorController);
	bool bHasExploded = false;
};
