#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDecoyActor.generated.h"

class UAIPerceptionStimuliSourceComponent;

/**
 * Decoy actor that draws bot aggro via AI perception stimuli.
 * Stands still at spawn location and auto-destroys after its lifespan expires.
 */
UCLASS()
class EXORIFT_API AExoDecoyActor : public AActor
{
	GENERATED_BODY()

public:
	AExoDecoyActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Decoy")
	UStaticMeshComponent* DecoyMesh;

	/** Registers this actor as a sight stimulus so bot AI perception detects it. */
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionStimuliSourceComponent* StimuliSource;
};
