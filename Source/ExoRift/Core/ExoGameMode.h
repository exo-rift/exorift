#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/ExoTypes.h"
#include "ExoGameMode.generated.h"

class AExoZoneSystem;
class AExoDropPodManager;
class AExoSupplyDropManager;
class AExoCharacter;

UCLASS()
class EXORIFT_API AExoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AExoGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void OnPlayerEliminated(AController* EliminatedPlayer, AController* Killer, const FString& WeaponName);
	void StartMatch();
	int32 GetAlivePlayerCount() const { return AliveCount; }

protected:
	void TransitionToPhase(EBRMatchPhase NewPhase);
	void TickWaiting(float DeltaTime);
	void TickDropPhase(float DeltaTime);
	void TickPlaying(float DeltaTime);
	void TickZoneDamage(float DeltaTime);
	void CheckWinCondition();
	void SpawnBots();
	FTransform GetNextSpawnTransform() const;

	UPROPERTY(EditDefaultsOnly, Category = "BR")
	int32 MinPlayersToStart = 1;

	UPROPERTY(EditDefaultsOnly, Category = "BR")
	float WaitingDuration = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "BR")
	float DropPhaseDuration = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "BR")
	int32 TargetBotCount = 20;

	UPROPERTY(EditDefaultsOnly, Category = "BR")
	TSubclassOf<APawn> BotPawnClass;

private:
	EBRMatchPhase CurrentPhase = EBRMatchPhase::WaitingForPlayers;
	float PhaseTimer = 0.f;
	int32 AliveCount = 0;
	int32 TotalPlayers = 0;

	UPROPERTY()
	TArray<AController*> AlivePlayers;

	UPROPERTY()
	AExoZoneSystem* ZoneSystem = nullptr;

	UPROPERTY()
	AExoDropPodManager* DropPodManager = nullptr;

	UPROPERTY()
	AExoSupplyDropManager* SupplyDropManager = nullptr;
};
