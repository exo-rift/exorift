#pragma once

#include "CoreMinimal.h"

class UCanvas;
class UFont;
class AExoPlayerState;

/**
 * In-game scoreboard overlay drawn on Canvas.
 * Shows all players sorted by kills, with stats columns.
 * Toggled by holding TAB.
 */
class FExoScoreboard
{
public:
	void Draw(UCanvas* Canvas, UFont* Font, const TArray<AExoPlayerState*>& Players,
		float MatchTime, int32 AliveCount, int32 TotalPlayers);

private:
	void DrawHeader(UCanvas* Canvas, UFont* Font, float X, float Y, float Width);
	void DrawPlayerRow(UCanvas* Canvas, UFont* Font, AExoPlayerState* PS,
		float X, float Y, float Width, bool bIsLocalPlayer, int32 Rank);

	// Layout
	static constexpr float PanelWidthRatio = 0.6f;   // 60% of screen
	static constexpr float RowHeight = 28.f;
	static constexpr float HeaderHeight = 40.f;
	static constexpr float TitleHeight = 50.f;
	static constexpr float Padding = 12.f;
};
