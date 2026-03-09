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
#include "Map/ExoSpawnPoint.h"
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
		break;
	}
}

void AExoGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (NewPlayer)
	{
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
	}
}

void AExoGameMode::OnPlayerEliminated(AController* EliminatedPlayer, AController* Killer, const FString& WeaponName)
{
	AlivePlayers.Remove(EliminatedPlayer);
	AliveCount = AlivePlayers.Num();

	// Update player state
	if (AExoPlayerState* PS = EliminatedPlayer->GetPlayerState<AExoPlayerState>())
	{
		PS->bIsAlive = false;
		PS->Placement = AliveCount + 1;
	}

	// Credit killer
	if (Killer && Killer != EliminatedPlayer)
	{
		if (AExoPlayerState* KillerPS = Killer->GetPlayerState<AExoPlayerState>())
		{
			KillerPS->Kills++;
		}

		// Register kill for streak tracking
		if (AExoCharacter* KillerChar = Cast<AExoCharacter>(Killer->GetPawn()))
		{
			if (UExoKillStreakComponent* StreakComp = KillerChar->GetKillStreakComponent())
			{
				StreakComp->RegisterKill();
			}
		}
	}

	// Kill feed
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
		if (DropPodManager)
		{
			DropPodManager->StartDropPhase(AlivePlayers);
		}
		break;
	case EBRMatchPhase::Playing:
		if (ZoneSystem)
		{
			ZoneSystem->StartZoneSequence();
		}
		break;
	default:
		break;
	}
}

void AExoGameMode::TickWaiting(float DeltaTime)
{
	int32 PlayerCount = GetNumPlayers();
	if (PlayerCount >= MinPlayersToStart)
	{
		PhaseTimer += DeltaTime;
		if (PhaseTimer >= WaitingDuration)
		{
			StartMatch();
		}
	}
}

void AExoGameMode::TickDropPhase(float DeltaTime)
{
	PhaseTimer += DeltaTime;
	if (PhaseTimer >= DropPhaseDuration)
	{
		TransitionToPhase(EBRMatchPhase::Playing);
	}
}

void AExoGameMode::TickPlaying(float DeltaTime)
{
	if (AExoGameState* GS = GetGameState<AExoGameState>())
	{
		GS->MatchElapsedTime += DeltaTime;
	}

	// Check if zone is shrinking and update phase accordingly
	if (ZoneSystem && ZoneSystem->IsShrinking() && CurrentPhase != EBRMatchPhase::ZoneShrinking)
	{
		CurrentPhase = EBRMatchPhase::ZoneShrinking;
		if (AExoGameState* GS = GetGameState<AExoGameState>())
		{
			GS->MatchPhase = EBRMatchPhase::ZoneShrinking;
		}
	}
	else if (ZoneSystem && !ZoneSystem->IsShrinking() && CurrentPhase == EBRMatchPhase::ZoneShrinking)
	{
		CurrentPhase = EBRMatchPhase::Playing;
		if (AExoGameState* GS = GetGameState<AExoGameState>())
		{
			GS->MatchPhase = EBRMatchPhase::Playing;
		}
	}
}

void AExoGameMode::TickZoneDamage(float DeltaTime)
{
	if (!ZoneSystem) return;

	for (AController* PC : AlivePlayers)
	{
		if (!PC || !PC->GetPawn()) continue;

		FVector PawnLoc = PC->GetPawn()->GetActorLocation();
		if (!ZoneSystem->IsInsideZone(PawnLoc))
		{
			float Damage = ZoneSystem->GetDamagePerSecond() * DeltaTime;
			PC->GetPawn()->TakeDamage(Damage, FDamageEvent(), nullptr, ZoneSystem);
		}
	}
}

void AExoGameMode::CheckWinCondition()
{
	if (AliveCount <= 1 && CurrentPhase != EBRMatchPhase::WaitingForPlayers && CurrentPhase != EBRMatchPhase::EndGame)
	{
		if (AlivePlayers.Num() > 0)
		{
			if (AExoPlayerState* WinnerPS = AlivePlayers[0]->GetPlayerState<AExoPlayerState>())
			{
				WinnerPS->Placement = 1;
				UE_LOG(LogExoRift, Log, TEXT("Winner: %s"), *WinnerPS->DisplayName);
			}
		}
		TransitionToPhase(EBRMatchPhase::EndGame);
	}
}

void AExoGameMode::SpawnBots()
{
	for (int32 i = 0; i < TargetBotCount; i++)
	{
		FActorSpawnParameters SpawnParams;
		AExoBotController* BotController = GetWorld()->SpawnActor<AExoBotController>(AExoBotController::StaticClass(), SpawnParams);
		if (BotController)
		{
			FTransform SpawnTransform = GetNextSpawnTransform();
			AExoBotCharacter* BotPawn = GetWorld()->SpawnActor<AExoBotCharacter>(
				AExoBotCharacter::StaticClass(), SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), SpawnParams);
			if (BotPawn)
			{
				BotController->Possess(BotPawn);
				AlivePlayers.Add(BotController);

				if (AExoPlayerState* PS = BotController->GetPlayerState<AExoPlayerState>())
				{
					PS->DisplayName = FString::Printf(TEXT("Bot_%02d"), i);
					PS->bIsAlive = true;
				}
			}
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
	// Find spawn points in the level
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExoSpawnPoint::StaticClass(), SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);
		return SpawnPoints[Index]->GetActorTransform();
	}

	// Fallback: random position in a 4km area
	float X = FMath::RandRange(-200000.f, 200000.f);
	float Y = FMath::RandRange(-200000.f, 200000.f);
	return FTransform(FVector(X, Y, 1000.f));
}
