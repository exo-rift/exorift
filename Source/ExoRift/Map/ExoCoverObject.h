#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoCoverObject.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ECoverType : uint8
{
	Crate,
	Jersey_Barrier,
	Wall_Half,
	Wall_Full,
	Rocks,
	Vehicle_Wreck,
	Sandbags
};

/**
 * Environment cover object that provides protection during firefights.
 * Uses engine primitives to create various cover shapes.
 */
UCLASS()
class EXORIFT_API AExoCoverObject : public AActor
{
	GENERATED_BODY()

public:
	AExoCoverObject();

	/** Type of cover to generate. */
	UPROPERTY(EditAnywhere, Category = "Cover")
	ECoverType CoverType = ECoverType::Crate;

	/** Scale multiplier. */
	UPROPERTY(EditAnywhere, Category = "Cover")
	float ScaleMultiplier = 1.f;

	/** Build the cover mesh. */
	void GenerateCover();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Cover")
	UStaticMeshComponent* CoverMesh;

	UPROPERTY(VisibleAnywhere, Category = "Cover")
	UStaticMeshComponent* CoverMesh2;

private:
	void SetupCrate();
	void SetupBarrier();
	void SetupHalfWall();
	void SetupFullWall();
	void SetupRocks();
	void SetupVehicleWreck();
	void SetupSandbags();
};
