#include "AI/ExoAILODManager.h"
#include "AI/ExoBotController.h"
#include "AI/ExoBotCharacter.h"
#include "ExoRift.h"

void UExoAILODManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTimer += DeltaTime;
	if (UpdateTimer < UpdateInterval) return;
	UpdateTimer = 0.f;

	UpdateAllLODLevels();
}

TStatId UExoAILODManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UExoAILODManager, STATGROUP_Tickables);
}

void UExoAILODManager::RegisterBot(AExoBotController* Bot)
{
	if (Bot)
	{
		RegisteredBots.AddUnique(Bot);
	}
}

void UExoAILODManager::UnregisterBot(AExoBotController* Bot)
{
	RegisteredBots.Remove(Bot);
}

int32 UExoAILODManager::GetBotCountAtLOD(EAILODLevel Level) const
{
	switch (Level)
	{
	case EAILODLevel::Full: return FullCount;
	case EAILODLevel::Simplified: return SimplifiedCount;
	case EAILODLevel::Basic: return BasicCount;
	}
	return 0;
}

void UExoAILODManager::UpdateAllLODLevels()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Gather all player camera locations
	TArray<FVector> PlayerLocations;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			PlayerLocations.Add(PC->GetPawn()->GetActorLocation());
		}
	}

	if (PlayerLocations.Num() == 0) return;

	FullCount = 0;
	SimplifiedCount = 0;
	BasicCount = 0;

	// Clean up stale entries and update LODs in one pass
	for (int32 i = RegisteredBots.Num() - 1; i >= 0; i--)
	{
		AExoBotController* Bot = RegisteredBots[i].Get();
		if (!Bot || !Bot->GetPawn())
		{
			RegisteredBots.RemoveAt(i);
			continue;
		}

		FVector BotLoc = Bot->GetPawn()->GetActorLocation();

		// Find minimum distance to any player
		float MinDistSq = MAX_FLT;
		for (const FVector& PlayerLoc : PlayerLocations)
		{
			float DistSq = FVector::DistSquared(BotLoc, PlayerLoc);
			MinDistSq = FMath::Min(MinDistSq, DistSq);
		}

		// Determine LOD level (distances in UE units: 100 UU = 1m)
		EAILODLevel NewLOD;
		if (MinDistSq < 10000.f * 10000.f)        // <100m
		{
			NewLOD = EAILODLevel::Full;
			FullCount++;
		}
		else if (MinDistSq < 50000.f * 50000.f)   // 100-500m
		{
			NewLOD = EAILODLevel::Simplified;
			SimplifiedCount++;
		}
		else
		{
			NewLOD = EAILODLevel::Basic;
			BasicCount++;
		}

		AExoBotCharacter* BotChar = Cast<AExoBotCharacter>(Bot->GetPawn());
		if (BotChar && BotChar->GetAILODLevel() != NewLOD)
		{
			BotChar->SetAILODLevel(NewLOD);
		}
	}
}
