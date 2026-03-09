#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;
class UWorld;

/**
 * Radial communication wheel with 8 quick-comm options.
 * Plain C++ — no UObject, no .generated.h.
 * Drawn on the HUD canvas when open.
 */
class EXORIFT_API FExoCommsWheel
{
public:
	static constexpr int32 NumOptions = 8;

	/** Open the comms wheel — captures mouse delta. */
	static void Open();

	/** Close the wheel and send the selected comm. */
	static void Close(UWorld* World);

	/** Feed mouse delta while the wheel is open. */
	static void UpdateMouse(FVector2D Delta);

	/** Draw the radial wheel overlay. */
	static void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

	/** Send a comm ping at the player's location. */
	static void SendComm(int32 Index, UWorld* World);

	/** Return the display text for the given option index. */
	static FString GetCommText(int32 Index);

	static bool bIsOpen;
	static int32 HighlightedOption;
	static FVector2D MouseDelta;

private:
	static constexpr float WheelRadius = 150.f;
	static constexpr float InnerDeadzone = 25.f;
	static constexpr float BackgroundAlpha = 0.75f;
};
