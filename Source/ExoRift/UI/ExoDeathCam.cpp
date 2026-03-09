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
	DrawSpectatePrompt(HUD, Canvas, Font);
}

void FExoDeathCam::DrawVignette(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	float Alpha = FMath::Clamp(ElapsedTime / VignetteFadeIn, 0.f, 1.f);
	float EdgeSize = W * 0.2f;
	FLinearColor VigCol(0.6f, 0.f, 0.f, 0.35f * Alpha);
	HUD->DrawRect(VigCol, 0.f, 0.f, EdgeSize, H);
	HUD->DrawRect(VigCol, W - EdgeSize, 0.f, EdgeSize, H);
	HUD->DrawRect(VigCol, 0.f, 0.f, W, EdgeSize * 0.5f);
	HUD->DrawRect(VigCol, 0.f, H - EdgeSize * 0.5f, W, EdgeSize * 0.5f);
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.3f * Alpha), 0.f, 0.f, W, H);
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
		X - 20.f, Y - 8.f, TW + 40.f, TH + 16.f);

	FLinearColor Col(0.9f, 0.15f, 0.15f, Alpha);
	HUD->DrawText(Text, Col, X, Y, Font, Scale);
}

void FExoDeathCam::DrawKillInfo(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay - 0.2f) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	FString Info = FString::Printf(TEXT("Killed by %s  [%s]"), *KillerName, *WeaponName);
	float TW, TH;
	HUD->GetTextSize(Info, TW, TH, Font, 1.1f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.35f;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f * Alpha),
		X - 10.f, Y - 4.f, TW + 20.f, TH + 8.f);
	HUD->DrawText(Info, FLinearColor(0.85f, 0.85f, 0.9f, Alpha), X, Y, Font, 1.1f);
}

void FExoDeathCam::DrawPlacement(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - TextFadeDelay - 0.4f) / 0.4f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	FString PlaceText = FString::Printf(TEXT("#%d / %d"), Placement, TotalPlayers);
	float TW, TH;
	HUD->GetTextSize(PlaceText, TW, TH, Font, 1.6f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.42f;

	FLinearColor Col(1.f, 0.85f, 0.3f, Alpha);
	HUD->DrawText(PlaceText, Col, X, Y, Font, 1.6f);
}

void FExoDeathCam::DrawSpectatePrompt(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	float Alpha = FMath::Clamp((ElapsedTime - PromptDelay) / 0.5f, 0.f, 1.f);
	if (Alpha <= 0.f) return;

	// Pulsing prompt
	float Pulse = 0.6f + 0.4f * FMath::Abs(FMath::Sin(ElapsedTime * 2.f));
	const FString Prompt = TEXT("Press SPACE to spectate");
	float TW, TH;
	HUD->GetTextSize(Prompt, TW, TH, Font, 1.f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.55f;

	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.3f * Alpha),
		X - 10.f, Y - 4.f, TW + 20.f, TH + 8.f);
	HUD->DrawText(Prompt, FLinearColor(0.7f, 0.75f, 0.8f, Alpha * Pulse),
		X, Y, Font, 1.f);
}
