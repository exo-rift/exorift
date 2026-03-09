#include "UI/ExoCountdown.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

// ---------------------------------------------------------------------------
// Pre-match countdown  (WaitingForPlayers phase)
// ---------------------------------------------------------------------------

void FExoCountdown::DrawPreMatchCountdown(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float TimeRemaining, int32 PlayerCount, int32 MinPlayers)
{
	if (!HUD || !Canvas || !Font) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const float CenterX = ScreenW * 0.5f;
	const float Time = HUD->GetWorld()->GetTimeSeconds();

	// --- Dark overlay ---
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f), 0.f, 0.f, ScreenW, ScreenH);

	const bool bEnoughPlayers = (PlayerCount >= MinPlayers);

	if (bEnoughPlayers && TimeRemaining > 0.f)
	{
		// --- "MATCH STARTING IN" label ---
		const FString HeaderText = TEXT("MATCH STARTING IN");
		float HW, HH;
		HUD->GetTextSize(HeaderText, HW, HH, Font, 1.3f);
		float HeaderX = (ScreenW - HW) * 0.5f;
		float HeaderY = ScreenH * 0.3f;
		HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f),
			HeaderX - 15.f, HeaderY - 5.f, HW + 30.f, HH + 10.f);
		HUD->DrawText(HeaderText, FLinearColor(0.8f, 0.85f, 0.9f, 1.f),
			HeaderX, HeaderY, Font, 1.3f);

		// --- Big countdown number ---
		int32 Seconds = FMath::CeilToInt(FMath::Max(TimeRemaining, 0.f));
		FString NumText = FString::Printf(TEXT("%d"), Seconds);

		// Scale pulse: briefly larger at each whole-second boundary
		float Frac = TimeRemaining - FMath::FloorToFloat(TimeRemaining);
		float ScalePulse = 1.f + FMath::Clamp(1.f - Frac, 0.f, 1.f) * 0.3f;
		// Ease out the pulse quickly
		ScalePulse = 1.f + (ScalePulse - 1.f) * FMath::Clamp(1.f - Frac * 3.f, 0.f, 1.f);

		float NumScale = 3.5f * ScalePulse;
		float NW, NH;
		HUD->GetTextSize(NumText, NW, NH, Font, NumScale);
		float NumX = (ScreenW - NW) * 0.5f;
		float NumY = HeaderY + HH + 20.f;

		// Color shifts towards red/orange in last 3 seconds
		FLinearColor NumColor;
		if (Seconds <= 1)
			NumColor = FLinearColor(1.f, 0.3f, 0.2f, 1.f);
		else if (Seconds <= 3)
			NumColor = FLinearColor(1.f, 0.7f, 0.2f, 1.f);
		else
			NumColor = FLinearColor(0.9f, 0.95f, 1.f, 1.f);

		HUD->DrawText(NumText, NumColor, NumX, NumY, Font, NumScale);

		// --- Player count ---
		FString PlayerText = FString::Printf(TEXT("Players: %d / %d required"),
			PlayerCount, MinPlayers);
		float PW, PH;
		HUD->GetTextSize(PlayerText, PW, PH, Font, 1.f);
		float PlayerX = (ScreenW - PW) * 0.5f;
		float PlayerY = NumY + NH + 30.f;
		HUD->DrawText(PlayerText, FLinearColor(0.6f, 0.7f, 0.8f, 0.9f),
			PlayerX, PlayerY, Font, 1.f);
	}
	else
	{
		// --- Not enough players: "WAITING FOR PLAYERS..." with animated dots ---
		int32 DotCount = (FMath::FloorToInt(Time * 2.f) % 4); // 0..3 dots cycle
		FString Dots;
		for (int32 i = 0; i < DotCount; ++i) Dots += TEXT(".");
		FString WaitText = FString::Printf(TEXT("WAITING FOR PLAYERS%s"), *Dots);

		float WW, WH;
		HUD->GetTextSize(WaitText, WW, WH, Font, 1.8f);
		float WaitX = (ScreenW - WW) * 0.5f;
		float WaitY = ScreenH * 0.38f;

		// Gentle pulse
		float Pulse = 0.7f + 0.3f * FMath::Abs(FMath::Sin(Time * 2.f));
		FLinearColor WaitColor(0.8f, 0.85f, 0.9f, Pulse);

		HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f),
			WaitX - 20.f, WaitY - 8.f, WW + 40.f, WH + 16.f);
		HUD->DrawText(WaitText, WaitColor, WaitX, WaitY, Font, 1.8f);

		// Player count underneath
		FString PlayerText = FString::Printf(TEXT("Players: %d / %d required"),
			PlayerCount, MinPlayers);
		float PW, PH;
		HUD->GetTextSize(PlayerText, PW, PH, Font, 1.f);
		float PlayerX = (ScreenW - PW) * 0.5f;
		float PlayerY = WaitY + WH + 20.f;
		HUD->DrawText(PlayerText, FLinearColor(0.6f, 0.7f, 0.8f, 0.8f),
			PlayerX, PlayerY, Font, 1.f);
	}
}

// ---------------------------------------------------------------------------
// Drop-phase HUD
// ---------------------------------------------------------------------------

void FExoCountdown::DrawDropPhaseHUD(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float TimeRemaining)
{
	if (!HUD || !Canvas || !Font) return;

	const float ScreenW = Canvas->SizeX;

	// --- "DEPLOYING IN" header ---
	const FString HeaderText = TEXT("DEPLOYING IN");
	float HW, HH;
	HUD->GetTextSize(HeaderText, HW, HH, Font, 1.3f);
	float HeaderX = (ScreenW - HW) * 0.5f;
	float HeaderY = Canvas->SizeY * 0.22f;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f),
		HeaderX - 15.f, HeaderY - 5.f, HW + 30.f, HH + 10.f);
	HUD->DrawText(HeaderText, FLinearColor(0.3f, 0.8f, 1.f, 1.f),
		HeaderX, HeaderY, Font, 1.3f);

	// --- Countdown number ---
	int32 Seconds = FMath::CeilToInt(FMath::Max(TimeRemaining, 0.f));
	FString NumText = FString::Printf(TEXT("%d"), Seconds);

	float Frac = TimeRemaining - FMath::FloorToFloat(TimeRemaining);
	float ScalePulse = 1.f + FMath::Clamp(1.f - Frac, 0.f, 1.f) * 0.2f;
	ScalePulse = 1.f + (ScalePulse - 1.f) * FMath::Clamp(1.f - Frac * 3.f, 0.f, 1.f);

	float NumScale = 2.8f * ScalePulse;
	float NW, NH;
	HUD->GetTextSize(NumText, NW, NH, Font, NumScale);
	float NumX = (ScreenW - NW) * 0.5f;
	float NumY = HeaderY + HH + 15.f;

	FLinearColor NumColor = FLinearColor(0.3f, 0.85f, 1.f, 1.f);
	if (Seconds <= 3) NumColor = FLinearColor(1.f, 0.7f, 0.2f, 1.f);
	HUD->DrawText(NumText, NumColor, NumX, NumY, Font, NumScale);

	// --- Subtitle ---
	const FString SubText = TEXT("Choose your landing zone");
	float SW, SH;
	HUD->GetTextSize(SubText, SW, SH, Font, 1.f);
	float SubX = (ScreenW - SW) * 0.5f;
	float SubY = NumY + NH + 20.f;
	HUD->DrawText(SubText, FLinearColor(0.6f, 0.75f, 0.85f, 0.85f),
		SubX, SubY, Font, 1.f);
}

// ---------------------------------------------------------------------------
// "GO!" banner (first 2 seconds of Playing phase)
// ---------------------------------------------------------------------------

void FExoCountdown::DrawMatchStartBanner(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float MatchTime)
{
	if (!HUD || !Canvas || !Font) return;
	if (MatchTime >= GoBannerDuration) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;

	// Alpha: fully opaque at start, fades out over the last 0.8s
	float FadeStart = GoBannerDuration - 0.8f;
	float Alpha = (MatchTime > FadeStart)
		? FMath::Clamp(1.f - (MatchTime - FadeStart) / 0.8f, 0.f, 1.f)
		: 1.f;

	// Scale: starts at 1.5x, settles to 1x over the first 0.3s, then stays
	float ScaleT = FMath::Clamp(MatchTime / 0.3f, 0.f, 1.f);
	float Scale = FMath::Lerp(1.5f, 1.f, ScaleT);

	float FontScale = 4.f * Scale;
	const FString GoText = TEXT("GO!");
	float TW, TH;
	HUD->GetTextSize(GoText, TW, TH, Font, FontScale);
	float X = (ScreenW - TW) * 0.5f;
	float Y = (ScreenH - TH) * 0.5f - 40.f;

	FLinearColor GoColor(0.2f, 1.f, 0.4f, Alpha);

	// Glow background
	float Pad = 30.f;
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f * Alpha),
		X - Pad, Y - Pad * 0.5f, TW + Pad * 2.f, TH + Pad);
	HUD->DrawText(GoText, GoColor, X, Y, Font, FontScale);
}
