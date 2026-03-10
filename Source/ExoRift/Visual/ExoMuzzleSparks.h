#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoMuzzleSparks.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Energy discharge sparks from the weapon muzzle when firing.
 * Creates small flying energy fragments that scatter from the barrel,
 * like electrical arcs breaking free from the energy weapon.
 */
UCLASS()
class AExoMuzzleSparks : public AActor
{
	GENERATED_BODY()

public:
	AExoMuzzleSparks();

	void InitSparks(const FRotator& FireDir, const FLinearColor& Color,
		EWeaponType WeaponType);

	virtual void Tick(float DeltaTime) override;

	static void SpawnSparks(UWorld* World, const FVector& Location,
		const FRotator& FireDir, const FLinearColor& Color, EWeaponType WeaponType);

private:
	static constexpr int32 NUM_MUZZLE_SPARKS = 6;

	UPROPERTY()
	TArray<UStaticMeshComponent*> SparkMeshes;

	UPROPERTY()
	UPointLightComponent* SparkLight;

	TArray<FVector> SparkVelocities;

	float Age = 0.f;
	float Lifetime = 0.15f;
	float BaseLightIntensity = 0.f;
};
