#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/ExoTypes.h"
#include "ExoAILODManager.generated.h"

class AExoBotController;

UCLASS()
class EXORIFT_API UExoAILODManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	void RegisterBot(AExoBotController* Bot);
	void UnregisterBot(AExoBotController* Bot);

	int32 GetBotCountAtLOD(EAILODLevel Level) const;

protected:
	void UpdateAllLODLevels();

	UPROPERTY()
	TArray<TWeakObjectPtr<AExoBotController>> RegisteredBots;

	float UpdateInterval = 0.5f;
	float UpdateTimer = 0.f;

	// Stats
	int32 FullCount = 0;
	int32 SimplifiedCount = 0;
	int32 BasicCount = 0;
};
