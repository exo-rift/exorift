// ExoMenuHUD.cpp — Core menu: draw dispatch, navigation, input handling
// Sub-screens and overlays in ExoMenuHUDScreens.cpp
#include "UI/ExoMenuHUD.h"
#include "UI/ExoMenuBackground.h"
#include "UI/ExoSettingsMenu.h"
#include "Core/ExoMatchmakingManager.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ExoRift.h"

// ---------------------------------------------------------------------------
// Sci-fi colour palette
// ---------------------------------------------------------------------------

const FLinearColor AExoMenuHUD::ColorCyan(0.0f, 0.85f, 1.0f, 1.0f);
const FLinearColor AExoMenuHUD::ColorCyanBright(0.4f, 0.95f, 1.0f, 1.0f);
const FLinearColor AExoMenuHUD::ColorGrey(0.55f, 0.55f, 0.6f, 1.0f);
const FLinearColor AExoMenuHUD::ColorDarkBg(0.02f, 0.02f, 0.04f, 0.92f);
const FLinearColor AExoMenuHUD::ColorWhite(0.92f, 0.94f, 0.97f, 1.0f);
const FLinearColor AExoMenuHUD::ColorDimWhite(0.5f, 0.52f, 0.56f, 0.8f);

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

AExoMenuHUD::AExoMenuHUD()
{
	static ConstructorHelpers::FObjectFinder<UFont> FontFinder(
		TEXT("/Engine/EngineFonts/Roboto"));
	if (FontFinder.Succeeded())
	{
		MenuFont = FontFinder.Object;
	}
}

// ---------------------------------------------------------------------------
// Main draw loop
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !MenuFont) return;

	float Time = GetWorld()->GetTimeSeconds();

	// Tick matchmaking
	TickMatchmaking(GetWorld()->GetDeltaSeconds());

	// Background visuals
	FExoMenuBackground::DrawBackground(this, Canvas, Time);
	FExoMenuBackground::DrawScanLines(this, Canvas, Time);
	FExoMenuBackground::DrawTitle(this, Canvas, MenuFont, Time);

	switch (MenuState)
	{
	case EMenuState::Main:
		DrawMainMenu();
		DrawCareerStats();
		break;
	case EMenuState::Settings:
		DrawSettingsScreen();
		break;
	case EMenuState::Credits:
		DrawCreditsScreen();
		break;
	}

	DrawVersionText();
	DrawBottomTips();
	if (bSearching) DrawMatchmakingOverlay();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawCenteredText(const FString& Text, float Y,
	float Scale, FLinearColor Color)
{
	float TW, TH;
	GetTextSize(Text, TW, TH, MenuFont, Scale);
	DrawText(Text, Color, (Canvas->SizeX - TW) * 0.5f, Y, MenuFont, Scale);
}

void AExoMenuHUD::DrawMenuOption(const FString& Label, float X, float Y,
	float W, float H, bool bSelected, float Time)
{
	// Background
	FLinearColor BgCol = bSelected
		? FLinearColor(0.0f, 0.12f, 0.2f, 0.85f)
		: FLinearColor(0.03f, 0.03f, 0.06f, 0.6f);
	DrawRect(BgCol, X, Y, W, H);

	// Border
	FLinearColor BorderCol = bSelected
		? FLinearColor(0.0f, 0.8f, 1.0f, 0.7f + 0.2f * FMath::Sin(Time * 3.f))
		: FLinearColor(0.15f, 0.2f, 0.25f, 0.4f);
	DrawLine(X, Y, X + W, Y, BorderCol);
	DrawLine(X, Y + H, X + W, Y + H, BorderCol);
	DrawLine(X, Y, X, Y + H, BorderCol);
	DrawLine(X + W, Y, X + W, Y + H, BorderCol);

	// Selected highlight glow bar on left
	if (bSelected)
	{
		float GlowAlpha = 0.6f + 0.3f * FMath::Sin(Time * 4.f);
		FLinearColor GlowBar(0.0f, 0.7f, 1.0f, GlowAlpha);
		DrawRect(GlowBar, X, Y, 4.f, H);

		// Pulsing brackets as selection indicator
		float BracketAlpha = 0.7f + 0.3f * FMath::Sin(Time * 5.f);
		FLinearColor BracketCol(0.0f, 0.9f, 1.0f, BracketAlpha);
		float BScale = 1.1f;

		FString LeftBracket = TEXT(">");
		float BW, BH;
		GetTextSize(LeftBracket, BW, BH, MenuFont, BScale);
		DrawText(LeftBracket, BracketCol, X + 14.f, Y + (H - BH) * 0.5f,
			MenuFont, BScale);

		FString RightBracket = TEXT("<");
		GetTextSize(RightBracket, BW, BH, MenuFont, BScale);
		DrawText(RightBracket, BracketCol, X + W - BW - 14.f,
			Y + (H - BH) * 0.5f, MenuFont, BScale);
	}

	// Label text — centered, slightly larger when selected
	float TextScale = bSelected ? 1.35f : 1.15f;
	FLinearColor TextCol = bSelected ? ColorCyanBright : ColorWhite;
	float LW, LH;
	GetTextSize(Label, LW, LH, MenuFont, TextScale);
	DrawText(Label, TextCol, X + (W - LW) * 0.5f, Y + (H - LH) * 0.5f,
		MenuFont, TextScale);
}

// ---------------------------------------------------------------------------
// Navigation input
// ---------------------------------------------------------------------------

void AExoMenuHUD::NavigateUp()
{
	if (MenuState == EMenuState::Main)
	{
		SelectedIndex = FMath::Max(SelectedIndex - 1, 0);
	}
	else if (MenuState == EMenuState::Settings)
	{
		FExoSettingsMenu::NavigateUp();
	}
}

void AExoMenuHUD::NavigateDown()
{
	if (MenuState == EMenuState::Main)
	{
		SelectedIndex = FMath::Min(SelectedIndex + 1, MainOptionCount - 1);
	}
	else if (MenuState == EMenuState::Settings)
	{
		FExoSettingsMenu::NavigateDown();
	}
}

void AExoMenuHUD::SelectCurrent()
{
	if (MenuState == EMenuState::Main)
	{
		HandleMainSelect();
	}
	else if (MenuState == EMenuState::Settings)
	{
		// Right arrow / Enter adjusts setting value up
		FExoSettingsMenu::AdjustValue(1.f);
	}
}

void AExoMenuHUD::GoBack()
{
	// Cancel matchmaking search
	if (bSearching)
	{
		if (UExoMatchmakingManager* MM = UExoMatchmakingManager::Get(GetWorld()))
			MM->CancelSearch();
		bSearching = false;
		return;
	}

	if (MenuState == EMenuState::Settings)
	{
		FExoSettingsMenu::bIsOpen = false;
		MenuState = EMenuState::Main;
		SelectedIndex = 1; // Return to "SETTINGS" position
	}
	else if (MenuState == EMenuState::Credits)
	{
		MenuState = EMenuState::Main;
		SelectedIndex = 2; // Return to "CREDITS" position
	}
	else if (MenuState == EMenuState::Main)
	{
		// Left arrow on settings adjusts value down
		FExoSettingsMenu::AdjustValue(-1.f);
	}
}

// ---------------------------------------------------------------------------
// Main menu selection actions
// ---------------------------------------------------------------------------

void AExoMenuHUD::HandleMainSelect()
{
	switch (SelectedIndex)
	{
	case 0: // PLAY — start matchmaking
	{
		if (UExoMatchmakingManager* MM = UExoMatchmakingManager::Get(GetWorld()))
		{
			MM->StartSearching();
			bSearching = true;
		}
		break;
	}
	case 1: // SETTINGS
		MenuState = EMenuState::Settings;
		FExoSettingsMenu::bIsOpen = true;
		FExoSettingsMenu::SelectedOption = 0;
		break;
	case 2: // CREDITS
		MenuState = EMenuState::Credits;
		break;
	case 3: // QUIT
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UKismetSystemLibrary::QuitGame(World,
				World->GetFirstPlayerController(),
				EQuitPreference::Quit, false);
		}
		break;
	}
	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// Matchmaking tick
// ---------------------------------------------------------------------------

void AExoMenuHUD::TickMatchmaking(float DeltaTime)
{
	if (!bSearching) return;
	UExoMatchmakingManager* MM = UExoMatchmakingManager::Get(GetWorld());
	if (!MM) return;
	MM->Tick(DeltaTime);

	if (MM->GetState() == EMatchmakingState::Idle)
		bSearching = false;
}
