// ExoDeathCam.cpp — Death screen overlay with elimination info and stats
#include "UI/ExoDeathCam.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

void FExoDeathCam::Init(const FString& InKillerName, const FString& InWeapon,
	int32 InPlace, int32 InTotal)
{
	KillerName = InKillerName;
	WeaponName = InWeapon;
	Placement = InPlace;
	TotalPlayers = InTotal;
	ElapsedTime = 0.f;
	bActive = true;
}

void FExoDeathCam::Tick(float DeltaTime)
{
	if (bActive) ElapsedTime += DeltaTime;
}

void FExoDeathCam::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!bActive || !HUD || !Canvas || !Font) return;
	DrawVignette(HUD, Canvas);
	DrawEliminatedText(HUD, Canvas, Font);
	DrawKillInfo(HUD, Canvas, Font);
	DrawPlacement(HUD, Canvas, Font);
	DrawStats(HUD, Canvas, Font);
	DrawSpectatePrompt(HUD, Canvas, Font);
}

void FExoDeathCam::DrawVignette(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	float Alpha = FMath::Clamp(ElapsedTime / VignetteFadeIn, 0.f, 1.f);

	// Full screen darken
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f * Alpha), 0.f, 0.f, W, H);

	// Red edge vignette (pulsing)
	float Pulse = 0.8f + 0.2f * FMath::Sin(ElapsedTime * 2.f);
	float EdgeSize = W * 0.14f;
	FLinearColor VigCol(0.5f, 0.f, 0.f, 0.25f * Alpha * Pulse);
	HUD->DrawRect(VigCol, 0.f, 0.f, EdgeSize, H);
	HUD->DrawRect(VigCol, W - EdgeSize, 0.f, EdgeSize, H);
	HUD->DrawRect(VigCol, 0.f, 0.f, W, EdgeSize * 0.35f);
	HUD->DrawRect(VigCol, 0.f, H - EdgeSize * 0.35f, W, EdgeSize * 0.35f);

	// Horizontal accent lines
	float LineAlpha = FMath::Clamp((ElapsedTime - 0.2f) / 0.3f, 0.f, 1.f);
	FLinearColor LineCol(0.7f, 0.1f, 0.1f, 0.35f * LineAlpha);
	float LineY1 = H * 0.22f;
	float LineY2 = H * 0.58f;
	HUD->DrawRect(LineCol, W * 0.1f, LineY1, W * 0.8f, 2.f);
	HUD->DrawRect(LineCol, W * 0.1f, LineY2, W * 0.8f, 2.f);

	// Corner brackets
	float BLen = 40.f;
	float Margin = W * 0.12f;
	float MTop = LineY1 - 10.f;
	float MBot = LineY2 + 12.f;
	FLinearColor BCol(0.7f, 0.15f, 0.15f, 0.3f * LineAlpha);
	HUD->DrawLine(Margin, MTop, Margin + BLen, MTop, BCol, 1.5f);
	HUD->DrawLine(Margin, MTop, Margin, MTop + BLen, BCol, 1.5f);
	HUD->DrawLine(W - Margin, MTop, W - Margin - BLen, MTop, BCol, 1.5f);
	HUD->DrawLine(W - Margin, MTop, W - Margin, MTop + BLen, BCol, 1.5f);
	HUD->DrawLine(Margin, MBot, Margin + BLen, MBot, BCol, 1.5f);
	HUD->DrawLine(Margin, MBot, Margin, MBot - BLen, BCol, 1.5f);
	HUD->DrawLine(W - Margin, MBot, W - Margin - BLen, MBot, BCol, 1.5f);
	HUD->DrawLine(W - Margin, MBot, W - Margin, MBot - BLen, BCol, 1.5f);

	// Subtle scan lines
	FLinearColor ScanCol(0.f, 0.f, 0.f, 0.02f * Alpha);
	for (float Y = 0.f; Y < H; Y += 4.f)
	{
		HUD->DrawRect(ScanCol, 0.f, Y, W, 1.f);
	}
}

void FExoDeathCam::DrawEliminatedText(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	const FString Text = TEXT("ELIMINATED");
	const float Scale = 2.8f;
	float TW, TH;
	HUD->GetTextSize(Text, TW, TH, Font, Scale);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.25f;

	// Multi-layer glow
	HUD->DrawRect(FLinearColor(0.4f, 0.f, 0.f, 0.1f * Alpha),
		X - 40.f, Y - 12.f, TW + 80.f, TH + 24.f);
	HUD->DrawRect(FLinearColor(0.5f, 0.f, 0.f, 0.15f * Alpha),
		X - 15.f, Y - 4.f, TW + 30.f, TH + 8.f);

	// Shadow
	HUD->DrawText(Text, FLinearColor(0.f, 0.f, 0.f, Alpha * 0.6f),
		X + 2.f, Y + 2.f, Font, Scale);

	// Main text with subtle pulse
	float TPulse = 0.85f + 0.15f * FMath::Sin(ElapsedTime * 1.5f);
	HUD->DrawText(Text, FLinearColor(0.9f, 0.12f, 0.12f, Alpha * TPulse),
		X, Y, Font, Scale);
}

void FExoDeathCam::DrawKillInfo(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay - 0.2f) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	float CX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.35f;

	FString KilledBy = TEXT("Killed by");
	float KBW, KBH;
	HUD->GetTextSize(KilledBy, KBW, KBH, Font, 0.9f);
	HUD->DrawText(KilledBy, FLinearColor(0.5f, 0.5f, 0.55f, Alpha),
		CX - KBW * 0.5f, Y, Font, 0.9f);

	// Killer name with panel background
	float TW, TH;
	HUD->GetTextSize(KillerName, TW, TH, Font, 1.5f);
	float NY = Y + KBH + 6.f;
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f * Alpha),
		CX - TW * 0.5f - 18.f, NY - 5.f, TW + 36.f, TH + 10.f);
	// Left accent
	HUD->DrawRect(FLinearColor(0.8f, 0.15f, 0.15f, 0.6f * Alpha),
		CX - TW * 0.5f - 18.f, NY - 5.f, 3.f, TH + 10.f);
	HUD->DrawText(KillerName, FLinearColor(0.95f, 0.95f, 1.f, Alpha),
		CX - TW * 0.5f, NY, Font, 1.5f);

	// Weapon
	FString WText = FString::Printf(TEXT("with %s"), *WeaponName);
	float WW, WH;
	HUD->GetTextSize(WText, WW, WH, Font, 0.85f);
	HUD->DrawText(WText, FLinearColor(0.55f, 0.6f, 0.65f, Alpha),
		CX - WW * 0.5f, NY + TH + 8.f, Font, 0.85f);
}

void FExoDeathCam::DrawPlacement(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay - 0.5f) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	float CX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.49f;

	FString PlaceNum = FString::Printf(TEXT("#%d"), Placement);
	float PW, PH;
	HUD->GetTextSize(PlaceNum, PW, PH, Font, 2.5f);

	FLinearColor PlaceColor;
	if (Placement <= 1)      PlaceColor = FLinearColor(1.f, 0.85f, 0.2f, Alpha);
	else if (Placement <= 3) PlaceColor = FLinearColor(1.f, 0.7f, 0.3f, Alpha);
	else if (Placement <= 10) PlaceColor = FLinearColor(0.78f, 0.8f, 0.85f, Alpha);
	else                     PlaceColor = FLinearColor(0.6f, 0.62f, 0.65f, Alpha);

	// Shadow
	HUD->DrawText(PlaceNum, FLinearColor(0.f, 0.f, 0.f, Alpha * 0.4f),
		CX - PW * 0.5f + 2.f, Y + 2.f, Font, 2.5f);
	HUD->DrawText(PlaceNum, PlaceColor, CX - PW * 0.5f, Y, Font, 2.5f);

	FString OfText = FString::Printf(TEXT("out of %d"), TotalPlayers);
	float OW, OH;
	HUD->GetTextSize(OfText, OW, OH, Font, 0.85f);
	HUD->DrawText(OfText, FLinearColor(0.5f, 0.52f, 0.55f, Alpha),
		CX - OW * 0.5f, Y + PH + 4.f, Font, 0.85f);
}

void FExoDeathCam::DrawStats(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - StatsDelay) / 0.5f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	float CX = Canvas->SizeX * 0.5f;
	float BaseY = Canvas->SizeY * 0.63f;
	float ColSpacing = 140.f;

	struct FStat { FString Label; FString Value; };
	TArray<FStat> Stats;
	Stats.Add({TEXT("Kills"), FString::Printf(TEXT("%d"), CachedKills)});
	Stats.Add({TEXT("Damage"), FString::Printf(TEXT("%d"), CachedDamage)});
	Stats.Add({TEXT("Accuracy"), FString::Printf(TEXT("%d%%"), CachedAccuracy)});
	Stats.Add({TEXT("Survived"), SurvivalTime});

	float TotalW = (Stats.Num() - 1) * ColSpacing;
	float StartX = CX - TotalW * 0.5f;

	// Panel background
	HUD->DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.55f * Alpha),
		StartX - 25.f, BaseY - 12.f, TotalW + 50.f, 62.f);
	// Top accent
	HUD->DrawLine(StartX - 25.f, BaseY - 12.f, StartX + TotalW + 25.f, BaseY - 12.f,
		FLinearColor(0.3f, 0.3f, 0.4f, 0.35f * Alpha), 1.f);

	for (int32 i = 0; i < Stats.Num(); i++)
	{
		float X = StartX + i * ColSpacing;

		float LW, LH;
		HUD->GetTextSize(Stats[i].Label, LW, LH, Font, 0.7f);
		HUD->DrawText(Stats[i].Label, FLinearColor(0.5f, 0.55f, 0.6f, Alpha),
			X - LW * 0.5f, BaseY, Font, 0.7f);

		float VW, VH;
		HUD->GetTextSize(Stats[i].Value, VW, VH, Font, 1.15f);
		HUD->DrawText(Stats[i].Value, FLinearColor(0.9f, 0.92f, 0.95f, Alpha),
			X - VW * 0.5f, BaseY + LH + 5.f, Font, 1.15f);

		// Vertical separator between stats
		if (i < Stats.Num() - 1)
		{
			float SepX = X + ColSpacing * 0.5f;
			HUD->DrawLine(SepX, BaseY + 2.f, SepX, BaseY + LH + VH + 5.f,
				FLinearColor(0.3f, 0.3f, 0.35f, 0.25f * Alpha), 1.f);
		}
	}
}

void FExoDeathCam::DrawSpectatePrompt(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - PromptDelay) / 0.5f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	float Pulse = 0.6f + 0.4f * FMath::Abs(FMath::Sin(ElapsedTime * 2.f));
	const FString Prompt = TEXT("Press SPACE to spectate  |  Press ESC for menu");
	float TW, TH;
	HUD->GetTextSize(Prompt, TW, TH, Font, 0.9f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.79f;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.3f * Alpha),
		X - 15.f, Y - 4.f, TW + 30.f, TH + 8.f);
	HUD->DrawText(Prompt, FLinearColor(0.55f, 0.65f, 0.75f, Alpha * Pulse),
		X, Y, Font, 0.9f);
}
