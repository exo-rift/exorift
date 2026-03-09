#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoWeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AExoWeaponBase;

UCLASS()
class EXORIFT_API AExoWeaponPickup : public AActor
{
	GENERATED_BODY()

public:
	AExoWeaponPickup();

	UPROPERTY(EditAnywhere, Category = "Pickup")
	EWeaponType WeaponType = EWeaponType::Rifle;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	bool bRespawns = false;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float RespawnTime = 30.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	void SpawnWeaponForPlayer(class AExoCharacter* Character);
	void SetPickupActive(bool bActive);

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
