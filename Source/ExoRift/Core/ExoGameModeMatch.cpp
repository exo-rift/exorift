// ExoGameModeMatch.cpp — Match lifecycle: bots, spawning, restart, end-game
#include "Core/ExoGameMode.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Player/ExoCharacter.h"
#include "AI/ExoBotCharacter.h"
#include "AI/ExoBotController.h"
#include "Map/ExoSpawnPoint.h"
#include "Map/ExoMapConfig.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

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
	UGameplayStatics::OpenLevel(GetWorld(), *UExoMapConfig::GetMenuMapPath(), true,
		TEXT("?game=/Script/ExoRift.ExoMenuGameMode"));
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
