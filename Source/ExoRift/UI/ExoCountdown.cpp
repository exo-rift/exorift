// ExoCountdown.cpp — Pre-match, drop phase, and match start HUD overlays
#include "UI/ExoCountdown.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

void FExoCountdown::DrawPreMatchCountdown(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float TimeRemaining, int32 PlayerCount, int32 MinPlayers)
{
	if (!HUD || !Canvas || !Font) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float CX = W * 0.5f;
	const float Time = HUD->GetWorld()->GetTimeSeconds();
	const bool bEnough = (PlayerCount >= MinPlayers);

	// Dark overlay
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f), 0.f, 0.f, W, H);

	// Subtle scan lines
	for (float Y = FMath::Fmod(Time * 8.f, 4.f); Y < H; Y += 4.f)
		HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.02f), 0.f, Y, W, 1.f);

	if (bEnough && TimeRemaining > 0.f)
	{
		// "MATCH STARTING IN" header
		const FString Header = TEXT("MATCH STARTING IN");
		float HW, HH;
		HUD->GetTextSize(Header, HW, HH, Font, 1.3f);
		float HX = (W - HW) * 0.5f;
		float HY = H * 0.28f;

		HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f),
			HX - 20.f, HY - 6.f, HW + 40.f, HH + 12.f);
		// Accent line under header
		HUD->DrawRect(FLinearColor(0.f, 0.5f, 0.8f, 0.4f),
			HX - 20.f, HY + HH + 6.f, HW + 40.f, 2.f);
		HUD->DrawText(Header, FLinearColor(0.75f, 0.85f, 0.95f, 1.f),
			HX, HY, Font, 1.3f);

		// Big countdown number with pulse on each tick
		int32 Seconds = FMath::CeilToInt(FMath::Max(TimeRemaining, 0.f));
		FString NumText = FString::Printf(TEXT("%d"), Seconds);
		float Frac = TimeRemaining - FMath::FloorToFloat(TimeRemaining);
		float ScalePulse = 1.f + FMath::Clamp(1.f - Frac * 3.f, 0.f, 1.f) * 0.3f;
		float NumScale = 4.f * ScalePulse;
		float NW, NH;
		HUD->GetTextSize(NumText, NW, NH, Font, NumScale);
		float NX = (W - NW) * 0.5f;
		float NY = HY + HH + 25.f;

		FLinearColor NumColor;
		if (Seconds <= 1)      NumColor = FLinearColor(1.f, 0.25f, 0.15f, 1.f);
		else if (Seconds <= 3) NumColor = FLinearColor(1.f, 0.65f, 0.15f, 1.f);
		else                   NumColor = FLinearColor(0.85f, 0.92f, 1.f, 1.f);

		// Glow behind number
		HUD->DrawRect(FLinearColor(NumColor.R * 0.3f, NumColor.G * 0.3f, NumColor.B * 0.3f, 0.1f),
			NX - 20.f, NY - 10.f, NW + 40.f, NH + 20.f);
		// Shadow
		HUD->DrawText(NumText, FLinearColor(0.f, 0.f, 0.f, 0.4f),
			NX + 2.f, NY + 2.f, Font, NumScale);
		HUD->DrawText(NumText, NumColor, NX, NY, Font, NumScale);

		// Circular arc timer around number
		float ArcR = NH * 0.6f;
		float ArcCX = CX;
		float ArcCY = NY + NH * 0.5f;
		int32 Segs = 24;
		float FillRatio = 1.f - (Frac); // Fills up as second completes
		int32 FilledSegs = FMath::RoundToInt32(FillRatio * Segs);
		for (int32 s = 0; s < Segs; s++)
		{
			float A1 = -PI * 0.5f + (2.f * PI * s) / Segs;
			float A2 = -PI * 0.5f + (2.f * PI * (s + 1)) / Segs;
			FLinearColor SC = (s < FilledSegs)
				? FLinearColor(NumColor.R, NumColor.G, NumColor.B, 0.4f)
				: FLinearColor(0.3f, 0.3f, 0.35f, 0.15f);
			HUD->DrawLine(
				ArcCX + FMath::Cos(A1) * ArcR, ArcCY + FMath::Sin(A1) * ArcR,
				ArcCX + FMath::Cos(A2) * ArcR, ArcCY + FMath::Sin(A2) * ArcR,
				SC, 2.f);
		}

		// Player count
		FString PText = FString::Printf(TEXT("%d players ready"), PlayerCount);
		float PW, PH;
		HUD->GetTextSize(PText, PW, PH, Font, 1.f);
		HUD->DrawText(PText, FLinearColor(0.5f, 0.65f, 0.8f, 0.85f),
			(W - PW) * 0.5f, NY + NH + 30.f, Font, 1.f);
	}
	else
	{
		// Waiting for players
		int32 Dots = (FMath::FloorToInt(Time * 2.f) % 4);
		FString DotStr;
		for (int32 d = 0; d < Dots; d++) DotStr += TEXT(".");
		FString WaitText = FString::Printf(TEXT("WAITING FOR PLAYERS%s"), *DotStr);

		float WW, WH;
		HUD->GetTextSize(WaitText, WW, WH, Font, 1.8f);
		float WX = (W - WW) * 0.5f;
		float WY = H * 0.36f;

		float Pulse = 0.7f + 0.3f * FMath::Abs(FMath::Sin(Time * 2.f));
		FLinearColor WaitColor(0.7f, 0.82f, 0.95f, Pulse);

		HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f),
			WX - 25.f, WY - 10.f, WW + 50.f, WH + 20.f);
		HUD->DrawText(WaitText, WaitColor, WX, WY, Font, 1.8f);

		// Loading spinner dots
		float SpinR = 15.f;
		float SpinY = WY + WH + 35.f;
		for (int32 i = 0; i < 8; i++)
		{
			float Angle = (2.f * PI * i) / 8.f + Time * 3.f;
			float DX = CX + FMath::Cos(Angle) * SpinR;
			float DY = SpinY + FMath::Sin(Angle) * SpinR;
			float Alpha = 0.15f + 0.6f * ((float)i / 8.f);
			HUD->DrawRect(FLinearColor(0.f, 0.6f, 1.f, Alpha),
				DX - 1.5f, DY - 1.5f, 3.f, 3.f);
		}

		// Player count
		FString PText = FString::Printf(TEXT("Players: %d / %d required"),
			PlayerCount, MinPlayers);
		float PW, PH;
		HUD->GetTextSize(PText, PW, PH, Font, 1.f);
		HUD->DrawText(PText, FLinearColor(0.55f, 0.65f, 0.75f, 0.8f),
			(W - PW) * 0.5f, SpinY + 30.f, Font, 1.f);
	}
}

void FExoCountdown::DrawDropPhaseHUD(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float TimeRemaining)
{
	if (!HUD || !Canvas || !Font) return;

	const float W = Canvas->SizeX;
	const float CX = W * 0.5f;

	// "DEPLOYING IN" header
	const FString Header = TEXT("DEPLOYING IN");
	float HW, HH;
	HUD->GetTextSize(Header, HW, HH, Font, 1.3f);
	float HX = (W - HW) * 0.5f;
	float HY = Canvas->SizeY * 0.2f;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f),
		HX - 18.f, HY - 5.f, HW + 36.f, HH + 10.f);
	// Cyan accent line
	HUD->DrawRect(FLinearColor(0.f, 0.6f, 1.f, 0.5f),
		HX - 18.f, HY + HH + 5.f, HW + 36.f, 2.f);
	HUD->DrawText(Header, FLinearColor(0.2f, 0.8f, 1.f, 1.f),
		HX, HY, Font, 1.3f);

	// Countdown number
	int32 Seconds = FMath::CeilToInt(FMath::Max(TimeRemaining, 0.f));
	FString NumText = FString::Printf(TEXT("%d"), Seconds);
	float Frac = TimeRemaining - FMath::FloorToFloat(TimeRemaining);
	float ScalePulse = 1.f + FMath::Clamp(1.f - Frac * 3.f, 0.f, 1.f) * 0.2f;
	float NumScale = 3.f * ScalePulse;
	float NW, NH;
	HUD->GetTextSize(NumText, NW, NH, Font, NumScale);
	float NX = (W - NW) * 0.5f;
	float NY = HY + HH + 18.f;

	FLinearColor NumColor = (Seconds <= 3)
		? FLinearColor(1.f, 0.65f, 0.15f, 1.f)
		: FLinearColor(0.25f, 0.85f, 1.f, 1.f);

	HUD->DrawText(NumText, FLinearColor(0.f, 0.f, 0.f, 0.35f),
		NX + 2.f, NY + 2.f, Font, NumScale);
	HUD->DrawText(NumText, NumColor, NX, NY, Font, NumScale);

	// "Choose your landing zone"
	const FString Sub = TEXT("Choose your landing zone");
	float SW, SH;
	HUD->GetTextSize(Sub, SW, SH, Font, 1.f);
	HUD->DrawText(Sub, FLinearColor(0.55f, 0.7f, 0.85f, 0.85f),
		(W - SW) * 0.5f, NY + NH + 22.f, Font, 1.f);
}

void FExoCountdown::DrawMatchStartBanner(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float MatchTime)
{
	if (!HUD || !Canvas || !Font) return;
	if (MatchTime >= GoBannerDuration) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	float FadeStart = GoBannerDuration - 0.8f;
	float Alpha = (MatchTime > FadeStart)
		? FMath::Clamp(1.f - (MatchTime - FadeStart) / 0.8f, 0.f, 1.f)
		: 1.f;

	float ScaleT = FMath::Clamp(MatchTime / 0.3f, 0.f, 1.f);
	float Scale = FMath::Lerp(1.5f, 1.f, ScaleT);

	float FontScale = 4.f * Scale;
	const FString GoText = TEXT("GO!");
	float TW, TH;
	HUD->GetTextSize(GoText, TW, TH, Font, FontScale);
	float X = (W - TW) * 0.5f;
	float Y = (H - TH) * 0.5f - 40.f;

	FLinearColor GoColor(0.15f, 1.f, 0.35f, Alpha);

	// Glow background
	HUD->DrawRect(FLinearColor(0.f, 0.3f, 0.1f, 0.15f * Alpha),
		X - 40.f, Y - 15.f, TW + 80.f, TH + 30.f);
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f * Alpha),
		X - 25.f, Y - 8.f, TW + 50.f, TH + 16.f);

	// Shadow
	HUD->DrawText(GoText, FLinearColor(0.f, 0.f, 0.f, Alpha * 0.5f),
		X + 3.f, Y + 3.f, Font, FontScale);
	HUD->DrawText(GoText, GoColor, X, Y, Font, FontScale);

	// Horizontal accent lines
	float LineY = Y + TH + 10.f;
	FLinearColor LineCol(0.1f, 0.8f, 0.3f, 0.4f * Alpha);
	HUD->DrawLine(X - 60.f, LineY, X - 10.f, LineY, LineCol, 1.5f);
	HUD->DrawLine(X + TW + 10.f, LineY, X + TW + 60.f, LineY, LineCol, 1.5f);
}
