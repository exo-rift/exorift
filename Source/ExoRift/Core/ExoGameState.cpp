#include "Core/ExoGameState.h"
#include "Net/UnrealNetwork.h"

AExoGameState::AExoGameState()
{
}

void AExoGameState::AddKillFeedEntry(const FKillFeedEntry& Entry)
{
	KillFeed.Insert(Entry, 0);
	if (KillFeed.Num() > MaxKillFeedEntries)
	{
		KillFeed.SetNum(MaxKillFeedEntries);
	}
}

void AExoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExoGameState, AliveCount);
	DOREPLIFETIME(AExoGameState, TotalPlayers);
	DOREPLIFETIME(AExoGameState, MatchPhase);
	DOREPLIFETIME(AExoGameState, MatchElapsedTime);
	DOREPLIFETIME(AExoGameState, CurrentZoneStage);
	DOREPLIFETIME(AExoGameState, ZoneHoldTimeRemaining);
	DOREPLIFETIME(AExoGameState, ZoneShrinkTimeRemaining);
	DOREPLIFETIME(AExoGameState, bZoneShrinking);
	DOREPLIFETIME(AExoGameState, WaitingTimeRemaining);
	DOREPLIFETIME(AExoGameState, DropPhaseTimeRemaining);
	DOREPLIFETIME(AExoGameState, EndGameTimeRemaining);
}
