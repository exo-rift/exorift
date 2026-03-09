#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoArmorComponent.h"
#include "ExoArmorPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AExoCharacter;

UCLASS()
class EXORIFT_API AExoArmorPickup : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoArmorPickup();

	UPROPERTY(EditAnywhere, Category = "Pickup")
	EArmorTier Tier = EArmorTier::Light;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bIsHelmet = false; // false = vest

	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bRespawns = false;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float RespawnTime = 30.f;

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void SetPickupActive(bool bActive);
	FLinearColor GetTierColor() const;
	FString GetDisplayName() const;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DisplayMesh;

	bool bIsActive = true;
	float RespawnTimer = 0.f;
	float BobPhase = 0.f;
	FVector BaseLocation;
};
