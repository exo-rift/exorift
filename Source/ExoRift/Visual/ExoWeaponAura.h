#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/ExoTypes.h"
#include "ExoWeaponAura.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Animated rarity aura on weapons — glowing particles that orbit the weapon.
 * Epic weapons get a subtle shimmer, Legendary weapons get full particle orbits.
 * Attach to weapon ViewModel. Call InitAura() once.
 */
UCLASS()
class UExoWeaponAura : public USceneComponent
{
	GENERATED_BODY()

public:
	UExoWeaponAura();

	void InitAura(EWeaponRarity InRarity, const FLinearColor& RarityColor);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	static constexpr int32 MAX_ORBS = 4;

	UPROPERTY()
	UStaticMeshComponent* Orbs[MAX_ORBS];

	UPROPERTY()
	UPointLightComponent* AuraLight;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> OrbMats;

	int32 ActiveOrbs = 0;
	FLinearColor AuraColor;
	float Phase = 0.f;

	UStaticMesh* SphereMesh = nullptr;
	UMaterialInterface* BaseMaterial = nullptr;
};
