#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoWaterPlane.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

/**
 * Simple water plane that creates ocean/lake surfaces.
 * Players entering the water volume take continuous damage
 * (simulating the island boundary or deep water).
 */
UCLASS()
class EXORIFT_API AExoWaterPlane : public AActor
{
	GENERATED_BODY()

public:
	AExoWaterPlane();

	virtual void Tick(float DeltaTime) override;

	/** Size of the water surface. */
	UPROPERTY(EditAnywhere, Category = "Water")
	float WaterExtent = 200000.f;

	/** Z height of water surface. */
	UPROPERTY(EditAnywhere, Category = "Water")
	float WaterLevel = -200.f;

	/** DPS when submerged. */
	UPROPERTY(EditAnywhere, Category = "Water")
	float DrowningDamage = 15.f;

	/** Whether water causes damage. */
	UPROPERTY(EditAnywhere, Category = "Water")
	bool bCausesDamage = true;

	float GetWaterLevel() const { return WaterLevel; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Water")
	UStaticMeshComponent* WaterMesh;

	UPROPERTY(VisibleAnywhere, Category = "Water")
	UBoxComponent* WaterVolume;

private:
	void ApplyDrowningDamage(float DeltaTime);
};
