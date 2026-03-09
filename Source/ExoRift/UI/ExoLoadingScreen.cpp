// ExoLoadingScreen.cpp — Full-screen loading screen with sci-fi visuals
#include "UI/ExoLoadingScreen.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

const FString FExoLoadingScreen::Tips[] =
{
	TEXT("Stay inside the zone to avoid damage"),
	TEXT("Overheat your weapon? Wait for it to cool down"),
	TEXT("Higher rarity weapons deal more damage"),
	TEXT("DBNO teammates can be revived \u2014 hold E"),
	TEXT("Use the dash ability to escape danger"),
	TEXT("Supply drops contain Epic and Legendary weapons"),
	TEXT("Headshots deal bonus damage"),
	TEXT("Seek cover when your health is low"),
	TEXT("Energy cells restore weapon ammunition"),
	TEXT("Higher ground gives a tactical advantage"),
	TEXT("Watch the minimap for nearby threats"),
	TEXT("The zone deals increasing damage each phase")
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

	// Full-screen dark background
	HUD->DrawRect(FLinearColor(0.01f, 0.01f, 0.02f, 1.f), 0.f, 0.f, W, H);

	// Blue radial glow at center-top
	float Pulse = 0.05f + 0.025f * FMath::Sin(ElapsedTime * 0.5f);
	HUD->DrawRect(FLinearColor(0.f, 0.1f, 0.25f, Pulse),
		W * 0.15f, 0.f, W * 0.7f, H * 0.35f);

	// Bottom blue glow
	float BotPulse = 0.03f + 0.015f * FMath::Sin(ElapsedTime * 0.7f + 1.f);
	HUD->DrawRect(FLinearColor(0.f, 0.05f, 0.15f, BotPulse),
		W * 0.2f, H * 0.7f, W * 0.6f, H * 0.3f);

	DrawHexOverlay(HUD, Canvas);
	DrawScanLines(HUD, Canvas);
	DrawCornerBrackets(HUD, Canvas);
	DrawLogo(HUD, Canvas, Font);
	DrawProgressBar(HUD, Canvas);
	DrawLoadingSpinner(HUD, Canvas);
	DrawTip(HUD, Canvas, Font);
}

void FExoLoadingScreen::DrawLogo(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	const float W = Canvas->SizeX;

	const FString Title = TEXT("EXORIFT");
	const float TitleScale = 3.5f;
	float TW, TH;
	HUD->GetTextSize(Title, TW, TH, Font, TitleScale);
	float TX = (W - TW) * 0.5f;
	float TY = Canvas->SizeY * 0.2f;

	// Multi-layer glow behind title
	float GlowPulse = 0.5f + 0.3f * FMath::Sin(ElapsedTime * 1.5f);
	HUD->DrawRect(FLinearColor(0.f, 0.4f, 0.8f, 0.06f * GlowPulse),
		TX - 40.f, TY - 15.f, TW + 80.f, TH + 30.f);
	HUD->DrawRect(FLinearColor(0.f, 0.6f, 1.f, 0.1f * GlowPulse),
		TX - 15.f, TY - 4.f, TW + 30.f, TH + 8.f);

	FLinearColor TitleCol = FLinearColor(0.85f, 0.95f, 1.f, 1.f);
	HUD->DrawText(Title, TitleCol, TX, TY, Font, TitleScale);

	// Accent lines flanking title
	float LineY = TY + TH + 8.f;
	float LineAlpha = 0.4f + 0.15f * FMath::Sin(ElapsedTime * 1.8f);
	FLinearColor LineCol(0.f, 0.7f, 1.f, LineAlpha);
	HUD->DrawLine(TX - 100.f, LineY, TX - 10.f, LineY, LineCol, 1.f);
	HUD->DrawLine(TX + TW + 10.f, LineY, TX + TW + 100.f, LineY, LineCol, 1.f);

	// "BATTLE ROYALE" subtitle
	const FString Sub = TEXT("BATTLE ROYALE");
	float SW, SH;
	HUD->GetTextSize(Sub, SW, SH, Font, 1.1f);
	HUD->DrawText(Sub, FLinearColor(0.5f, 0.55f, 0.6f, 0.9f),
		(W - SW) * 0.5f, LineY + 10.f, Font, 1.1f);

	// "LOADING..." with animated dots
	int32 Dots = ((int32)(ElapsedTime * 2.f)) % 4;
	FString Loading = TEXT("LOADING");
	for (int32 d = 0; d < Dots; d++) Loading += TEXT(".");
	float LW, LH;
	HUD->GetTextSize(Loading, LW, LH, Font, 1.0f);
	float LAlpha = 0.5f + 0.4f * FMath::Abs(FMath::Sin(ElapsedTime * 2.f));
	HUD->DrawText(Loading, FLinearColor(0.5f, 0.7f, 0.9f, LAlpha),
		(W - LW) * 0.5f, LineY + 10.f + SH + 20.f, Font, 1.0f);
}

void FExoLoadingScreen::DrawProgressBar(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float BarW = W * 0.35f;
	const float BarH = 4.f;
	const float BarX = (W - BarW) * 0.5f;
	const float BarY = H * 0.62f;

	// Background track
	HUD->DrawRect(FLinearColor(0.08f, 0.08f, 0.12f, 0.6f), BarX, BarY, BarW, BarH);

	// Animated fill — two overlapping pulses sliding right
	float Phase1 = FMath::Fmod(ElapsedTime * 0.5f, 1.2f) - 0.1f;
	float Phase2 = FMath::Fmod(ElapsedTime * 0.5f + 0.5f, 1.2f) - 0.1f;
	float Len = 0.2f;

	auto DrawPulse = [&](float Phase, float Alpha)
	{
		float Start = FMath::Clamp(Phase, 0.f, 1.f);
		float End = FMath::Clamp(Phase + Len, 0.f, 1.f);
		if (End > Start)
		{
			HUD->DrawRect(FLinearColor(0.f, 0.7f, 1.f, Alpha),
				BarX + BarW * Start, BarY, BarW * (End - Start), BarH);
		}
	};
	DrawPulse(Phase1, 0.9f);
	DrawPulse(Phase2, 0.5f);

	// End caps (small brackets)
	FLinearColor CapCol(0.f, 0.6f, 1.f, 0.4f);
	HUD->DrawLine(BarX, BarY - 3.f, BarX, BarY + BarH + 3.f, CapCol, 1.f);
	HUD->DrawLine(BarX, BarY - 3.f, BarX + 8.f, BarY - 3.f, CapCol, 1.f);
	HUD->DrawLine(BarX + BarW, BarY - 3.f, BarX + BarW, BarY + BarH + 3.f, CapCol, 1.f);
	HUD->DrawLine(BarX + BarW, BarY - 3.f, BarX + BarW - 8.f, BarY - 3.f, CapCol, 1.f);
}

void FExoLoadingScreen::DrawTip(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Tip label
	FString Label = TEXT("\u25B8 TIP");
	float LabelW, LabelH;
	HUD->GetTextSize(Label, LabelW, LabelH, Font, 0.85f);
	HUD->DrawText(Label, FLinearColor(0.f, 0.7f, 1.f, 0.7f),
		(W - LabelW) * 0.5f, H * 0.74f, Font, 0.85f);

	// Tip text
	FString TipText = Tips[TipIndex];
	float TW, TH;
	HUD->GetTextSize(TipText, TW, TH, Font, 0.9f);
	HUD->DrawText(TipText, FLinearColor(0.55f, 0.58f, 0.65f, 0.9f),
		(W - TW) * 0.5f, H * 0.74f + LabelH + 6.f, Font, 0.9f);
}

void FExoLoadingScreen::DrawScanLines(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Subtle horizontal scan lines
	FLinearColor LineCol(0.f, 0.f, 0.f, 0.03f);
	float Spacing = 4.f;
	float Offset = FMath::Fmod(ElapsedTime * 10.f, Spacing);
	for (float Y = Offset; Y < H; Y += Spacing)
	{
		HUD->DrawRect(LineCol, 0.f, Y, W, 1.f);
	}

	// Sweeping bright scan line
	float SweepY = FMath::Fmod(ElapsedTime * 30.f, H + 40.f) - 20.f;
	float SweepAlpha = 0.05f + 0.02f * FMath::Sin(ElapsedTime * 2.f);
	HUD->DrawRect(FLinearColor(0.f, 0.5f, 1.f, SweepAlpha), 0.f, SweepY, W, 2.f);
	HUD->DrawRect(FLinearColor(0.f, 0.3f, 0.6f, SweepAlpha * 0.3f),
		0.f, SweepY - 6.f, W, 14.f);
}

void FExoLoadingScreen::DrawCornerBrackets(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Margin = 30.f;
	const float BracketLen = 60.f;
	float Alpha = 0.25f + 0.1f * FMath::Sin(ElapsedTime * 1.2f);
	FLinearColor Col(0.f, 0.6f, 1.f, Alpha);

	// Top-left
	HUD->DrawLine(Margin, Margin, Margin + BracketLen, Margin, Col, 1.5f);
	HUD->DrawLine(Margin, Margin, Margin, Margin + BracketLen, Col, 1.5f);

	// Top-right
	HUD->DrawLine(W - Margin, Margin, W - Margin - BracketLen, Margin, Col, 1.5f);
	HUD->DrawLine(W - Margin, Margin, W - Margin, Margin + BracketLen, Col, 1.5f);

	// Bottom-left
	HUD->DrawLine(Margin, H - Margin, Margin + BracketLen, H - Margin, Col, 1.5f);
	HUD->DrawLine(Margin, H - Margin, Margin, H - Margin - BracketLen, Col, 1.5f);

	// Bottom-right
	HUD->DrawLine(W - Margin, H - Margin, W - Margin - BracketLen, H - Margin, Col, 1.5f);
	HUD->DrawLine(W - Margin, H - Margin, W - Margin, H - Margin - BracketLen, Col, 1.5f);
}

void FExoLoadingScreen::DrawHexOverlay(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float GridSize = 50.f;
	const float RowH = GridSize * 0.866f;

	float Alpha = 0.015f + 0.008f * FMath::Sin(ElapsedTime * 0.3f);
	FLinearColor Col(0.1f, 0.35f, 0.6f, Alpha);
	float OffsetY = FMath::Fmod(ElapsedTime * 3.f, RowH * 2.f);

	for (float Y = -RowH + OffsetY; Y < H + RowH; Y += RowH)
	{
		int32 Row = FMath::FloorToInt((Y - OffsetY + RowH) / RowH);
		float XOff = (Row % 2 == 0) ? 0.f : GridSize * 0.5f;
		for (float X = -GridSize + XOff; X < W + GridSize; X += GridSize)
		{
			float Arm = 4.f;
			HUD->DrawLine(X - Arm, Y, X + Arm, Y, Col, 0.5f);
			HUD->DrawLine(X, Y - Arm, X, Y + Arm, Col, 0.5f);
		}
	}
}

void FExoLoadingScreen::DrawLoadingSpinner(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	float CX = W * 0.5f;
	float CY = H * 0.67f;
	float R = 12.f;
	int32 NumDots = 8;

	for (int32 i = 0; i < NumDots; i++)
	{
		float Angle = (2.f * PI * i) / NumDots + ElapsedTime * 3.f;
		float DotX = CX + FMath::Cos(Angle) * R;
		float DotY = CY + FMath::Sin(Angle) * R;
		float Alpha = 0.2f + 0.6f * ((float)i / NumDots);
		float Size = 1.5f + 0.5f * ((float)i / NumDots);
		HUD->DrawRect(FLinearColor(0.f, 0.7f, 1.f, Alpha),
			DotX - Size, DotY - Size, Size * 2.f, Size * 2.f);
	}
}
