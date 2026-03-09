#include "Core/ExoCareerStats.h"
#include "Misc/ConfigCacheIni.h"
#include "ExoRift.h"

const TCHAR* UExoCareerStats::ConfigSection = TEXT("ExoRift.CareerStats");

UExoCareerStats* UExoCareerStats::Get(UWorld* World)
{
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UExoCareerStats>() : nullptr;
}

void UExoCareerStats::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadStats();
	UE_LOG(LogExoRift, Log, TEXT("Career stats loaded: %d matches, %d kills, %d wins"),
		TotalMatches, TotalKills, TotalWins);
}

void UExoCareerStats::Deinitialize()
{
	SaveStats();
	Super::Deinitialize();
}

void UExoCareerStats::RecordMatch(int32 Kills, int32 Deaths, int32 DamageDealt,
	int32 Placement, int32 TotalPlayers, float MatchDuration)
{
	TotalMatches++;
	TotalKills += Kills;
	TotalDeaths += Deaths;
	TotalDamageDealt += DamageDealt;
	TotalPlayTimeSeconds += MatchDuration;

	if (Placement == 1) TotalWins++;
	if (Placement <= 5) Top5Finishes++;
	if (Placement < BestPlacement) BestPlacement = Placement;
	if (Kills > MostKillsInMatch) MostKillsInMatch = Kills;

	UE_LOG(LogExoRift, Log, TEXT("Match recorded: #%d, %d kills, %d dmg (career: %d matches, %.1f%% WR)"),
		Placement, Kills, DamageDealt, TotalMatches, GetWinRate());

	SaveStats();
}

void UExoCareerStats::SaveStats()
{
	GConfig->SetInt(ConfigSection, TEXT("TotalMatches"), TotalMatches, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("TotalKills"), TotalKills, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("TotalDeaths"), TotalDeaths, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("TotalWins"), TotalWins, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("TotalDamageDealt"), TotalDamageDealt, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("BestPlacement"), BestPlacement, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("MostKillsInMatch"), MostKillsInMatch, GGameIni);
	GConfig->SetFloat(ConfigSection, TEXT("TotalPlayTime"), TotalPlayTimeSeconds, GGameIni);
	GConfig->SetInt(ConfigSection, TEXT("Top5Finishes"), Top5Finishes, GGameIni);
	GConfig->Flush(false, GGameIni);
}

void UExoCareerStats::LoadStats()
{
	GConfig->GetInt(ConfigSection, TEXT("TotalMatches"), TotalMatches, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("TotalKills"), TotalKills, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("TotalDeaths"), TotalDeaths, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("TotalWins"), TotalWins, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("TotalDamageDealt"), TotalDamageDealt, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("BestPlacement"), BestPlacement, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("MostKillsInMatch"), MostKillsInMatch, GGameIni);
	GConfig->GetFloat(ConfigSection, TEXT("TotalPlayTime"), TotalPlayTimeSeconds, GGameIni);
	GConfig->GetInt(ConfigSection, TEXT("Top5Finishes"), Top5Finishes, GGameIni);
}
