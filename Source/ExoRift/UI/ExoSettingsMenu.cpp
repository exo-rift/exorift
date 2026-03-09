#include "UI/ExoSettingsMenu.h"
#include "Core/ExoGameSettings.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

bool FExoSettingsMenu::bIsOpen = false;
int32 FExoSettingsMenu::SelectedOption = 0;

// ---------------------------------------------------------------------------
// Menu controls
// ---------------------------------------------------------------------------

void FExoSettingsMenu::ToggleMenu()
{
	bIsOpen = !bIsOpen;
	if (bIsOpen) SelectedOption = 0;
}

void FExoSettingsMenu::NavigateUp()
{
	if (!bIsOpen) return;
	SelectedOption = FMath::Max(SelectedOption - 1, 0);
}

void FExoSettingsMenu::NavigateDown()
{
	if (!bIsOpen) return;
	SelectedOption = FMath::Min(SelectedOption + 1, UExoGameSettings::SettingsCount - 1);
}

void FExoSettingsMenu::AdjustValue(float Direction)
{
	if (!bIsOpen) return;
	UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
	UExoGameSettings* Settings = UExoGameSettings::Get(World);
	if (!Settings) return;

	Settings->AdjustSetting(SelectedOption, Direction);
	Settings->SaveSettings();
	Settings->ApplySettings(World);
}

// ---------------------------------------------------------------------------
// Helper: draw corner brackets at a rect's corners
// ---------------------------------------------------------------------------

static void DrawCornerBrackets(AHUD* HUD, float X, float Y, float W, float H,
	const FLinearColor& Color, float Len = 12.f)
{
	// Top-left
	HUD->DrawLine(X, Y, X + Len, Y, Color);
	HUD->DrawLine(X, Y, X, Y + Len, Color);
	// Top-right
	HUD->DrawLine(X + W, Y, X + W - Len, Y, Color);
	HUD->DrawLine(X + W, Y, X + W, Y + Len, Color);
	// Bottom-left
	HUD->DrawLine(X, Y + H, X + Len, Y + H, Color);
	HUD->DrawLine(X, Y + H, X, Y + H - Len, Color);
	// Bottom-right
	HUD->DrawLine(X + W, Y + H, X + W - Len, Y + H, Color);
	HUD->DrawLine(X + W, Y + H, X + W, Y + H - Len, Color);
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void FExoSettingsMenu::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!bIsOpen || !HUD || !Canvas || !Font) return;

	UExoGameSettings* Settings = UExoGameSettings::Get(HUD->GetWorld());
	if (!Settings) return;

	const float CanvasW = Canvas->SizeX;
	const float CanvasH = Canvas->SizeY;

	// Full-screen dim overlay
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), 0.f, 0.f, CanvasW, CanvasH);

	// Panel dimensions
	const float PanelW = 520.f;
	const float RowH = 38.f;
	const float HeaderH = 56.f;
	const float FooterH = 44.f;
	const float PanelH = HeaderH + RowH * UExoGameSettings::SettingsCount + FooterH;
	const float PanelX = (CanvasW - PanelW) * 0.5f;
	const float PanelY = (CanvasH - PanelH) * 0.5f;

	// Colors
	FLinearColor BgColor(0.02f, 0.025f, 0.05f, 0.92f);
	FLinearColor Cyan(0.3f, 0.85f, 1.f, 1.f);
	FLinearColor CyanDim(0.15f, 0.4f, 0.6f, 0.6f);
	FLinearColor White(0.9f, 0.92f, 0.95f, 1.f);
	FLinearColor Dim(0.45f, 0.5f, 0.55f, 1.f);
	FLinearColor Shadow(0.f, 0.f, 0.f, 0.6f);

	// Background panel
	HUD->DrawRect(BgColor, PanelX, PanelY, PanelW, PanelH);

	// Corner brackets on outer panel
	DrawCornerBrackets(HUD, PanelX, PanelY, PanelW, PanelH, Cyan, 18.f);

	// Top accent bar (full width)
	HUD->DrawRect(Cyan, PanelX, PanelY, PanelW, 2.f);
	// Bottom accent bar
	HUD->DrawRect(Cyan, PanelX, PanelY + PanelH - 2.f, PanelW, 2.f);

	// Left accent stripe (header region)
	HUD->DrawRect(Cyan, PanelX, PanelY, 3.f, HeaderH);

	// Title — "SETTINGS" with shadow
	float TitleX = PanelX + 22.f;
	float TitleY = PanelY + 14.f;
	HUD->DrawText(TEXT("SETTINGS"), Shadow, TitleX + 1.f, TitleY + 1.f, Font, 1.4f);
	HUD->DrawText(TEXT("SETTINGS"), Cyan, TitleX, TitleY, Font, 1.4f);

	// Small diamond icon before title
	float DiamX = PanelX + 12.f;
	float DiamY = TitleY + 10.f;
	HUD->DrawLine(DiamX, DiamY, DiamX + 4.f, DiamY - 4.f, Cyan);
	HUD->DrawLine(DiamX + 4.f, DiamY - 4.f, DiamX + 8.f, DiamY, Cyan);
	HUD->DrawLine(DiamX + 8.f, DiamY, DiamX + 4.f, DiamY + 4.f, Cyan);
	HUD->DrawLine(DiamX + 4.f, DiamY + 4.f, DiamX, DiamY, Cyan);

	// Header separator with accent
	float SepY = PanelY + HeaderH - 4.f;
	HUD->DrawRect(CyanDim, PanelX + 8.f, SepY, PanelW - 16.f, 1.f);
	HUD->DrawRect(Cyan, PanelX + 8.f, SepY, 60.f, 1.f);

	// Settings rows
	float StartY = PanelY + HeaderH;
	FLinearColor HighlightBg(0.06f, 0.15f, 0.3f, 0.5f);
	FLinearColor SelectedAccent(0.3f, 0.9f, 1.f, 1.f);

	for (int32 i = 0; i < UExoGameSettings::SettingsCount; ++i)
	{
		float RowY = StartY + i * RowH;
		bool bSelected = (i == SelectedOption);

		// Alternating row tint
		if (i % 2 == 1)
		{
			HUD->DrawRect(FLinearColor(0.04f, 0.04f, 0.07f, 0.3f),
				PanelX + 4.f, RowY, PanelW - 8.f, RowH);
		}

		if (bSelected)
		{
			// Highlight background
			HUD->DrawRect(HighlightBg, PanelX + 4.f, RowY, PanelW - 8.f, RowH);
			// Left cyan accent stripe on selected row
			HUD->DrawRect(SelectedAccent, PanelX + 4.f, RowY + 2.f, 3.f, RowH - 4.f);
			// Top/bottom accent lines
			HUD->DrawRect(FLinearColor(Cyan.R, Cyan.G, Cyan.B, 0.3f),
				PanelX + 8.f, RowY, PanelW - 16.f, 1.f);
			HUD->DrawRect(FLinearColor(Cyan.R, Cyan.G, Cyan.B, 0.3f),
				PanelX + 8.f, RowY + RowH - 1.f, PanelW - 16.f, 1.f);
		}

		FLinearColor NameColor = bSelected ? SelectedAccent : White;
		FLinearColor ValColor = bSelected ? White : Dim;

		// Setting name with shadow
		float NameX = PanelX + 28.f;
		float TextY = RowY + 9.f;
		HUD->DrawText(Settings->GetSettingName(i), Shadow,
			NameX + 1.f, TextY + 1.f, Font, 0.9f);
		HUD->DrawText(Settings->GetSettingName(i), NameColor,
			NameX, TextY, Font, 0.9f);

		// Value on right with arrow indicators
		FString ValStr = Settings->GetSettingValue(i);
		if (bSelected)
		{
			ValStr = FString::Printf(TEXT("<  %s  >"), *ValStr);
		}
		float ValW, ValH;
		HUD->GetTextSize(ValStr, ValW, ValH, Font, 0.9f);
		float ValX = PanelX + PanelW - ValW - 28.f;
		HUD->DrawText(ValStr, Shadow, ValX + 1.f, TextY + 1.f, Font, 0.9f);
		HUD->DrawText(ValStr, ValColor, ValX, TextY, Font, 0.9f);

		// Small dot indicator for selected row
		if (bSelected)
		{
			float DotX = PanelX + 15.f;
			float DotY = RowY + RowH * 0.5f;
			HUD->DrawRect(SelectedAccent, DotX, DotY - 1.5f, 3.f, 3.f);
		}
	}

	// Footer separator
	float FootSepY = StartY + UExoGameSettings::SettingsCount * RowH;
	HUD->DrawRect(CyanDim, PanelX + 8.f, FootSepY, PanelW - 16.f, 1.f);

	// Footer hint with shadow
	float FootY = FootSepY + 12.f;
	FString FooterText = TEXT("[Arrows] Navigate / Adjust     [ESC] Close");
	HUD->DrawText(FooterText, Shadow, PanelX + 28.f + 1.f, FootY + 1.f, Font, 0.7f);
	HUD->DrawText(FooterText, Dim, PanelX + 28.f, FootY, Font, 0.7f);

	// Bottom-right version tag
	FString VerTag = TEXT("v1.0");
	float VerW, VerH;
	HUD->GetTextSize(VerTag, VerW, VerH, Font, 0.6f);
	HUD->DrawText(VerTag, FLinearColor(0.3f, 0.3f, 0.4f, 0.5f),
		PanelX + PanelW - VerW - 12.f, PanelY + PanelH - VerH - 8.f, Font, 0.6f);
}
