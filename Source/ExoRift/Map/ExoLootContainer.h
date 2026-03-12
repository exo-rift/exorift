#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "Core/ExoTypes.h"
#include "ExoLootContainer.generated.h"

class UStaticMeshComponent;
class AExoCharacter;

UENUM(BlueprintType)
enum class ELootContainerState : uint8
{
	Closed,
	Opening,
	Open,
	Empty
};

/**
 * Openable loot container scattered around the map.
 * Spawns 2-3 random pickups (weapons, energy, armor, grenades) when opened.
 */
UCLASS()
class EXORIFT_API AExoLootContainer : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoLootContainer();

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

	ELootContainerState GetState() const { return State; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void StartOpening(AExoCharacter* Interactor);
	void FinishOpening();
	void SpawnLoot();
	void BuildVisuals();
	FVector ComputeEjectVelocity() const;

	/** Rarity roll: 60% Common, 25% Rare, 10% Epic, 5% Legendary. */
	EWeaponRarity RollRarity() const;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ContainerMesh;

	// Runtime-built parts
	UPROPERTY()
	UStaticMeshComponent* LidComp = nullptr;
	UPROPERTY()
	class UPointLightComponent* ContainerGlow = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	int32 MinItems = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	int32 MaxItems = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	float OpenDuration = 1.f;

	ELootContainerState State = ELootContainerState::Closed;
	float OpenTimer = 0.f;

	UPROPERTY()
	AExoCharacter* PendingInteractor = nullptr;

	/** True if a real imported mesh was loaded (skip material override on body). */
	bool bHasRealMesh = false;
};
