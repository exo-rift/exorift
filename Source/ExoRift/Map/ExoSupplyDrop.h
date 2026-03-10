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

	ESupplyDropState GetState() const { return CurrentState; }

protected:
	virtual void BeginPlay() override;

	void TickFalling(float DeltaTime);
	void SpawnLoot();
	void TransitionToState(ESupplyDropState NewState);
	void BuildCrateMesh();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CrateMesh;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* BeaconLight;

	// Procedural parts
	UPROPERTY()
	UStaticMeshComponent* CrateBody;
	UPROPERTY()
	UStaticMeshComponent* CrateLid;
	UPROPERTY()
	UStaticMeshComponent* ParachuteDome;
	UPROPERTY()
	UStaticMeshComponent* ParachuteStrut1;
	UPROPERTY()
	UStaticMeshComponent* ParachuteStrut2;
	UPROPERTY()
	UPointLightComponent* CrateGlow;

	// Descent trail and landing effects
	UPROPERTY()
	UStaticMeshComponent* SmokeTrail = nullptr;

	UPROPERTY()
	UPointLightComponent* TrailLight = nullptr;

	UPROPERTY()
	UStaticMeshComponent* ImpactRing = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* ImpactRingMat = nullptr;

	float ImpactRingTimer = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float DropSpeed = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float SwayAmplitude = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float BeaconRange = 30000.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float BeaconIntensity = 50000.f;

private:
	ESupplyDropState CurrentState = ESupplyDropState::Falling;
	float SwayTimer = 0.f;
	float LidOpenAlpha = 0.f;

	// Cached meshes
	UStaticMesh* CubeMeshRef = nullptr;
	UStaticMesh* SphereMeshRef = nullptr;
	UStaticMesh* CylinderMeshRef = nullptr;
	UMaterialInterface* BaseMaterialRef = nullptr;
};
