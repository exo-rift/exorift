#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExoCareerStats.generated.h"

/**
 * Persistent career statistics saved to local config.
 * Tracks lifetime performance across all matches.
 */
UCLASS()
class EXORIFT_API UExoCareerStats : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Record end-of-match stats
	void RecordMatch(int32 Kills, int32 Deaths, int32 DamageDealt,
		int32 Placement, int32 TotalPlayers, float MatchDuration);

	// Getters
	int32 GetTotalMatches() const { return TotalMatches; }
	int32 GetTotalKills() const { return TotalKills; }
	int32 GetTotalDeaths() const { return TotalDeaths; }
	int32 GetTotalWins() const { return TotalWins; }
	float GetKDRatio() const { return TotalDeaths > 0 ? (float)TotalKills / TotalDeaths : (float)TotalKills; }
	float GetWinRate() const { return TotalMatches > 0 ? (float)TotalWins / TotalMatches * 100.f : 0.f; }
	int32 GetBestPlacement() const { return BestPlacement; }
	int32 GetMostKillsInMatch() const { return MostKillsInMatch; }
	float GetTotalPlayTime() const { return TotalPlayTimeSeconds; }
	int32 GetTotalDamageDealt() const { return TotalDamageDealt; }
	float GetAverageKillsPerMatch() const { return TotalMatches > 0 ? (float)TotalKills / TotalMatches : 0.f; }
	int32 GetTop5Count() const { return Top5Finishes; }

	static UExoCareerStats* Get(UWorld* World);

private:
	void SaveStats();
	void LoadStats();

	int32 TotalMatches = 0;
	int32 TotalKills = 0;
	int32 TotalDeaths = 0;
	int32 TotalWins = 0;
	int32 TotalDamageDealt = 0;
	int32 BestPlacement = 999;
	int32 MostKillsInMatch = 0;
	float TotalPlayTimeSeconds = 0.f;
	int32 Top5Finishes = 0;

	static const TCHAR* ConfigSection;
};
