#include "Core/ExoMatchmakingManager.h"
#include "Map/ExoMapConfig.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

UExoMatchmakingManager* UExoMatchmakingManager::Get(UWorld* World)
{
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	return GI->GetSubsystem<UExoMatchmakingManager>();
}

void UExoMatchmakingManager::StartSearching()
{
	if (State != EMatchmakingState::Idle) return;

	SearchTimer = 0.f;
	SearchDuration = FMath::RandRange(2.5f, 5.0f);
	PlayersFound = 0;
	SimulatedPing = FMath::RandRange(15, 65);

	// Random region
	const TCHAR* Regions[] = { TEXT("EU-West"), TEXT("EU-North"), TEXT("US-East"), TEXT("US-West"), TEXT("Asia-East") };
	Region = Regions[FMath::RandRange(0, 4)];

	TransitionTo(EMatchmakingState::Searching);
	UE_LOG(LogExoRift, Log, TEXT("Matchmaking: searching in %s (ping %dms)"), *Region, SimulatedPing);
}

void UExoMatchmakingManager::CancelSearch()
{
	if (State == EMatchmakingState::Searching)
	{
		TransitionTo(EMatchmakingState::Idle);
		UE_LOG(LogExoRift, Log, TEXT("Matchmaking: search cancelled"));
	}
}

void UExoMatchmakingManager::Tick(float DeltaTime)
{
	switch (State)
	{
	case EMatchmakingState::Searching:
	{
		SearchTimer += DeltaTime;

		// Gradually "find" players
		int32 TargetPlayers = FMath::Min(
			(int32)(SearchTimer / SearchDuration * 25.f) + FMath::RandRange(0, 3), 25);
		PlayersFound = FMath::Max(PlayersFound, TargetPlayers);

		if (SearchTimer >= SearchDuration)
		{
			PlayersFound = 25;
			TransitionTo(EMatchmakingState::Found);
			UE_LOG(LogExoRift, Log, TEXT("Matchmaking: match found! %d players, %dms ping"),
				PlayersFound, SimulatedPing);
		}
		break;
	}
	case EMatchmakingState::Found:
	{
		LoadTimer += DeltaTime;
		if (LoadTimer >= 1.5f) // Brief "match found" display
		{
			TransitionTo(EMatchmakingState::Loading);
			// Transition to gameplay map with explicit game mode override
			UGameplayStatics::OpenLevel(GetWorld(), *UExoMapConfig::GetBRMapPath(), true,
				TEXT("?game=/Script/ExoRift.ExoGameMode"));
		}
		break;
	}
	case EMatchmakingState::Loading:
	case EMatchmakingState::Idle:
		break;
	}
}

void UExoMatchmakingManager::TransitionTo(EMatchmakingState NewState)
{
	State = NewState;
	LoadTimer = 0.f;
	OnStateChanged.Broadcast(NewState);
}
