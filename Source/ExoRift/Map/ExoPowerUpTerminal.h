#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/ExoInteractionComponent.h"
#include "ExoPowerUpTerminal.generated.h"

class AExoCharacter;

UENUM()
enum class EPowerUpType : uint8
{
	SpeedBoost,     // +30% move speed for 15s
	DamageBoost,    // +25% damage for 15s
	ShieldRecharge, // Restore 50 shield
	OverheatReset   // Clear weapon heat + temp 50% heat reduction for 10s
};

/**
 * Interactable terminal that grants a temporary power-up.
 * One-time use with a long respawn timer.
 */
UCLASS()
class EXORIFT_API AExoPowerUpTerminal : public AActor, public IExoInteractable
{
	GENERATED_BODY()

public:
	AExoPowerUpTerminal();

	void InitTerminal(EPowerUpType Type);

	virtual void Interact(AExoCharacter* Interactor) override;
	virtual FString GetInteractionPrompt() override;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	void BuildVisuals();
	void ApplyPowerUp(AExoCharacter* Target);
	FString GetPowerUpName() const;

	UPROPERTY()
	UStaticMeshComponent* BaseMesh;

	UPROPERTY()
	UStaticMeshComponent* ScreenMesh;

	UPROPERTY()
	UStaticMeshComponent* PillarMesh;

	/** Real imported computer mesh (replaces primitives when available) */
	UPROPERTY()
	UStaticMeshComponent* ComputerMesh;

	UPROPERTY()
	class UPointLightComponent* TerminalLight;

	UPROPERTY()
	class UMaterialInstanceDynamic* ScreenMat;

	EPowerUpType PowerUp = EPowerUpType::SpeedBoost;
	FLinearColor TypeColor;
	bool bUsed = false;
	bool bHasRealMesh = false;
	float RespawnTimer = 0.f;
	float RespawnTime = 60.f;
};
