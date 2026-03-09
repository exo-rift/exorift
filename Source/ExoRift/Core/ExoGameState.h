#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/ExoTypes.h"
#include "ExoGameState.generated.h"

UCLASS()
class EXORIFT_API AExoGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AExoGameState();

	void AddKillFeedEntry(const FKillFeedEntry& Entry);
	const TArray<FKillFeedEntry>& GetKillFeed() const { return KillFeed; }

	UPROPERTY(Replicated)
	int32 AliveCount = 0;

	UPROPERTY(Replicated)
	int32 TotalPlayers = 0;

	UPROPERTY(Replicated)
	EBRMatchPhase MatchPhase = EBRMatchPhase::WaitingForPlayers;

	UPROPERTY(Replicated)
	float MatchElapsedTime = 0.f;

	UPROPERTY(Replicated)
	int32 CurrentZoneStage = 0;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	static constexpr int32 MaxKillFeedEntries = 10;

	UPROPERTY()
	TArray<FKillFeedEntry> KillFeed;
};
