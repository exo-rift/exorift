#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ExoWorldSetup.generated.h"

/**
 * Auto-spawns all required BR systems when the world starts.
 * This means you can hit Play on ANY map and the BR systems work.
 */
UCLASS()
class EXORIFT_API UExoWorldSetup : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void EnsureActorExists(UWorld* World, UClass* ActorClass, const FVector& Location = FVector::ZeroVector);
};
