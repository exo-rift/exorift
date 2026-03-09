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
// Draw
// ---------------------------------------------------------------------------

void FExoSettingsMenu::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!bIsOpen || !HUD || !Canvas || !Font) return;

	UExoGameSettings* Settings = UExoGameSettings::Get(HUD->GetWorld());
	if (!Settings) return;

	const float CanvasW = Canvas->SizeX;
	const float CanvasH = Canvas->SizeY;

	// Panel dimensions
	const float PanelW = 500.f;
	const float RowH = 36.f;
	const float PanelH = 60.f + RowH * UExoGameSettings::SettingsCount + 40.f;
	const float PanelX = (CanvasW - PanelW) * 0.5f;
	const float PanelY = (CanvasH - PanelH) * 0.5f;

	// Background
	FLinearColor BgColor(0.03f, 0.03f, 0.06f, 0.88f);
	HUD->DrawRect(BgColor, PanelX, PanelY, PanelW, PanelH);

	// Border
	FLinearColor Border(0.2f, 0.5f, 1.f, 0.6f);
	HUD->DrawLine(PanelX, PanelY, PanelX + PanelW, PanelY, Border);
	HUD->DrawLine(PanelX, PanelY + PanelH, PanelX + PanelW, PanelY + PanelH, Border);
	HUD->DrawLine(PanelX, PanelY, PanelX, PanelY + PanelH, Border);
	HUD->DrawLine(PanelX + PanelW, PanelY, PanelX + PanelW, PanelY + PanelH, Border);

	// Title
	FLinearColor TitleColor(0.4f, 0.8f, 1.f, 1.f);
	FLinearColor White(0.9f, 0.92f, 0.95f, 1.f);
	FLinearColor Dim(0.5f, 0.55f, 0.6f, 1.f);

	float TitleX = PanelX + 20.f;
	float TitleY = PanelY + 14.f;
	HUD->DrawText(TEXT("SETTINGS"), TitleColor, TitleX, TitleY, Font, 1.3f);

	// Separator
	HUD->DrawLine(PanelX + 10.f, PanelY + 48.f, PanelX + PanelW - 10.f, PanelY + 48.f, Border);

	// Settings rows
	float StartY = PanelY + 60.f;
	FLinearColor HighlightBg(0.1f, 0.3f, 0.6f, 0.4f);
	FLinearColor SelectedColor(0.3f, 0.9f, 1.f, 1.f);

	for (int32 i = 0; i < UExoGameSettings::SettingsCount; ++i)
	{
		float RowY = StartY + i * RowH;
		bool bSelected = (i == SelectedOption);

		if (bSelected)
		{
			HUD->DrawRect(HighlightBg, PanelX + 4.f, RowY - 2.f, PanelW - 8.f, RowH - 2.f);
		}

		FLinearColor NameColor = bSelected ? SelectedColor : White;
		FLinearColor ValColor = bSelected ? White : Dim;

		// Setting name on left
		HUD->DrawText(Settings->GetSettingName(i), NameColor,
			PanelX + 30.f, RowY + 6.f, Font, 0.9f);

		// Value on right with arrows
		FString ValStr = Settings->GetSettingValue(i);
		if (bSelected)
		{
			ValStr = FString::Printf(TEXT("< %s >"), *ValStr);
		}
		float ValW, ValH;
		HUD->GetTextSize(ValStr, ValW, ValH, Font, 0.9f);
		HUD->DrawText(ValStr, ValColor,
			PanelX + PanelW - ValW - 30.f, RowY + 6.f, Font, 0.9f);
	}

	// Footer hint
	float FootY = StartY + UExoGameSettings::SettingsCount * RowH + 10.f;
	HUD->DrawText(TEXT("Arrow Keys: Navigate / Adjust    ESC: Close"), Dim,
		PanelX + 30.f, FootY, Font, 0.7f);
}
