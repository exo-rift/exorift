#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoJumpPad.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Launches pawns into the air on overlap. Placed at key locations
 * for fast vertical repositioning. Glows when ready, briefly dims
 * after use (1s cooldown).
 */
UCLASS()
class AExoJumpPad : public AActor
{
	GENERATED_BODY()

public:
	AExoJumpPad();
	virtual void Tick(float DeltaTime) override;

	/** Configure the pad's launch power and visual color. */
	void InitPad(float InLaunchSpeed, const FLinearColor& Color);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnPadOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BasePlatform;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GlowRing;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CenterLens;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere)
	UPointLightComponent* PadLight;

	UPROPERTY()
	UMaterialInstanceDynamic* RingMat;

	UPROPERTY()
	UMaterialInstanceDynamic* LensMat;

	float LaunchSpeed = 2500.f;
	float Cooldown = 0.f;
	float CooldownTime = 1.f;
	FLinearColor AccentColor = FLinearColor(0.1f, 0.8f, 1.f);
};
