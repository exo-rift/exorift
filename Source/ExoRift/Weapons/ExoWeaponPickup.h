#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoWeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AExoWeaponBase;
class AExoCharacter;

UCLASS()
class EXORIFT_API AExoWeaponPickup : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoWeaponPickup();

	UPROPERTY(EditAnywhere, Category = "Pickup")
	EWeaponType WeaponType = EWeaponType::Rifle;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	EWeaponRarity Rarity = EWeaponRarity::Common;

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

	void SpawnWeaponForPlayer(AExoCharacter* Character);
	void SetPickupActive(bool bActive);
	void BuildPickupModel();

	FString GetWeaponDisplayName() const;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DisplayMesh;

	bool bIsActive = true;
	float RespawnTimer = 0.f;

	// Floating bob animation
	float BobPhase = 0.f;
	FVector BaseLocation;
};
