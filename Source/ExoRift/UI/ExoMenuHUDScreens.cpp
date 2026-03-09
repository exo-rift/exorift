// ExoMenuHUDScreens.cpp — Menu sub-screens: main options, credits, matchmaking overlay, career stats
#include "UI/ExoMenuHUD.h"
#include "UI/ExoSettingsMenu.h"
#include "Core/ExoMatchmakingManager.h"
#include "Core/ExoCareerStats.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

// ---------------------------------------------------------------------------
// Helper: corner brackets
// ---------------------------------------------------------------------------

static void DrawBrackets(AHUD* HUD, float X, float Y, float W, float H,
	const FLinearColor& Color, float Len = 14.f)
{
	HUD->DrawLine(X, Y, X + Len, Y, Color);
	HUD->DrawLine(X, Y, X, Y + Len, Color);
	HUD->DrawLine(X + W, Y, X + W - Len, Y, Color);
	HUD->DrawLine(X + W, Y, X + W, Y + Len, Color);
	HUD->DrawLine(X, Y + H, X + Len, Y + H, Color);
	HUD->DrawLine(X, Y + H, X, Y + H - Len, Color);
	HUD->DrawLine(X + W, Y + H, X + W - Len, Y + H, Color);
	HUD->DrawLine(X + W, Y + H, X + W, Y + H - Len, Color);
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

	// Player count placeholder with panel
	FString OnlineText = TEXT("Players Online: --");
	float OW, OH;
	GetTextSize(OnlineText, OW, OH, MenuFont, 0.8f);
	float OX = (W - OW) * 0.5f;
	float OY = StartY + TotalH + 50.f;
	DrawRect(FLinearColor(0.01f, 0.01f, 0.03f, 0.5f),
		OX - 20.f, OY - 5.f, OW + 40.f, OH + 10.f);
	DrawText(OnlineText, ColorDimWhite, OX, OY, MenuFont, 0.8f);
}

// ---------------------------------------------------------------------------
// Settings screen
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawSettingsScreen()
{
	FExoSettingsMenu::bIsOpen = true;
	FExoSettingsMenu::Draw(this, Canvas, MenuFont);

	FString BackHint = TEXT("Press ESC to return");
	float BW, BH;
	GetTextSize(BackHint, BW, BH, MenuFont, 0.8f);
	float BX = (Canvas->SizeX - BW) * 0.5f;
	float BY = Canvas->SizeY * 0.82f;
	DrawText(BackHint, ColorDimWhite, BX, BY, MenuFont, 0.8f);
}

// ---------------------------------------------------------------------------
// Credits screen — sci-fi styled panel
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawCreditsScreen()
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Time = GetWorld()->GetTimeSeconds();

	// Dim overlay
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), 0.f, 0.f, W, H);

	// Credit lines
	struct FCreditLine { FString Text; float Scale; bool bAccent; };
	const FCreditLine Lines[] = {
		{ TEXT("EXORIFT BATTLE ROYALE"), 1.8f, true },
		{ TEXT(""), 0.f, false },
		{ TEXT("Developed by"), 0.85f, false },
		{ TEXT("SPOT CLOUD B.V."), 1.2f, true },
		{ TEXT(""), 0.f, false },
		{ TEXT("Powered by"), 0.85f, false },
		{ TEXT("UNREAL ENGINE 5.7"), 1.1f, true },
		{ TEXT(""), 0.f, false },
		{ TEXT("Copyright 2026 Spot Cloud b.v."), 0.85f, false },
		{ TEXT("All rights reserved."), 0.8f, false },
	};

	const int32 Count = UE_ARRAY_COUNT(Lines);
	float LineHeight = 36.f;
	float TotalHeight = Count * LineHeight;
	float PanelW = 500.f;
	float PanelH = TotalHeight + 60.f;
	float PanelX = (W - PanelW) * 0.5f;
	float PanelY = (H - PanelH) * 0.5f - 20.f;

	// Panel background
	DrawRect(FLinearColor(0.02f, 0.025f, 0.05f, 0.9f), PanelX, PanelY, PanelW, PanelH);

	// Corner brackets
	DrawBrackets(this, PanelX, PanelY, PanelW, PanelH, ColorCyan, 20.f);

	// Top/bottom accent bars
	DrawRect(ColorCyan, PanelX, PanelY, PanelW, 2.f);
	DrawRect(ColorCyan, PanelX, PanelY + PanelH - 2.f, PanelW, 2.f);

	// Diamond icon at top center
	float DX = W * 0.5f;
	float DY = PanelY + 18.f;
	float DS = 6.f;
	DrawLine(DX, DY - DS, DX + DS, DY, ColorCyan);
	DrawLine(DX + DS, DY, DX, DY + DS, ColorCyan);
	DrawLine(DX, DY + DS, DX - DS, DY, ColorCyan);
	DrawLine(DX - DS, DY, DX, DY - DS, ColorCyan);

	// Draw credit lines
	float StartY = PanelY + 35.f;
	FLinearColor Shadow(0.f, 0.f, 0.f, 0.6f);

	for (int32 i = 0; i < Count; ++i)
	{
		if (Lines[i].Text.IsEmpty()) continue;

		FLinearColor Col = Lines[i].bAccent ? ColorCyanBright : ColorWhite;
		float Scale = Lines[i].Scale;
		float TW, TH;
		GetTextSize(Lines[i].Text, TW, TH, MenuFont, Scale);
		float TX = (W - TW) * 0.5f;
		float TY = StartY + i * LineHeight;

		DrawText(Lines[i].Text, Shadow, TX + 1.f, TY + 1.f, MenuFont, Scale);
		DrawText(Lines[i].Text, Col, TX, TY, MenuFont, Scale);
	}

	// Separator before copyright
	float SepY = StartY + 7 * LineHeight - 5.f;
	FLinearColor SepCol(0.15f, 0.4f, 0.6f, 0.4f);
	DrawLine(PanelX + 40.f, SepY, PanelX + PanelW - 40.f, SepY, SepCol);

	// ESC hint
	FString BackHint = TEXT("[ESC] Return");
	float BW, BH;
	GetTextSize(BackHint, BW, BH, MenuFont, 0.8f);
	DrawText(BackHint, ColorDimWhite, (W - BW) * 0.5f, PanelY + PanelH + 20.f,
		MenuFont, 0.8f);
}

// ---------------------------------------------------------------------------
// Version text
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
// Bottom tips bar
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawBottomTips()
{
	if (MenuState != EMenuState::Main) return;

	FString Tip = TEXT("[Arrows] Navigate     [Enter] Select");
	float TW, TH;
	GetTextSize(Tip, TW, TH, MenuFont, 0.7f);
	float TX = (Canvas->SizeX - TW) * 0.5f;
	float TY = Canvas->SizeY - TH - 40.f;

	DrawRect(FLinearColor(0.01f, 0.01f, 0.03f, 0.5f),
		TX - 20.f, TY - 6.f, TW + 40.f, TH + 12.f);
	// Subtle top accent
	DrawRect(FLinearColor(0.f, 0.5f, 0.8f, 0.3f),
		TX - 20.f, TY - 6.f, TW + 40.f, 1.f);
	DrawText(Tip, ColorDimWhite, TX, TY, MenuFont, 0.7f);
}

// ---------------------------------------------------------------------------
// Matchmaking overlay — animated search + match found
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawMatchmakingOverlay()
{
	UExoMatchmakingManager* MM = UExoMatchmakingManager::Get(GetWorld());
	if (!MM) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Time = GetWorld()->GetTimeSeconds();

	// Dim overlay
	DrawRect(FLinearColor(0.f, 0.f, 0.02f, 0.65f), 0, 0, W, H);

	float CenterY = H * 0.38f;

	if (MM->GetState() == EMatchmakingState::Searching)
	{
		// Panel
		float PanelW = 440.f;
		float PanelH = 220.f;
		float PanelX = (W - PanelW) * 0.5f;
		float PanelY = CenterY - 30.f;

		DrawRect(FLinearColor(0.02f, 0.025f, 0.05f, 0.92f),
			PanelX, PanelY, PanelW, PanelH);
		DrawBrackets(this, PanelX, PanelY, PanelW, PanelH, ColorCyan, 16.f);
		DrawRect(ColorCyan, PanelX, PanelY, PanelW, 2.f);
		DrawRect(ColorCyan, PanelX, PanelY + PanelH - 2.f, PanelW, 2.f);

		// Spinning indicator (rotating line segments in a circle)
		float SpinCX = W * 0.5f;
		float SpinCY = PanelY + 40.f;
		float SpinR = 14.f;
		int32 Segments = 8;
		for (int32 i = 0; i < Segments; ++i)
		{
			float Angle = (float)i / Segments * 2.f * PI + Time * 4.f;
			float NextAngle = Angle + (2.f * PI / Segments) * 0.6f;
			float Alpha = (float)(i + 1) / Segments;
			FLinearColor SegCol(ColorCyan.R, ColorCyan.G, ColorCyan.B, Alpha * 0.8f);
			DrawLine(
				SpinCX + FMath::Cos(Angle) * SpinR,
				SpinCY + FMath::Sin(Angle) * SpinR,
				SpinCX + FMath::Cos(NextAngle) * SpinR,
				SpinCY + FMath::Sin(NextAngle) * SpinR,
				SegCol);
		}

		// "SEARCHING" with animated dots
		int32 Dots = ((int32)(Time * 2.f) % 4);
		FString Searching = TEXT("SEARCHING");
		for (int32 i = 0; i < Dots; i++) Searching += TEXT(".");
		float SW, SH;
		GetTextSize(Searching, SW, SH, MenuFont, 1.3f);
		DrawText(Searching, FLinearColor(0.f, 0.f, 0.f, 0.5f),
			(W - SW) * 0.5f + 1.f, PanelY + 60.f + 1.f, MenuFont, 1.3f);
		DrawText(Searching, ColorCyan,
			(W - SW) * 0.5f, PanelY + 60.f, MenuFont, 1.3f);

		// Info line with separator
		DrawLine(PanelX + 20.f, PanelY + 98.f,
			PanelX + PanelW - 20.f, PanelY + 98.f,
			FLinearColor(0.1f, 0.3f, 0.5f, 0.4f));

		FString Info = FString::Printf(TEXT("Region: %s  |  Ping: %dms  |  Players: %d/25"),
			*MM->GetRegion(), MM->GetPing(), MM->GetPlayersFound());
		float IW, IH;
		GetTextSize(Info, IW, IH, MenuFont, 0.85f);
		DrawText(Info, ColorWhite, (W - IW) * 0.5f, PanelY + 108.f, MenuFont, 0.85f);

		// Progress bar with glow
		float BarW = PanelW - 60.f;
		float BarH = 8.f;
		float BarX = PanelX + 30.f;
		float BarY = PanelY + 145.f;
		float Progress = FMath::Clamp(MM->GetPlayersFound() / 25.f, 0.f, 1.f);

		DrawRect(FLinearColor(0.05f, 0.05f, 0.1f, 0.8f), BarX, BarY, BarW, BarH);
		DrawRect(ColorCyan, BarX, BarY, BarW * Progress, BarH);
		// Glow at progress tip
		if (Progress > 0.01f)
		{
			float TipX = BarX + BarW * Progress - 3.f;
			DrawRect(FLinearColor(0.3f, 0.9f, 1.f, 0.5f),
				TipX, BarY - 2.f, 6.f, BarH + 4.f);
		}

		// Cancel hint
		FString Cancel = TEXT("[ESC] Cancel");
		float CW, CH;
		GetTextSize(Cancel, CW, CH, MenuFont, 0.8f);
		DrawText(Cancel, ColorDimWhite,
			(W - CW) * 0.5f, PanelY + PanelH - 32.f, MenuFont, 0.8f);
	}
	else if (MM->GetState() == EMatchmakingState::Found)
	{
		// Flash panel
		float FlashAlpha = 0.8f + 0.2f * FMath::Sin(Time * 6.f);
		float PanelW = 400.f;
		float PanelH = 120.f;
		float PanelX = (W - PanelW) * 0.5f;
		float PanelY = CenterY;

		DrawRect(FLinearColor(0.02f, 0.05f, 0.1f, 0.92f),
			PanelX, PanelY, PanelW, PanelH);
		DrawBrackets(this, PanelX, PanelY, PanelW, PanelH,
			FLinearColor(ColorCyan.R, ColorCyan.G, ColorCyan.B, FlashAlpha), 18.f);
		DrawRect(FLinearColor(ColorCyan.R, ColorCyan.G, ColorCyan.B, FlashAlpha),
			PanelX, PanelY, PanelW, 2.f);
		DrawRect(FLinearColor(ColorCyan.R, ColorCyan.G, ColorCyan.B, FlashAlpha),
			PanelX, PanelY + PanelH - 2.f, PanelW, 2.f);

		DrawCenteredText(TEXT("MATCH FOUND!"), PanelY + 25.f, 1.8f, ColorCyanBright);
		DrawCenteredText(TEXT("Loading..."), PanelY + 75.f, 1.0f, ColorWhite);
	}
}

// ---------------------------------------------------------------------------
// Career stats panel — bottom-left corner
// ---------------------------------------------------------------------------

void AExoMenuHUD::DrawCareerStats()
{
	UExoCareerStats* Stats = UExoCareerStats::Get(GetWorld());
	if (!Stats || Stats->GetTotalMatches() == 0) return;

	const float Pad = 15.f;
	const float PW = 270.f;
	const float PH = 160.f;
	const float X = 20.f;
	const float Y = Canvas->SizeY - PH - 20.f;

	// Panel background
	DrawRect(FLinearColor(0.02f, 0.025f, 0.05f, 0.85f), X, Y, PW, PH);

	// Corner brackets
	DrawBrackets(this, X, Y, PW, PH, ColorCyan, 12.f);

	// Left accent stripe
	DrawRect(ColorCyan, X, Y, 3.f, PH);

	// Top accent bar
	DrawRect(FLinearColor(0.f, 0.5f, 0.8f, 0.4f), X, Y, PW, 1.f);

	// Header
	FLinearColor Shadow(0.f, 0.f, 0.f, 0.5f);
	DrawText(TEXT("CAREER STATS"), Shadow, X + Pad + 6.f, Y + Pad + 1.f, MenuFont, 0.85f);
	DrawText(TEXT("CAREER STATS"), ColorCyan, X + Pad + 5.f, Y + Pad, MenuFont, 0.85f);

	// Separator
	float SepY = Y + Pad + 22.f;
	DrawLine(X + 8.f, SepY, X + PW - 8.f, SepY,
		FLinearColor(0.1f, 0.3f, 0.5f, 0.4f));
	DrawLine(X + 8.f, SepY, X + 60.f, SepY, ColorCyan);

	// Stats
	float StatY = SepY + 6.f;
	FLinearColor StatLabel(0.6f, 0.62f, 0.68f, 0.9f);

	auto DrawStat = [&](const FString& Label, const FString& Value)
	{
		DrawText(Label, StatLabel, X + Pad + 5.f, StatY, MenuFont, 0.7f);
		float VW, VH;
		GetTextSize(Value, VW, VH, MenuFont, 0.7f);
		DrawText(Value, ColorWhite, X + PW - VW - Pad, StatY, MenuFont, 0.7f);
		StatY += 21.f;
	};

	DrawStat(TEXT("Matches"), FString::Printf(TEXT("%d"), Stats->GetTotalMatches()));
	DrawStat(TEXT("Wins"), FString::Printf(TEXT("%d (%.0f%%)"),
		Stats->GetTotalWins(), Stats->GetWinRate()));
	DrawStat(TEXT("K/D"), FString::Printf(TEXT("%.2f"), Stats->GetKDRatio()));
	DrawStat(TEXT("Avg Kills"), FString::Printf(TEXT("%.1f"),
		Stats->GetAverageKillsPerMatch()));
	DrawStat(TEXT("Best Place"), FString::Printf(TEXT("#%d"),
		Stats->GetBestPlacement()));
}
