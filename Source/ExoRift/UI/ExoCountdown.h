#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Plain C++ helper for pre-match countdown HUD drawing.
 * No UObject, no .generated.h — just static draw functions called from ExoHUD.
 */
class EXORIFT_API FExoCountdown
{
public:
	/**
	 * Draw the pre-match countdown overlay during WaitingForPlayers phase.
	 * Shows countdown number, player count, and "waiting for players" when not enough.
	 */
	static void DrawPreMatchCountdown(AHUD* HUD, UCanvas* Canvas, UFont* Font,
		float TimeRemaining, int32 PlayerCount, int32 MinPlayers);

	/**
	 * Draw the drop-phase HUD showing deployment countdown and
	 * "Choose your landing zone" subtitle.
	 */
	static void DrawDropPhaseHUD(AHUD* HUD, UCanvas* Canvas, UFont* Font,
		float TimeRemaining);

	/**
	 * Draw a brief "GO!" banner that appears for ~2 seconds when Playing phase starts.
	 * Scales up and fades out over the duration.
	 */
	static void DrawMatchStartBanner(AHUD* HUD, UCanvas* Canvas, UFont* Font,
		float MatchTime);

private:
	// Duration (seconds) that the "GO!" banner is visible
	static constexpr float GoBannerDuration = 2.f;
};
