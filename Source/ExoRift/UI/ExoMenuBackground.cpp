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

	// Secondary glow at bottom-center (warmer)
	float BotGlowAlpha = 0.03f + 0.01f * FMath::Sin(Time * 0.45f + 1.f);
	float BotGlowW = W * 0.4f;
	float BotGlowH = H * 0.2f;
	HUD->DrawRect(FLinearColor(0.05f, 0.02f, 0.15f, BotGlowAlpha),
		(W - BotGlowW) * 0.5f, H - BotGlowH, BotGlowW, BotGlowH);

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

	// Primary sweep line
	float SweepY = FMath::Fmod(Time * 40.f, H + 60.f) - 30.f;
	float SweepAlpha = 0.06f + 0.03f * FMath::Sin(Time * 2.f);
	FLinearColor SweepCol(0.0f, 0.6f, 1.0f, SweepAlpha);
	HUD->DrawRect(SweepCol, 0.f, SweepY, W, 2.f);
	FLinearColor SweepGlow(0.0f, 0.3f, 0.5f, SweepAlpha * 0.4f);
	HUD->DrawRect(SweepGlow, 0.f, SweepY - 8.f, W, 18.f);

	// Secondary slower sweep (dimmer, opposite direction)
	float Sweep2Y = H - FMath::Fmod(Time * 25.f, H + 80.f) + 40.f;
	float Sweep2Alpha = 0.03f + 0.015f * FMath::Sin(Time * 1.3f);
	HUD->DrawRect(FLinearColor(0.3f, 0.1f, 0.5f, Sweep2Alpha), 0.f, Sweep2Y, W, 1.f);
	HUD->DrawRect(FLinearColor(0.15f, 0.05f, 0.25f, Sweep2Alpha * 0.3f),
		0.f, Sweep2Y - 5.f, W, 11.f);
}

// ---------------------------------------------------------------------------
// Floating particles — small dots drifting upward
// ---------------------------------------------------------------------------

void FExoMenuBackground::DrawParticles(AHUD* HUD, UCanvas* Canvas, float Time)
{
	if (!HUD || !Canvas) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	constexpr int32 NumParticles = 50;

	for (int32 i = 0; i < NumParticles; i++)
	{
		float Seed = i * 137.508f;
		float X = FMath::Fmod(Seed * 7.31f, W);
		float BaseY = FMath::Fmod(Seed * 11.71f, H);
		float Speed = 12.f + (i % 5) * 6.f;
		float Y = FMath::Fmod(BaseY - Time * Speed + H * 10.f, H);

		// Horizontal drift
		float Drift = FMath::Sin(Time * 0.3f + Seed * 0.05f) * 20.f;

		float Alpha = 0.08f + 0.06f * FMath::Sin(Time * 0.5f + Seed * 0.1f);
		float Size = 1.f + (i % 3);

		// Mix blue and cyan particles
		float R = (i % 7 < 2) ? 0.3f : 0.1f;
		float G = (i % 7 < 2) ? 0.15f : 0.5f;
		float B = (i % 7 < 2) ? 0.6f : 0.9f;

		FLinearColor Col(R, G, B, Alpha);
		HUD->DrawRect(Col, X + Drift, Y, Size, Size);

		// Trailing fade
		HUD->DrawRect(FLinearColor(R * 0.5f, G * 0.6f, B * 0.7f, Alpha * 0.25f),
			X + Drift, Y + Size, Size * 0.8f, Size * 3.f);
	}
}

// ---------------------------------------------------------------------------
// Hex grid — subtle tech pattern
// ---------------------------------------------------------------------------

void FExoMenuBackground::DrawHexGrid(AHUD* HUD, UCanvas* Canvas, float Time)
{
	if (!HUD || !Canvas) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float GridSize = 60.f;
	const float RowH = GridSize * 0.866f;

	float Alpha = 0.025f + 0.01f * FMath::Sin(Time * 0.3f);
	FLinearColor LineCol(0.1f, 0.4f, 0.7f, Alpha);

	float OffsetY = FMath::Fmod(Time * 5.f, RowH * 2.f);

	for (float Y = -RowH + OffsetY; Y < H + RowH; Y += RowH)
	{
		int32 Row = FMath::FloorToInt((Y - OffsetY + RowH) / RowH);
		float XOff = (Row % 2 == 0) ? 0.f : GridSize * 0.5f;

		for (float X = -GridSize + XOff; X < W + GridSize; X += GridSize)
		{
			float CX = X;
			float CY = Y;
			float Arm = 6.f;
			HUD->DrawLine(CX - Arm, CY, CX + Arm, CY, LineCol, 0.5f);
			HUD->DrawLine(CX, CY - Arm, CX, CY + Arm, LineCol, 0.5f);

			// Occasional connecting lines for hex feel
			if (Row % 3 == 0 && FMath::Fmod(X, GridSize * 2.f) < GridSize)
			{
				FLinearColor ConnCol(0.1f, 0.35f, 0.6f, Alpha * 0.5f);
				HUD->DrawLine(CX + Arm, CY, CX + GridSize * 0.5f - Arm, CY, ConnCol, 0.3f);
			}
		}
	}
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
	const FLinearColor Shadow(0.f, 0.f, 0.f, 0.5f);

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

	// Shadow text
	HUD->DrawText(Title, Shadow, TX + 2.f, TY + 2.f, Font, TitleScale);

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

	// Diamond terminators on accent lines
	float LD = 4.f;
	float LDX = TX - LineLen - 20.f;
	HUD->DrawLine(LDX, LineY, LDX - LD, LineY - LD, LineCol);
	HUD->DrawLine(LDX - LD, LineY - LD, LDX - LD * 2.f, LineY, LineCol);
	HUD->DrawLine(LDX - LD * 2.f, LineY, LDX - LD, LineY + LD, LineCol);
	HUD->DrawLine(LDX - LD, LineY + LD, LDX, LineY, LineCol);

	float RDX = TX + TW + LineLen + 20.f;
	HUD->DrawLine(RDX, LineY, RDX + LD, LineY - LD, LineCol);
	HUD->DrawLine(RDX + LD, LineY - LD, RDX + LD * 2.f, LineY, LineCol);
	HUD->DrawLine(RDX + LD * 2.f, LineY, RDX + LD, LineY + LD, LineCol);
	HUD->DrawLine(RDX + LD, LineY + LD, RDX, LineY, LineCol);

	// --- Subtitle — "BATTLE ROYALE" with letter spacing feel ---
	FString Subtitle = TEXT("B A T T L E   R O Y A L E");
	float SubScale = 1.0f;
	float SW, SH;
	HUD->GetTextSize(Subtitle, SW, SH, Font, SubScale);
	float SX = (W - SW) * 0.5f;
	float SY = LineY + 12.f;
	HUD->DrawText(Subtitle, Shadow, SX + 1.f, SY + 1.f, Font, SubScale);
	HUD->DrawText(Subtitle, ColorGrey, SX, SY, Font, SubScale);

	// Thin accent lines below subtitle
	float SubLineY = SY + SH + 6.f;
	float SubLineW = SW * 0.4f;
	FLinearColor SubLineCol(0.f, 0.5f, 0.8f, 0.25f);
	HUD->DrawLine((W - SubLineW) * 0.5f, SubLineY,
		(W + SubLineW) * 0.5f, SubLineY, SubLineCol);
}
