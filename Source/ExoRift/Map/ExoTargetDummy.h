#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoTargetDummy.generated.h"

/**
 * Destructible target dummy for pre-match warmup / firing range.
 * Shows floating damage numbers when shot, respawns after a short delay.
 */
UCLASS()
class EXORIFT_API AExoTargetDummy : public AActor
{
	GENERATED_BODY()

public:
	AExoTargetDummy();

	virtual float TakeDamage(float Damage, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Tick(float DeltaTime) override;

	void InitDummy(const FLinearColor& Color, float InHealth = 200.f);

private:
	void Shatter();
	void Respawn();
	void BuildVisuals();

	UPROPERTY()
	UStaticMeshComponent* TorsoMesh;

	UPROPERTY()
	UStaticMeshComponent* HeadMesh;

	UPROPERTY()
	UStaticMeshComponent* BasePlateMesh;

	UPROPERTY()
	class UPointLightComponent* GlowLight;

	UPROPERTY()
	class UMaterialInstanceDynamic* TorsoMat;

	UPROPERTY()
	class UMaterialInstanceDynamic* HeadMat;

	float MaxHealth = 200.f;
	float CurrentHealth = 200.f;
	bool bAlive = true;
	float RespawnTimer = 0.f;
	float HitFlashTimer = 0.f;
	FLinearColor AccentColor;
};
