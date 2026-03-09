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
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "ExoRift.h"

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
	CheckWinCondition();
}

void AExoGameMode::StartMatch()
{
	bMatchStarted = true;
	for (AController* PC : AlivePlayers)
		if (AExoCharacter* C = Cast<AExoCharacter>(PC->GetPawn())) SetWarmupInvulnerability(C, false);
	SpawnBots();
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

void AExoGameMode::SpawnBots()
{
	FActorSpawnParameters SpawnParams;
	for (int32 i = 0; i < TargetBotCount; i++)
	{
		AExoBotController* BC = GetWorld()->SpawnActor<AExoBotController>(AExoBotController::StaticClass(), SpawnParams);
		if (!BC) continue;
		FTransform ST = GetNextSpawnTransform();
		AExoBotCharacter* BP = GetWorld()->SpawnActor<AExoBotCharacter>(
			AExoBotCharacter::StaticClass(), ST.GetLocation(), ST.GetRotation().Rotator(), SpawnParams);
		if (!BP) continue;
		BC->Possess(BP);
		AlivePlayers.Add(BC);
		if (AExoPlayerState* PS = BC->GetPlayerState<AExoPlayerState>())
		{
			PS->DisplayName = FString::Printf(TEXT("Bot_%02d"), i);
			PS->bIsAlive = true;
		}
	}
	AliveCount = AlivePlayers.Num();
	TotalPlayers = AliveCount;
	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		GS->AliveCount = AliveCount;
		GS->TotalPlayers = TotalPlayers;
	}
}

FTransform AExoGameMode::GetNextSpawnTransform() const
{
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExoSpawnPoint::StaticClass(), SpawnPoints);
	if (SpawnPoints.Num() > 0)
		return SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)]->GetActorTransform();
	return FTransform(FVector(FMath::RandRange(-200000.f, 200000.f), FMath::RandRange(-200000.f, 200000.f), 1000.f));
}

void AExoGameMode::SetWarmupInvulnerability(AExoCharacter* Char, bool bEnable)
{
	if (Char) Char->SetCanBeDamaged(!bEnable);
}

void AExoGameMode::TickEndGame(float DeltaTime)
{
	EndGameTimer += DeltaTime;
	float Remaining = FMath::Max(EndGameDuration - EndGameTimer, 0.f);
	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		GS->EndGameTimeRemaining = Remaining;
	}
	if (EndGameTimer >= EndGameDuration) { RestartMatch(); }
}

void AExoGameMode::RestartMatch()
{
	UE_LOG(LogExoRift, Log, TEXT("Restarting match"));
	bMatchStarted = false;
	RemoveAllBots();
	AlivePlayers.Empty();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;
		AlivePlayers.AddUnique(PC);
		if (AExoPlayerState* PS = PC->GetPlayerState<AExoPlayerState>())
		{
			PS->ResetStats();
		}
		// Respawn if dead or spectating
		APawn* Pawn = PC->GetPawn();
		if (!Pawn || !Pawn->IsA(AExoCharacter::StaticClass()))
		{
			if (Pawn) { PC->UnPossess(); Pawn->Destroy(); }
			FTransform ST = GetNextSpawnTransform();
			AExoCharacter* NewChar = GetWorld()->SpawnActor<AExoCharacter>(
				AExoCharacter::StaticClass(), ST.GetLocation(), ST.GetRotation().Rotator());
			if (NewChar) { PC->Possess(NewChar); }
		}
	}

	AliveCount = AlivePlayers.Num();
	TotalPlayers = AliveCount;
	EndGameTimer = 0.f;
	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		GS->AliveCount = AliveCount;
		GS->TotalPlayers = TotalPlayers;
		GS->MatchElapsedTime = 0.f;
		GS->EndGameTimeRemaining = 0.f;
	}
	TransitionToPhase(EBRMatchPhase::WaitingForPlayers);
}

void AExoGameMode::ReturnToMainMenu()
{
	UE_LOG(LogExoRift, Log, TEXT("Returning to main menu"));
	UGameplayStatics::OpenLevel(GetWorld(), *UExoMapConfig::GetMenuMapPath());
}

void AExoGameMode::RemoveAllBots()
{
	TArray<AController*> BotsToRemove;
	for (AController* C : AlivePlayers)
	{
		if (C && C->IsA(AExoBotController::StaticClass())) { BotsToRemove.Add(C); }
	}
	for (AController* Bot : BotsToRemove)
	{
		AlivePlayers.Remove(Bot);
		if (APawn* P = Bot->GetPawn()) { Bot->UnPossess(); P->Destroy(); }
		Bot->Destroy();
	}
}
