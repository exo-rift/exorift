#include "UI/ExoMenuHUD.h"
#include "UI/ExoMenuBackground.h"
#include "UI/ExoSettingsMenu.h"
#include "Map/ExoMapConfig.h"
#include "Core/ExoMatchmakingManager.h"
#include "Core/ExoCareerStats.h"
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
// Main menu options
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawMainMenu()
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Time = GetWorld()->GetTimeSeconds();

	const FString Options[] = {
		TEXT("PLAY"),
		TEXT("SETTINGS"),
		TEXT("CREDITS"),
		TEXT("QUIT")
	};

	float OptionW = 320.f;
	float OptionH = 52.f;
	float OptionGap = 12.f;
	float TotalH = MainOptionCount * OptionH + (MainOptionCount - 1) * OptionGap;
	float StartX = (W - OptionW) * 0.5f;
	float StartY = (H - TotalH) * 0.5f + H * 0.05f;

	for (int32 i = 0; i < MainOptionCount; ++i)
	{
		float Y = StartY + i * (OptionH + OptionGap);
		bool bSelected = (i == SelectedIndex);
		DrawMenuOption(Options[i], StartX, Y, OptionW, OptionH, bSelected, Time);
	}

	// Player count placeholder
	FString OnlineText = TEXT("Players Online: --");
	float OW, OH;
	GetTextSize(OnlineText, OW, OH, MenuFont, 0.8f);
	float OX = (W - OW) * 0.5f;
	float OY = StartY + TotalH + 50.f;
	DrawText(OnlineText, ColorDimWhite, OX, OY, MenuFont, 0.8f);
}

// ---------------------------------------------------------------------------
// Settings screen — delegates to FExoSettingsMenu + "Back" hint
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawSettingsScreen()
{
	// Force the static settings menu open so it draws
	FExoSettingsMenu::bIsOpen = true;
	FExoSettingsMenu::Draw(this, Canvas, MenuFont);

	// "Press ESC to return" below the settings panel
	FString BackHint = TEXT("Press ESC to return");
	float BW, BH;
	GetTextSize(BackHint, BW, BH, MenuFont, 0.8f);
	float BX = (Canvas->SizeX - BW) * 0.5f;
	float BY = Canvas->SizeY * 0.82f;
	DrawText(BackHint, ColorDimWhite, BX, BY, MenuFont, 0.8f);
}

// ---------------------------------------------------------------------------
// Credits screen
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawCreditsScreen()
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	const FString Lines[] = {
		TEXT("EXORIFT BATTLE ROYALE"),
		TEXT(""),
		TEXT("Developed by Spot Cloud b.v."),
		TEXT(""),
		TEXT("Powered by Unreal Engine 5.7"),
		TEXT(""),
		TEXT("Copyright 2026 Spot Cloud b.v."),
		TEXT("All rights reserved.")
	};

	float LineHeight = 38.f;
	int32 Count = UE_ARRAY_COUNT(Lines);
	float TotalHeight = Count * LineHeight;
	float StartY = (H - TotalHeight) * 0.5f - 20.f;

	for (int32 i = 0; i < Count; ++i)
	{
		if (Lines[i].IsEmpty()) continue;

		FLinearColor Col = (i == 0) ? ColorCyan : ColorWhite;
		float Scale = (i == 0) ? 1.6f : 1.0f;
		DrawCenteredText(Lines[i], StartY + i * LineHeight, Scale, Col);
	}

	// ESC hint
	FString BackHint = TEXT("Press ESC to return");
	float BW, BH;
	GetTextSize(BackHint, BW, BH, MenuFont, 0.8f);
	DrawText(BackHint, ColorDimWhite, (W - BW) * 0.5f, H * 0.85f, MenuFont, 0.8f);
}

// ---------------------------------------------------------------------------
// Version text — bottom-right
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawVersionText()
{
	FString Version = TEXT("v0.1.0 ALPHA");
	float VW, VH;
	GetTextSize(Version, VW, VH, MenuFont, 0.75f);
	float VX = Canvas->SizeX - VW - 20.f;
	float VY = Canvas->SizeY - VH - 15.f;
	DrawText(Version, ColorDimWhite, VX, VY, MenuFont, 0.75f);
}

// ---------------------------------------------------------------------------
// Bottom tip bar
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawBottomTips()
{
	if (MenuState != EMenuState::Main) return;

	FString Tip = TEXT("Arrow keys to navigate  —  Enter to select");
	float TW, TH;
	GetTextSize(Tip, TW, TH, MenuFont, 0.7f);
	float TX = (Canvas->SizeX - TW) * 0.5f;
	float TY = Canvas->SizeY - TH - 40.f;

	FLinearColor TipBg(0.f, 0.f, 0.f, 0.3f);
	DrawRect(TipBg, TX - 15.f, TY - 5.f, TW + 30.f, TH + 10.f);
	DrawText(Tip, ColorDimWhite, TX, TY, MenuFont, 0.7f);
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
// Matchmaking
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

void AExoMenuHUD::DrawMatchmakingOverlay()
{
	UExoMatchmakingManager* MM = UExoMatchmakingManager::Get(GetWorld());
	if (!MM) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Time = GetWorld()->GetTimeSeconds();

	// Dim overlay
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), 0, 0, W, H);

	float CenterY = H * 0.4f;

	if (MM->GetState() == EMatchmakingState::Searching)
	{
		// Animated "SEARCHING..." text
		int32 Dots = ((int32)(Time * 2.f) % 4);
		FString Searching = TEXT("SEARCHING");
		for (int32 i = 0; i < Dots; i++) Searching += TEXT(".");
		DrawCenteredText(Searching, CenterY, 1.5f, ColorCyan);

		// Info line
		FString Info = FString::Printf(TEXT("Region: %s  |  Ping: %dms  |  Players: %d/25"),
			*MM->GetRegion(), MM->GetPing(), MM->GetPlayersFound());
		DrawCenteredText(Info, CenterY + 50.f, 0.9f, ColorWhite);

		// Progress bar
		float BarW = 300.f;
		float BarH = 6.f;
		float BarX = (W - BarW) * 0.5f;
		float BarY = CenterY + 90.f;
		float Progress = FMath::Clamp(MM->GetPlayersFound() / 25.f, 0.f, 1.f);
		DrawRect(FLinearColor(0.1f, 0.1f, 0.15f, 0.8f), BarX, BarY, BarW, BarH);
		DrawRect(ColorCyan, BarX, BarY, BarW * Progress, BarH);

		// Cancel hint
		DrawCenteredText(TEXT("Press ESC to cancel"), CenterY + 130.f, 0.8f, ColorDimWhite);
	}
	else if (MM->GetState() == EMatchmakingState::Found)
	{
		DrawCenteredText(TEXT("MATCH FOUND!"), CenterY, 1.8f, ColorCyanBright);
		DrawCenteredText(TEXT("Loading..."), CenterY + 55.f, 1.0f, ColorWhite);
	}
}

// ---------------------------------------------------------------------------
// Career stats panel — bottom-left corner
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawCareerStats()
{
	UExoCareerStats* Stats = UExoCareerStats::Get(GetWorld());
	if (!Stats || Stats->GetTotalMatches() == 0) return;

	const float Padding = 15.f;
	const float X = 20.f;
	float Y = Canvas->SizeY - 170.f;

	DrawRect(FLinearColor(0.01f, 0.01f, 0.04f, 0.7f), X, Y, 260.f, 150.f);

	Y += Padding;
	DrawText(TEXT("CAREER STATS"), ColorCyan, X + Padding, Y, MenuFont, 0.85f);
	Y += 24.f;

	FLinearColor StatColor(0.7f, 0.72f, 0.78f, 0.9f);
	auto DrawStat = [&](const FString& Label, const FString& Value)
	{
		DrawText(Label, StatColor, X + Padding, Y, MenuFont, 0.7f);
		float VW, VH;
		GetTextSize(Value, VW, VH, MenuFont, 0.7f);
		DrawText(Value, ColorWhite, X + 245.f - VW, Y, MenuFont, 0.7f);
		Y += 20.f;
	};

	DrawStat(TEXT("Matches"), FString::Printf(TEXT("%d"), Stats->GetTotalMatches()));
	DrawStat(TEXT("Wins"), FString::Printf(TEXT("%d (%.0f%%)"), Stats->GetTotalWins(), Stats->GetWinRate()));
	DrawStat(TEXT("K/D"), FString::Printf(TEXT("%.2f"), Stats->GetKDRatio()));
	DrawStat(TEXT("Avg Kills"), FString::Printf(TEXT("%.1f"), Stats->GetAverageKillsPerMatch()));
	DrawStat(TEXT("Best Place"), FString::Printf(TEXT("#%d"), Stats->GetBestPlacement()));
}
