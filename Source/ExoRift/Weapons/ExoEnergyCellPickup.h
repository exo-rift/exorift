#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoEnergyCellPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class AExoCharacter;

UCLASS()
class EXORIFT_API AExoEnergyCellPickup : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoEnergyCellPickup();

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float EnergyAmount = 50.f;

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
	void BuildCellModel();

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DisplayMesh;

	UPROPERTY()
	UPointLightComponent* GlowLight;

	UPROPERTY()
	UStaticMesh* CachedCylinder;

	UPROPERTY()
	UStaticMesh* CachedSphere;

	bool bIsActive = true;
	float RespawnTimer = 0.f;
	float BobPhase = 0.f;
	FVector BaseLocation;
};
