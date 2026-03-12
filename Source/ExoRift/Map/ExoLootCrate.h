#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoLootCrate.generated.h"

class AExoCharacter;

/**
 * Physical loot container that can be opened by players.
 * Glows and hums until opened, then pops out random weapon pickups.
 */
UCLASS()
class EXORIFT_API AExoLootCrate : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoLootCrate();

	UPROPERTY(EditAnywhere, Category = "Crate")
	int32 ItemCount = 2;

	// IExoInteractable
	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void SpawnContents();
	void BuildVisuals();

	UPROPERTY()
	UStaticMeshComponent* CrateBody;

	UPROPERTY()
	UStaticMeshComponent* CrateLid;

	UPROPERTY()
	UStaticMeshComponent* GlowStrip1;

	UPROPERTY()
	UStaticMeshComponent* GlowStrip2;

	UPROPERTY()
	class UPointLightComponent* CrateLight;

	UPROPERTY()
	class UMaterialInstanceDynamic* StripMat1;

	UPROPERTY()
	class UMaterialInstanceDynamic* StripMat2;

	UPROPERTY()
	class UMaterialInstanceDynamic* LidMat;

	bool bOpened = false;
	float OpenTimer = 0.f;
	float BobPhase = 0.f;

	FLinearColor CrateColor;

	/** True when CrateBody uses an imported mesh (skip dynamic material override) */
	bool bUsingRealMesh = false;
};
