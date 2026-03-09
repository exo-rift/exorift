#include "Core/ExoGameMode.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoPlayerController.h"
#include "Player/ExoKillStreakComponent.h"
#include "AI/ExoBotCharacter.h"
#include "AI/ExoBotController.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoDropPodManager.h"
#include "Map/ExoSupplyDropManager.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoMapConfig.h"
#include "Core/ExoAudioManager.h"
#include "Core/ExoCareerStats.h"
#include "UI/ExoHUD.h"
#include "UI/ExoNotificationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "ExoRift.h"

static void PushNotification(UWorld* World, const FString& Msg, FLinearColor Color = FLinearColor::White)
{
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC) return;
	AExoHUD* HUD = Cast<AExoHUD>(PC->GetHUD());
	if (HUD) HUD->GetNotifications().AddNotification(Msg, Color, 5.f);
}

AExoGameMode::AExoGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = AExoCharacter::StaticClass();
	PlayerControllerClass = AExoPlayerController::StaticClass();
	GameStateClass = AExoGameState::StaticClass();
	PlayerStateClass = AExoPlayerState::StaticClass();
	HUDClass = AExoHUD::StaticClass();
}

void AExoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogExoRift, Log, TEXT("ExoRift BR initializing on map: %s"), *MapName);
}

void AExoGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Find zone system in level
	for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
	{
		ZoneSystem = *It;
		break;
	}

	// Spawn drop pod manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	DropPodManager = GetWorld()->SpawnActor<AExoDropPodManager>(AExoDropPodManager::StaticClass(), SpawnParams);

	// Spawn supply drop manager
	SupplyDropManager = GetWorld()->SpawnActor<AExoSupplyDropManager>(AExoSupplyDropManager::StaticClass(), SpawnParams);

	TransitionToPhase(EBRMatchPhase::WaitingForPlayers);
}

void AExoGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentPhase)
	{
	case EBRMatchPhase::WaitingForPlayers:
		TickWaiting(DeltaTime);
		break;
	case EBRMatchPhase::DropPhase:
		TickDropPhase(DeltaTime);
		break;
	case EBRMatchPhase::Playing:
	case EBRMatchPhase::ZoneShrinking:
		TickPlaying(DeltaTime);
		TickZoneDamage(DeltaTime);
		break;
	case EBRMatchPhase::EndGame:
		TickEndGame(DeltaTime);
		break;
	}
}

void AExoGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	if (!NewPlayer) return;

	AlivePlayers.AddUnique(NewPlayer);
	AliveCount = AlivePlayers.Num();
	TotalPlayers = AliveCount;
	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		GS->AliveCount = AliveCount;
		GS->TotalPlayers = TotalPlayers;
	}
	if (AExoPlayerState* PS = NewPlayer->GetPlayerState<AExoPlayerState>())
	{
		PS->bIsAlive = true;
		PS->DisplayName = FString::Printf(TEXT("Player_%d"), AlivePlayers.Num());
	}
	// During warmup, grant invulnerability
	if (!bMatchStarted)
	{
		if (AExoCharacter* Char = Cast<AExoCharacter>(NewPlayer->GetPawn()))
			SetWarmupInvulnerability(Char, true);
	}
}

void AExoGameMode::OnPlayerEliminated(AController* EliminatedPlayer, AController* Killer, const FString& WeaponName)
{
	AlivePlayers.Remove(EliminatedPlayer);
	AliveCount = AlivePlayers.Num();
	if (AExoPlayerState* PS = EliminatedPlayer->GetPlayerState<AExoPlayerState>())
	{
		PS->bIsAlive = false;
		PS->Placement = AliveCount + 1;
		PS->Deaths++;
	}
	if (Killer && Killer != EliminatedPlayer)
	{
		if (AExoPlayerState* KPS = Killer->GetPlayerState<AExoPlayerState>()) KPS->Kills++;
		if (AExoCharacter* KC = Cast<AExoCharacter>(Killer->GetPawn()))
			if (UExoKillStreakComponent* SC = KC->GetKillStreakComponent()) SC->RegisterKill();
	}
	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		FKillFeedEntry Entry;
		Entry.KillerName = Killer ? Killer->GetPlayerState<AExoPlayerState>()->DisplayName : TEXT("Zone");
		Entry.VictimName = EliminatedPlayer->GetPlayerState<AExoPlayerState>()->DisplayName;
		Entry.WeaponName = WeaponName;
		Entry.Timestamp = GetWorld()->GetTimeSeconds();
		GS->AddKillFeedEntry(Entry);
		GS->AliveCount = AliveCount;
	}
	// Milestone notifications
	if (AliveCount == 10 || AliveCount == 5 || AliveCount == 3 || AliveCount == 2)
	{
		PushNotification(GetWorld(),
			FString::Printf(TEXT("%d players remaining"), AliveCount),
			FLinearColor(1.f, 0.7f, 0.2f, 1.f));
	}

	CheckWinCondition();
}

void AExoGameMode::StartMatch()
{
	bMatchStarted = true;
	for (AController* PC : AlivePlayers)
		if (AExoCharacter* C = Cast<AExoCharacter>(PC->GetPawn())) SetWarmupInvulnerability(C, false);
	SpawnBots();
	PushNotification(GetWorld(),
		FString::Printf(TEXT("Match started — %d players"), AliveCount),
		FLinearColor(0.2f, 0.9f, 0.4f, 1.f));
	TransitionToPhase(EBRMatchPhase::DropPhase);
}

void AExoGameMode::TransitionToPhase(EBRMatchPhase NewPhase)
{
	CurrentPhase = NewPhase;
	PhaseTimer = 0.f;

	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		GS->MatchPhase = NewPhase;
	}

	UE_LOG(LogExoRift, Log, TEXT("Phase transition: %d"), static_cast<int32>(NewPhase));

	switch (NewPhase)
	{
	case EBRMatchPhase::DropPhase:
		if (DropPodManager) DropPodManager->StartDropPhase(AlivePlayers);
		break;
	case EBRMatchPhase::Playing:
		if (ZoneSystem) ZoneSystem->StartZoneSequence();
		break;
	case EBRMatchPhase::EndGame:
		{
			EndGameTimer = 0.f;
			if (AExoGameState* EndGS = GetGameState<AExoGameState>())
			{
				EndGS->EndGameTimeRemaining = EndGameDuration;
			}
			// Victory/defeat stinger
			APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
			bool bWon = LocalPC && AlivePlayers.Contains(LocalPC);
			if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
			{
				bWon ? Audio->PlayVictoryStinger() : Audio->PlayDefeatStinger();
			}
			// Record career stats for the local player
			if (LocalPC)
			{
				if (AExoPlayerState* PS = LocalPC->GetPlayerState<AExoPlayerState>())
				{
					if (UExoCareerStats* Career = UExoCareerStats::Get(GetWorld()))
					{
						AExoGameState* GS = GetGameState<AExoGameState>();
						Career->RecordMatch(PS->Kills, PS->Deaths, PS->DamageDealt,
							PS->Placement, TotalPlayers, GS ? GS->MatchElapsedTime : 0.f);
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void AExoGameMode::TickWaiting(float DeltaTime)
{
	AExoGameState* GS = GetGameState<AExoGameState>();
	if (GetNumPlayers() >= MinPlayersToStart)
	{
		PhaseTimer += DeltaTime;
		if (GS) GS->WaitingTimeRemaining = FMath::Max(WaitingDuration - PhaseTimer, 0.f);
		if (PhaseTimer >= WaitingDuration) StartMatch();
	}
	else
	{
		PhaseTimer = 0.f;
		if (GS) GS->WaitingTimeRemaining = 0.f;
	}
}

void AExoGameMode::TickDropPhase(float DeltaTime)
{
	PhaseTimer += DeltaTime;
	if (AExoGameState* GS = GetGameState<AExoGameState>())
		GS->DropPhaseTimeRemaining = FMath::Max(DropPhaseDuration - PhaseTimer, 0.f);
	if (PhaseTimer >= DropPhaseDuration) TransitionToPhase(EBRMatchPhase::Playing);
}

void AExoGameMode::TickPlaying(float DeltaTime)
{
	AExoGameState* GS = GetGameState<AExoGameState>();
	if (GS)
	{
		GS->MatchElapsedTime += DeltaTime;
	}

	// Push zone system state into replicated GameState every frame
	if (ZoneSystem && GS)
	{
		GS->CurrentZoneStage = ZoneSystem->GetCurrentStageIndex();
		GS->bZoneShrinking = ZoneSystem->IsShrinking();
		GS->ZoneHoldTimeRemaining = ZoneSystem->GetHoldTimeRemaining();
		GS->ZoneShrinkTimeRemaining = ZoneSystem->GetShrinkTimeRemaining();
	}

	// Track zone shrink state for phase transitions
	if (ZoneSystem && ZoneSystem->IsShrinking() && CurrentPhase != EBRMatchPhase::ZoneShrinking)
	{
		CurrentPhase = EBRMatchPhase::ZoneShrinking;
		if (GS) GS->MatchPhase = EBRMatchPhase::ZoneShrinking;
	}
	else if (ZoneSystem && !ZoneSystem->IsShrinking() && CurrentPhase == EBRMatchPhase::ZoneShrinking)
	{
		CurrentPhase = EBRMatchPhase::Playing;
		if (GS) GS->MatchPhase = EBRMatchPhase::Playing;
	}
}

void AExoGameMode::TickZoneDamage(float DeltaTime)
{
	if (!ZoneSystem) return;
	for (AController* PC : AlivePlayers)
	{
		if (!PC || !PC->GetPawn()) continue;
		if (!ZoneSystem->IsInsideZone(PC->GetPawn()->GetActorLocation()))
			PC->GetPawn()->TakeDamage(ZoneSystem->GetDamagePerSecond() * DeltaTime, FDamageEvent(), nullptr, ZoneSystem);
	}
}

void AExoGameMode::CheckWinCondition()
{
	if (AliveCount > 1 || CurrentPhase == EBRMatchPhase::WaitingForPlayers || CurrentPhase == EBRMatchPhase::EndGame)
		return;
	if (AlivePlayers.Num() > 0)
	{
		if (AExoPlayerState* WPS = AlivePlayers[0]->GetPlayerState<AExoPlayerState>())
		{
			WPS->Placement = 1;
			UE_LOG(LogExoRift, Log, TEXT("Winner: %s"), *WPS->DisplayName);
		}
	}
	TransitionToPhase(EBRMatchPhase::EndGame);
}

// SpawnBots, RestartMatch, ReturnToMainMenu, RemoveAllBots, TickEndGame,
// GetNextSpawnTransform, SetWarmupInvulnerability are in ExoGameModeMatch.cpp
