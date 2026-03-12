#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoSupplyDropManager.generated.h"

class AExoSupplyDrop;
class AExoZoneSystem;

UCLASS()
class EXORIFT_API AExoSupplyDropManager : public AActor
{
	GENERATED_BODY()

public:
	AExoSupplyDropManager();

	virtual void Tick(float DeltaTime) override;

	/** Get all active (non-depleted) supply drops for HUD indicators. */
	const TArray<AExoSupplyDrop*>& GetActiveDrops() const { return ActiveDrops; }

protected:
	void SpawnDrop();
	void CleanupDepletedDrops();

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float DropInterval = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	int32 MaxActiveDrops = 3;

	UPROPERTY(EditDefaultsOnly, Category = "SupplyDrop")
	float DropAltitude = 30000.f;

private:
	UPROPERTY()
	TArray<AExoSupplyDrop*> ActiveDrops;

	float DropTimer = 0.f;
	bool bIsActive = false;
};
