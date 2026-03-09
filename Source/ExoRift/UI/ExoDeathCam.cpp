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
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f * Alpha), 0.f, 0.f, W, H);

	// Red edge vignette
	float EdgeSize = W * 0.15f;
	FLinearColor VigCol(0.6f, 0.f, 0.f, 0.3f * Alpha);
	HUD->DrawRect(VigCol, 0.f, 0.f, EdgeSize, H);
	HUD->DrawRect(VigCol, W - EdgeSize, 0.f, EdgeSize, H);
	HUD->DrawRect(VigCol, 0.f, 0.f, W, EdgeSize * 0.4f);
	HUD->DrawRect(VigCol, 0.f, H - EdgeSize * 0.4f, W, EdgeSize * 0.4f);

	// Horizontal line accent
	float LineAlpha = FMath::Clamp((ElapsedTime - 0.2f) / 0.3f, 0.f, 1.f);
	float LineY = H * 0.22f;
	HUD->DrawRect(FLinearColor(0.8f, 0.1f, 0.1f, 0.4f * LineAlpha),
		W * 0.15f, LineY, W * 0.7f, 2.f);
	HUD->DrawRect(FLinearColor(0.8f, 0.1f, 0.1f, 0.4f * LineAlpha),
		W * 0.15f, H * 0.58f, W * 0.7f, 2.f);
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

	// Glow behind text
	HUD->DrawRect(FLinearColor(0.5f, 0.f, 0.f, 0.15f * Alpha),
		X - 30.f, Y - 10.f, TW + 60.f, TH + 20.f);

	// Shadow
	HUD->DrawText(Text, FLinearColor(0.f, 0.f, 0.f, Alpha * 0.5f),
		X + 2.f, Y + 2.f, Font, Scale);

	FLinearColor Col(0.9f, 0.15f, 0.15f, Alpha);
	HUD->DrawText(Text, Col, X, Y, Font, Scale);
}

void FExoDeathCam::DrawKillInfo(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay - 0.2f) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	// "Killed by" line
	FString KilledBy = TEXT("Killed by");
	float KBW, KBH;
	HUD->GetTextSize(KilledBy, KBW, KBH, Font, 0.9f);
	float CX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.35f;
	HUD->DrawText(KilledBy, FLinearColor(0.5f, 0.5f, 0.55f, Alpha),
		CX - KBW * 0.5f, Y, Font, 0.9f);

	// Killer name (larger, white)
	float TW, TH;
	HUD->GetTextSize(KillerName, TW, TH, Font, 1.4f);
	float NY = Y + KBH + 4.f;
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.3f * Alpha),
		CX - TW * 0.5f - 15.f, NY - 4.f, TW + 30.f, TH + 8.f);
	HUD->DrawText(KillerName, FLinearColor(0.95f, 0.95f, 1.f, Alpha),
		CX - TW * 0.5f, NY, Font, 1.4f);

	// Weapon name
	FString WeaponBracket = FString::Printf(TEXT("with %s"), *WeaponName);
	float WW, WH;
	HUD->GetTextSize(WeaponBracket, WW, WH, Font, 0.85f);
	float WY = NY + TH + 6.f;
	HUD->DrawText(WeaponBracket, FLinearColor(0.6f, 0.65f, 0.7f, Alpha),
		CX - WW * 0.5f, WY, Font, 0.85f);
}

void FExoDeathCam::DrawPlacement(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay - 0.5f) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	float CX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.48f;

	// Placement number (large)
	FString PlaceNum = FString::Printf(TEXT("#%d"), Placement);
	float PW, PH;
	HUD->GetTextSize(PlaceNum, PW, PH, Font, 2.2f);

	// Gold for top 3, silver for top 10, white otherwise
	FLinearColor PlaceColor;
	if (Placement <= 1) PlaceColor = FLinearColor(1.f, 0.85f, 0.2f, Alpha);
	else if (Placement <= 3) PlaceColor = FLinearColor(1.f, 0.7f, 0.3f, Alpha);
	else if (Placement <= 10) PlaceColor = FLinearColor(0.8f, 0.82f, 0.85f, Alpha);
	else PlaceColor = FLinearColor(0.6f, 0.62f, 0.65f, Alpha);

	HUD->DrawText(PlaceNum, PlaceColor, CX - PW * 0.5f, Y, Font, 2.2f);

	// "out of X" sub-text
	FString OfText = FString::Printf(TEXT("out of %d"), TotalPlayers);
	float OW, OH;
	HUD->GetTextSize(OfText, OW, OH, Font, 0.85f);
	HUD->DrawText(OfText, FLinearColor(0.5f, 0.52f, 0.55f, Alpha),
		CX - OW * 0.5f, Y + PH + 2.f, Font, 0.85f);
}

void FExoDeathCam::DrawStats(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - StatsDelay) / 0.5f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	// Display cached stats if available
	float CX = Canvas->SizeX * 0.5f;
	float BaseY = Canvas->SizeY * 0.62f;
	float ColSpacing = 140.f;

	// Stat labels and values
	struct FStat { FString Label; FString Value; };
	TArray<FStat> Stats;
	Stats.Add({TEXT("Kills"), FString::Printf(TEXT("%d"), CachedKills)});
	Stats.Add({TEXT("Damage"), FString::Printf(TEXT("%d"), CachedDamage)});
	Stats.Add({TEXT("Accuracy"), FString::Printf(TEXT("%d%%"), CachedAccuracy)});
	Stats.Add({TEXT("Survived"), SurvivalTime});

	float TotalW = (Stats.Num() - 1) * ColSpacing;
	float StartX = CX - TotalW * 0.5f;

	// Background panel
	HUD->DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.6f * Alpha),
		StartX - 30.f, BaseY - 10.f, TotalW + 60.f, 60.f);

	// Horizontal separator above
	HUD->DrawRect(FLinearColor(0.3f, 0.3f, 0.35f, 0.4f * Alpha),
		StartX - 20.f, BaseY - 10.f, TotalW + 40.f, 1.f);

	for (int32 i = 0; i < Stats.Num(); i++)
	{
		float X = StartX + i * ColSpacing;

		// Label
		float LW, LH;
		HUD->GetTextSize(Stats[i].Label, LW, LH, Font, 0.7f);
		HUD->DrawText(Stats[i].Label, FLinearColor(0.5f, 0.55f, 0.6f, Alpha),
			X - LW * 0.5f, BaseY, Font, 0.7f);

		// Value
		float VW, VH;
		HUD->GetTextSize(Stats[i].Value, VW, VH, Font, 1.1f);
		HUD->DrawText(Stats[i].Value, FLinearColor(0.9f, 0.92f, 0.95f, Alpha),
			X - VW * 0.5f, BaseY + LH + 4.f, Font, 1.1f);
	}
}

void FExoDeathCam::DrawSpectatePrompt(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - PromptDelay) / 0.5f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	// Pulsing prompt
	float Pulse = 0.6f + 0.4f * FMath::Abs(FMath::Sin(ElapsedTime * 2.f));
	const FString Prompt = TEXT("Press SPACE to spectate  |  Press ESC for menu");
	float TW, TH;
	HUD->GetTextSize(Prompt, TW, TH, Font, 0.9f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.78f;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.3f * Alpha),
		X - 15.f, Y - 4.f, TW + 30.f, TH + 8.f);
	HUD->DrawText(Prompt, FLinearColor(0.6f, 0.65f, 0.7f, Alpha * Pulse),
		X, Y, Font, 0.9f);
}
