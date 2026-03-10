#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoAutoSlidingDoor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

/**
 * Automatic sliding door — opens when a pawn enters the trigger volume,
 * closes when all pawns leave. Used in buildings for immersion.
 */
UCLASS()
class EXORIFT_API AExoAutoSlidingDoor : public AActor
{
	GENERATED_BODY()

public:
	AExoAutoSlidingDoor();

	virtual void Tick(float DeltaTime) override;

	/** Configure door size, direction, and color. */
	void InitDoor(float Width, float Height, float SlideDistance,
		const FLinearColor& AccentColor);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorPanelL;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorPanelR;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorFrame;
	UPROPERTY(VisibleAnywhere)
	USphereComponent* TriggerVolume;
	UPROPERTY()
	UPointLightComponent* DoorLight;
	UPROPERTY()
	UMaterialInstanceDynamic* AccentMat;

	float DoorWidth = 200.f;
	float DoorHeight = 300.f;
	float SlideAmount = 100.f;
	int32 OverlapCount = 0;
	float OpenBlend = 0.f;
	float OpenSpeed = 5.f;
	FVector ClosedOffsetL;
	FVector ClosedOffsetR;
};
