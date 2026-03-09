#include "UI/ExoMenuBackground.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

// ---------------------------------------------------------------------------
// Animated background — pulsing dark gradient with vignette
// ---------------------------------------------------------------------------

void FExoMenuBackground::DrawBackground(AHUD* HUD, UCanvas* Canvas, float Time)
{
	if (!HUD || !Canvas) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Base dark background with subtle blue tint pulsing
	float Pulse = 0.02f + 0.008f * FMath::Sin(Time * 0.4f);
	FLinearColor BaseBg(Pulse, Pulse, Pulse * 1.8f, 1.0f);
	HUD->DrawRect(BaseBg, 0.f, 0.f, W, H);

	// Subtle blue radial glow at center-top
	float GlowAlpha = 0.06f + 0.02f * FMath::Sin(Time * 0.6f);
	float GlowW = W * 0.6f;
	float GlowH = H * 0.35f;
	FLinearColor GlowCol(0.0f, 0.15f, 0.3f, GlowAlpha);
	HUD->DrawRect(GlowCol, (W - GlowW) * 0.5f, 0.f, GlowW, GlowH);

	// Vignette: darker edges
	float EdgeAlpha = 0.35f;
	float EdgeSize = W * 0.15f;
	FLinearColor EdgeCol(0.f, 0.f, 0.f, EdgeAlpha);
	HUD->DrawRect(EdgeCol, 0.f, 0.f, EdgeSize, H);
	HUD->DrawRect(EdgeCol, W - EdgeSize, 0.f, EdgeSize, H);
	HUD->DrawRect(EdgeCol, 0.f, 0.f, W, H * 0.08f);
	HUD->DrawRect(EdgeCol, 0.f, H * 0.92f, W, H * 0.08f);
}

// ---------------------------------------------------------------------------
// Scan lines overlay
// ---------------------------------------------------------------------------

void FExoMenuBackground::DrawScanLines(AHUD* HUD, UCanvas* Canvas, float Time)
{
	if (!HUD || !Canvas) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	FLinearColor LineCol(0.0f, 0.0f, 0.0f, 0.04f);
	float LineSpacing = 4.f;
	float Offset = FMath::Fmod(Time * 15.f, LineSpacing);

	for (float Y = Offset; Y < H; Y += LineSpacing)
	{
		HUD->DrawRect(LineCol, 0.f, Y, W, 1.f);
	}

	// Single bright scan line that sweeps slowly
	float SweepY = FMath::Fmod(Time * 40.f, H + 60.f) - 30.f;
	float SweepAlpha = 0.06f + 0.03f * FMath::Sin(Time * 2.f);
	FLinearColor SweepCol(0.0f, 0.6f, 1.0f, SweepAlpha);
	HUD->DrawRect(SweepCol, 0.f, SweepY, W, 2.f);
	FLinearColor SweepGlow(0.0f, 0.3f, 0.5f, SweepAlpha * 0.4f);
	HUD->DrawRect(SweepGlow, 0.f, SweepY - 8.f, W, 18.f);
}

// ---------------------------------------------------------------------------
// Title — "EXORIFT" + accent lines + "BATTLE ROYALE"
// ---------------------------------------------------------------------------

void FExoMenuBackground::DrawTitle(AHUD* HUD, UCanvas* Canvas, UFont* Font, float Time)
{
	if (!HUD || !Canvas || !Font) return;

	const float W = Canvas->SizeX;
	const FLinearColor ColorCyanBright(0.4f, 0.95f, 1.0f, 1.0f);
	const FLinearColor ColorWhite(0.92f, 0.94f, 0.97f, 1.0f);
	const FLinearColor ColorGrey(0.55f, 0.55f, 0.6f, 1.0f);

	// --- EXORIFT — large title with glow ---
	FString Title = TEXT("EXORIFT");
	float TitleScale = 3.5f;
	float TW, TH;
	HUD->GetTextSize(Title, TW, TH, Font, TitleScale);
	float TX = (W - TW) * 0.5f;
	float TY = Canvas->SizeY * 0.12f;

	// Glow layers behind the title (pulsing cyan)
	float GlowPulse = 0.5f + 0.3f * FMath::Sin(Time * 1.5f);
	FLinearColor GlowOuter(0.0f, 0.6f, 1.0f, 0.08f * GlowPulse);
	HUD->DrawRect(GlowOuter, TX - 30.f, TY - 10.f, TW + 60.f, TH + 20.f);
	FLinearColor GlowInner(0.0f, 0.8f, 1.0f, 0.12f * GlowPulse);
	HUD->DrawRect(GlowInner, TX - 10.f, TY - 3.f, TW + 20.f, TH + 6.f);

	// Title text — bright white with cyan tint
	FLinearColor TitleColor = FMath::Lerp(ColorWhite, ColorCyanBright,
		0.15f + 0.1f * FMath::Sin(Time * 1.2f));
	HUD->DrawText(Title, TitleColor, TX, TY, Font, TitleScale);

	// Horizontal accent lines flanking the title
	float LineY = TY + TH + 8.f;
	float LineAlpha = 0.4f + 0.15f * FMath::Sin(Time * 1.8f);
	FLinearColor LineCol(0.0f, 0.7f, 1.0f, LineAlpha);
	float LineLen = 120.f;
	HUD->DrawLine(TX - LineLen - 20.f, LineY, TX - 20.f, LineY, LineCol, 1.f);
	HUD->DrawLine(TX + TW + 20.f, LineY, TX + TW + LineLen + 20.f, LineY, LineCol, 1.f);

	// --- Subtitle — "BATTLE ROYALE" ---
	FString Subtitle = TEXT("BATTLE ROYALE");
	float SubScale = 1.2f;
	float SW, SH;
	HUD->GetTextSize(Subtitle, SW, SH, Font, SubScale);
	float SX = (W - SW) * 0.5f;
	float SY = LineY + 10.f;
	HUD->DrawText(Subtitle, ColorGrey, SX, SY, Font, SubScale);
}
