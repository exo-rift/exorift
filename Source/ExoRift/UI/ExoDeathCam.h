#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Plain C++ class that draws the death-screen overlay on Canvas:
 * blood-red vignette, "ELIMINATED" text, kill info, placement,
 * combat stats summary, and a prompt to spectate.
 */
class FExoDeathCam
{
public:
	/** Initialize with kill details. Call once on death. */
	void Init(const FString& InKillerName, const FString& InWeapon,
		int32 InPlace, int32 InTotal);

	/** Advance fade and animation timers. */
	void Tick(float DeltaTime);

	/** Draw the full death overlay onto the HUD canvas. */
	void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

	bool IsActive() const { return bActive; }
	void Reset() { bActive = false; ElapsedTime = 0.f; }

	// Stats to display (set externally before calling Init)
	int32 CachedKills = 0;
	int32 CachedDamage = 0;
	int32 CachedAccuracy = 0;
	FString SurvivalTime = TEXT("0:00");

private:
	void DrawVignette(AHUD* HUD, UCanvas* Canvas);
	void DrawEliminatedText(AHUD* HUD, UCanvas* Canvas, UFont* Font);
	void DrawKillInfo(AHUD* HUD, UCanvas* Canvas, UFont* Font);
	void DrawPlacement(AHUD* HUD, UCanvas* Canvas, UFont* Font);
	void DrawStats(AHUD* HUD, UCanvas* Canvas, UFont* Font);
	void DrawSpectatePrompt(AHUD* HUD, UCanvas* Canvas, UFont* Font);

	bool bActive = false;
	float ElapsedTime = 0.f;

	FString KillerName;
	FString WeaponName;
	int32 Placement = 0;
	int32 TotalPlayers = 0;

	static constexpr float VignetteFadeIn = 0.5f;
	static constexpr float TextFadeDelay = 0.3f;
	static constexpr float StatsDelay = 1.0f;
	static constexpr float PromptDelay = 1.8f;
};
