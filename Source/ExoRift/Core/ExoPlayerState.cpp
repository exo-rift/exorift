#include "Core/ExoPlayerState.h"
#include "Player/ExoPlayerCustomization.h"
#include "Net/UnrealNetwork.h"

AExoPlayerState::AExoPlayerState()
{
	DisplayName = TEXT("Player");
}

void AExoPlayerState::ResetStats()
{
	Kills = 0;
	Deaths = 0;
	Assists = 0;
	DamageDealt = 0;
	Placement = 0;
	bIsAlive = true;
	Revives = 0;
	HeadshotKills = 0;
	DamageTaken = 0.f;
	LongestKillDistance = 0.f;
	ShotsFired = 0;
	ShotsHit = 0;
}

void AExoPlayerState::InitDisplayNameFromCustomization(UWorld* World)
{
	if (UExoPlayerCustomization* Cust = UExoPlayerCustomization::Get(World))
	{
		if (!Cust->PlayerName.IsEmpty())
		{
			DisplayName = Cust->PlayerName;
		}
	}
}

void AExoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExoPlayerState, Kills);
	DOREPLIFETIME(AExoPlayerState, Deaths);
	DOREPLIFETIME(AExoPlayerState, Assists);
	DOREPLIFETIME(AExoPlayerState, DamageDealt);
	DOREPLIFETIME(AExoPlayerState, Placement);
	DOREPLIFETIME(AExoPlayerState, bIsAlive);
	DOREPLIFETIME(AExoPlayerState, DisplayName);
	DOREPLIFETIME(AExoPlayerState, Revives);
	DOREPLIFETIME(AExoPlayerState, HeadshotKills);
}
