#pragma once

#include "CoreMinimal.h"

struct FMatchSummaryEntry
{
	FString PlayerName;
	int32 Kills = 0;
	int32 Placement = 0;
	bool bIsLocalPlayer = false;
};

// Static helper drawn from HUD during EndGame phase
class EXORIFT_API FExoMatchSummary
{
public:
	static void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

private:
	static void DrawBackground(AHUD* HUD, UCanvas* Canvas);
	static void DrawTitle(AHUD* HUD, UCanvas* Canvas, UFont* Font, int32 LocalPlacement);
	static void DrawLeaderboard(AHUD* HUD, UCanvas* Canvas, UFont* Font,
		const TArray<FMatchSummaryEntry>& Entries);
	static void DrawPersonalStats(AHUD* HUD, UCanvas* Canvas, UFont* Font,
		const FMatchSummaryEntry& LocalEntry);
};
