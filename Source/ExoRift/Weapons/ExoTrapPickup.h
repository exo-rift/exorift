#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoTrapPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AExoCharacter;

/**
 * Pickup actor that grants proximity mines to the player's trap component.
 */
UCLASS()
class EXORIFT_API AExoTrapPickup : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoTrapPickup();

	UPROPERTY(EditAnywhere, Category = "Pickup")
	int32 TrapCount = 1;

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

protected:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DisplayMesh;
};
