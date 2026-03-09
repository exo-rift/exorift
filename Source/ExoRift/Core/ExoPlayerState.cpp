#include "Core/ExoPlayerState.h"
#include "Net/UnrealNetwork.h"

AExoPlayerState::AExoPlayerState()
{
	DisplayName = TEXT("Player");
}

void AExoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExoPlayerState, Kills);
	DOREPLIFETIME(AExoPlayerState, Placement);
	DOREPLIFETIME(AExoPlayerState, bIsAlive);
	DOREPLIFETIME(AExoPlayerState, DisplayName);
}
