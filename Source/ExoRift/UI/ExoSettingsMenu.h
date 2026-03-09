#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

/**
 * Plain C++ settings menu overlay — no UObject, no .generated.h.
 * Drawn on top of the HUD when open. Toggled by ESC.
 */
class FExoSettingsMenu
{
public:
	static void Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font);

	static void ToggleMenu();
	static void NavigateUp();
	static void NavigateDown();
	static void AdjustValue(float Direction);

	static bool bIsOpen;
	static int32 SelectedOption;
};
