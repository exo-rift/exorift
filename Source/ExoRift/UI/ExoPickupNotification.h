#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Brief center-screen notifications for weapon pickups and eliminations.
 * Fades in quickly, holds briefly, fades out.
 */
class FExoPickupNotification
{
public:
	/** Show a weapon pickup notification. */
	static void ShowWeaponPickup(const FString& WeaponName, const FLinearColor& RarityColor);

	/** Show an elimination notification (kill confirm). */
	static void ShowElimination(const FString& VictimName, bool bHeadshot);

	static void Tick(float DeltaTime);
	static void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

private:
	struct FNotifEntry
	{
		FString Text;
		FString SubText;
		FLinearColor Color;
		float Age = 0.f;
		float Lifetime = 2.5f;
		bool bIsElimination = false;
	};

	static TArray<FNotifEntry> Entries;
	static constexpr int32 MaxEntries = 3;
};
