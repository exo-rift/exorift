#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoSupplyDrop.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UENUM()
enum class ESupplyDropState : uint8
{
	Falling,
	Landed,
	Opened,
	Depleted
};

UCLASS()
class EXORIFT_API AExoSupplyDrop : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoSupplyDrop();

	virtual void Tick(float DeltaTime) override;

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

	/** Current state of the supply drop. */
	ESupplyDropState GetState() const { return CurrentState; }

protected:
	virtual void BeginPlay() override;

	void TickFalling(float DeltaTime);
	void SpawnLoot();
	void TransitionToState(ESupplyDropState NewState);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CrateMesh;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* BeaconLight;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float DropSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float BeaconRange = 30000.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float BeaconIntensity = 50000.f;

private:
	ESupplyDropState CurrentState = ESupplyDropState::Falling;
};
