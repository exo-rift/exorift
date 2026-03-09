#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ExoMenuHUD.generated.h"

/** Current menu screen state. */
UENUM()
enum class EMenuState : uint8
{
	Main,
	Settings,
	Credits
};

/**
 * Full-screen Canvas HUD for the main menu.
 * Draws title, animated background, menu options, settings, and credits.
 */
UCLASS()
class EXORIFT_API AExoMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	AExoMenuHUD();

	virtual void DrawHUD() override;

	// Called by ExoMenuPlayerController
	void NavigateUp();
	void NavigateDown();
	void SelectCurrent();
	void GoBack();

protected:
	void DrawMainMenu();
	void DrawSettingsScreen();
	void DrawCreditsScreen();
	void DrawVersionText();
	void DrawBottomTips();

	// Helpers
	void DrawCenteredText(const FString& Text, float Y, float Scale,
		FLinearColor Color);
	void DrawMenuOption(const FString& Label, float X, float Y,
		float W, float H, bool bSelected, float Time);

private:
	void HandleMainSelect();

	EMenuState MenuState = EMenuState::Main;
	int32 SelectedIndex = 0;
	int32 MainOptionCount = 4;

	UPROPERTY()
	UFont* MenuFont;

	// Sci-fi color palette
	static const FLinearColor ColorCyan;
	static const FLinearColor ColorCyanBright;
	static const FLinearColor ColorGrey;
	static const FLinearColor ColorDarkBg;
	static const FLinearColor ColorWhite;
	static const FLinearColor ColorDimWhite;
};
