#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoDeathBox.generated.h"

class AExoCharacter;

/**
 * Loot container dropped at a player's death location.
 * Contains their weapons and glows with a pulsing holographic effect.
 */
UCLASS()
class EXORIFT_API AExoDeathBox : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoDeathBox();

	/** Populate the box with weapons from the dead player. */
	void InitFromPlayer(const FString& PlayerName,
		const TArray<TPair<EWeaponType, EWeaponRarity>>& Weapons);

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void BuildVisuals();

	UPROPERTY()
	UStaticMeshComponent* BoxBody;

	UPROPERTY()
	UStaticMeshComponent* BoxLid;

	UPROPERTY()
	UStaticMeshComponent* HoloBeam;

	UPROPERTY()
	class UPointLightComponent* BoxLight;

	UPROPERTY()
	class UMaterialInstanceDynamic* BodyMat;

	UPROPERTY()
	class UMaterialInstanceDynamic* BeamMat;

	TArray<TPair<EWeaponType, EWeaponRarity>> StoredWeapons;
	FString DeadPlayerName;
	bool bLooted = false;
	float Age = 0.f;
	float DespawnTime = 120.f; // 2 minutes
};
