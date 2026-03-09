#include "UI/ExoLoadingScreen.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

// ---- Static tips array ----
const FString FExoLoadingScreen::Tips[] =
{
	TEXT("Stay inside the zone to avoid damage"),
	TEXT("Overheat your weapon? Wait for it to cool down"),
	TEXT("Higher rarity weapons deal more damage"),
	TEXT("DBNO teammates can be revived \u2014 hold E"),
	TEXT("Use the dash ability to escape danger"),
	TEXT("Supply drops contain Epic and Legendary weapons"),
	TEXT("Headshots deal bonus damage"),
	TEXT("Seek cover when your health is low")
};
const int32 FExoLoadingScreen::TipCount = UE_ARRAY_COUNT(Tips);

FExoLoadingScreen::FExoLoadingScreen()
{
	TipIndex = FMath::RandRange(0, TipCount - 1);
}

void FExoLoadingScreen::Activate()
{
	ElapsedTime = 0.f;
	TipIndex = FMath::RandRange(0, TipCount - 1);
}

void FExoLoadingScreen::Tick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
}

void FExoLoadingScreen::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!HUD || !Canvas || !Font) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Full-screen black background
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 1.f), 0.f, 0.f, W, H);

	// Subtle blue glow at top
	float Pulse = 0.04f + 0.02f * FMath::Sin(ElapsedTime * 0.5f);
	HUD->DrawRect(FLinearColor(0.f, 0.1f, 0.25f, Pulse),
		W * 0.2f, 0.f, W * 0.6f, H * 0.3f);

	DrawLogo(HUD, Canvas, Font);
	DrawProgressBar(HUD, Canvas);
	DrawTip(HUD, Canvas, Font);
}

void FExoLoadingScreen::DrawLogo(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	const float W = Canvas->SizeX;

	// "EXORIFT" title — same style as ExoMenuBackground
	const FString Title = TEXT("EXORIFT");
	const float TitleScale = 3.5f;
	float TW, TH;
	HUD->GetTextSize(Title, TW, TH, Font, TitleScale);
	float TX = (W - TW) * 0.5f;
	float TY = Canvas->SizeY * 0.2f;

	// Glow behind title
	float GlowPulse = 0.5f + 0.3f * FMath::Sin(ElapsedTime * 1.5f);
	HUD->DrawRect(FLinearColor(0.f, 0.6f, 1.f, 0.1f * GlowPulse),
		TX - 20.f, TY - 6.f, TW + 40.f, TH + 12.f);

	FLinearColor TitleCol = FLinearColor(0.85f, 0.95f, 1.f, 1.f);
	HUD->DrawText(Title, TitleCol, TX, TY, Font, TitleScale);

	// "LOADING..." subtitle
	const FString Loading = TEXT("LOADING...");
	float LW, LH;
	HUD->GetTextSize(Loading, LW, LH, Font, 1.2f);
	float LAlpha = 0.5f + 0.5f * FMath::Abs(FMath::Sin(ElapsedTime * 2.f));
	HUD->DrawText(Loading, FLinearColor(0.6f, 0.7f, 0.8f, LAlpha),
		(W - LW) * 0.5f, TY + TH + 20.f, Font, 1.2f);
}

void FExoLoadingScreen::DrawProgressBar(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float BarW = W * 0.4f;
	const float BarH = 6.f;
	const float BarX = (W - BarW) * 0.5f;
	const float BarY = H * 0.6f;

	// Background
	HUD->DrawRect(FLinearColor(0.1f, 0.1f, 0.12f, 0.8f), BarX, BarY, BarW, BarH);

	// Pulsing fill that slides back and forth
	float Phase = FMath::Fmod(ElapsedTime * 0.6f, 1.f);
	float FillStart = Phase * 0.7f;
	float FillLen = 0.3f;
	HUD->DrawRect(FLinearColor(0.f, 0.7f, 1.f, 0.9f),
		BarX + BarW * FillStart, BarY + 1.f,
		BarW * FillLen, BarH - 2.f);
}

void FExoLoadingScreen::DrawTip(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	FString TipText = FString::Printf(TEXT("TIP: %s"), *Tips[TipIndex]);
	float TW, TH;
	HUD->GetTextSize(TipText, TW, TH, Font, 0.9f);

	HUD->DrawText(TipText, FLinearColor(0.5f, 0.55f, 0.6f, 0.9f),
		(W - TW) * 0.5f, H * 0.75f, Font, 0.9f);
}
