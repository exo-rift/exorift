#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExoMatchmakingManager.generated.h"

UENUM(BlueprintType)
enum class EMatchmakingState : uint8
{
	Idle,
	Searching,
	Found,
	Loading
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStateChanged, EMatchmakingState, NewState);

/**
 * Simulates matchmaking flow for the menu.
 * In a real game this would talk to a backend service.
 * For now it creates a convincing fake search → found → load transition.
 */
UCLASS()
class EXORIFT_API UExoMatchmakingManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void StartSearching();
	void CancelSearch();
	EMatchmakingState GetState() const { return State; }
	float GetSearchTime() const { return SearchTimer; }
	int32 GetPlayersFound() const { return PlayersFound; }
	FString GetRegion() const { return Region; }
	int32 GetPing() const { return SimulatedPing; }

	UPROPERTY()
	FOnMatchmakingStateChanged OnStateChanged;

	void Tick(float DeltaTime);

	static UExoMatchmakingManager* Get(UWorld* World);

private:
	void TransitionTo(EMatchmakingState NewState);

	EMatchmakingState State = EMatchmakingState::Idle;
	float SearchTimer = 0.f;
	float SearchDuration = 0.f; // Randomized fake search time
	float LoadTimer = 0.f;
	int32 PlayersFound = 0;
	int32 SimulatedPing = 0;
	FString Region = TEXT("EU-West");
};
