#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/Interface.h"
#include "ExoInteractionComponent.generated.h"

class AExoCharacter;

// ---------------------------------------------------------------------------
// IExoInteractable — interface for objects that can be interacted with
// ---------------------------------------------------------------------------

UINTERFACE(MinimalAPI, Blueprintable)
class UExoInteractable : public UInterface
{
	GENERATED_BODY()
};

class EXORIFT_API IExoInteractable
{
	GENERATED_BODY()

public:
	virtual void Interact(AExoCharacter* Interactor) = 0;
	virtual FString GetInteractionPrompt() = 0;
};

// ---------------------------------------------------------------------------
// UExoInteractionComponent — traces forward to find interactables
// ---------------------------------------------------------------------------

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempt to interact with the current target. */
	void TryInteract();

	/** The interactable actor currently in front of the player. */
	AActor* GetCurrentInteractable() const { return CurrentInteractable.Get(); }

	/** Prompt text from the current interactable, empty if none. */
	FString GetCurrentPrompt() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TraceRange = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float TraceRadius = 30.f;

private:
	TWeakObjectPtr<AActor> CurrentInteractable;

	void UpdateTrace();
};
