#pragma once

#include "CoreMinimal.h"

class UCanvas;
class UFont;

/**
 * Tracks rapid kills and announces multi-kill events + first blood.
 * Draws centered announcement text with dramatic scaling animation.
 */
class FExoKillAnnouncer
{
public:
	/** Call when the local player scores a kill. */
	static void RegisterKill();

	/** Call once at match start to reset state. */
	static void Reset();

	void Tick(float DeltaTime);
	void Draw(UCanvas* Canvas, UFont* Font);

private:
	struct FAnnouncement
	{
		FString Text;
		FLinearColor Color;
		float Duration;
		float ElapsedTime = 0.f;
	};

	void ShowAnnouncement(const FString& Text, const FLinearColor& Color, float Duration);

	TArray<FAnnouncement> ActiveAnnouncements;

	// Static state for multi-kill tracking
	static int32 TotalMatchKills;
	static int32 RapidKillCount;
	static float RapidKillTimer;
	static bool bFirstBloodClaimed;
	static constexpr float RapidKillWindow = 4.f;
};
