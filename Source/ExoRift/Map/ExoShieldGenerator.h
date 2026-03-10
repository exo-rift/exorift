#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShieldGenerator.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Deployable shield generator — creates a temporary energy dome that
 * blocks incoming projectiles. Has limited health and self-destructs.
 * Placed at strategic map positions for tactical gameplay.
 */
UCLASS()
class AExoShieldGenerator : public AActor
{
	GENERATED_BODY()

public:
	AExoShieldGenerator();
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float Damage, const FDamageEvent& Event,
		AController* EventInstigator, AActor* Causer) override;

	void InitGenerator(float InShieldRadius, const FLinearColor& Color);

protected:
	virtual void BeginPlay() override;

private:
	void DestroyShield();

	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* BaseMesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* PylonMesh;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* ShieldDome;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* CoreLight;

	UPROPERTY() UMaterialInstanceDynamic* DomeMat;

	float ShieldHealth = 500.f;
	float MaxShieldHealth = 500.f;
	float ShieldRadius = 800.f;
	FLinearColor ShieldColor = FLinearColor(0.1f, 0.6f, 1.f);
	bool bShieldActive = true;
};
