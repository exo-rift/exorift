#include "Map/ExoZoneEffects.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void FExoZoneEffects::Draw(UCanvas* Canvas, FVector2D ZoneScreenCenter,
	float ZoneScreenRadius, FVector2D PlayerScreenPos,
	float DistToEdge, bool bZoneShrinking, float ShrinkCountdown)
{
	if (!Canvas) return;

	const float WarningDistance = 500.f;
	float Proximity01 = 0.f;

	if (DistToEdge < 0.f)
		Proximity01 = 1.f;
	else if (DistToEdge < WarningDistance)
		Proximity01 = 1.f - (DistToEdge / WarningDistance);

	if (Proximity01 > 0.01f)
	{
		DrawEdgeWarning(Canvas, Proximity01);
	}

	if (DistToEdge < 0.f)
	{
		DrawDirectionalArrow(Canvas, PlayerScreenPos, ZoneScreenCenter);
	}

	if (bZoneShrinking && ShrinkCountdown > 0.f)
	{
		DrawShrinkCountdown(Canvas, ShrinkCountdown);
	}
}

void FExoZoneEffects::DrawEdgeWarning(UCanvas* Canvas, float Proximity01)
{
	float W = Canvas->SizeX;
	float H = Canvas->SizeY;

	float Time = Canvas->GetWorld() ? Canvas->GetWorld()->GetTimeSeconds() : 0.f;
	float Pulse = 0.5f + 0.5f * FMath::Sin(Time * 4.f);
	float Alpha = Proximity01 * (0.12f + 0.08f * Pulse);

	FLinearColor RedTint(1.f, 0.05f, 0.05f, Alpha);

	// Graduated border strips — thicker as proximity increases
	float BorderThickness = 8.f + Proximity01 * 24.f;

	FCanvasTileItem TopStrip(FVector2D(0, 0), FVector2D(W, BorderThickness), RedTint);
	TopStrip.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TopStrip);

	FCanvasTileItem BottomStrip(FVector2D(0, H - BorderThickness),
		FVector2D(W, BorderThickness), RedTint);
	BottomStrip.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BottomStrip);

	FCanvasTileItem LeftStrip(FVector2D(0, 0), FVector2D(BorderThickness, H), RedTint);
	LeftStrip.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(LeftStrip);

	FCanvasTileItem RightStrip(FVector2D(W - BorderThickness, 0),
		FVector2D(BorderThickness, H), RedTint);
	RightStrip.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(RightStrip);

	// Brighter inner edge (2px)
	float InnerAlpha = Alpha * 1.5f;
	FLinearColor BrightEdge(1.f, 0.1f, 0.05f, FMath::Min(InnerAlpha, 0.4f));
	FCanvasTileItem TopInner(FVector2D(0, 0), FVector2D(W, 2.f), BrightEdge);
	TopInner.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TopInner);
	FCanvasTileItem BottomInner(FVector2D(0, H - 2.f), FVector2D(W, 2.f), BrightEdge);
	BottomInner.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BottomInner);
	FCanvasTileItem LeftInner(FVector2D(0, 0), FVector2D(2.f, H), BrightEdge);
	LeftInner.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(LeftInner);
	FCanvasTileItem RightInner(FVector2D(W - 2.f, 0), FVector2D(2.f, H), BrightEdge);
	RightInner.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(RightInner);

	// Corner bracket accents (sci-fi style)
	float BracketLen = 40.f + Proximity01 * 20.f;
	float BracketThick = 3.f;
	FLinearColor BracketCol(1.f, 0.15f, 0.1f, FMath::Min(Alpha * 3.f, 0.6f));

	// Top-left
	FCanvasTileItem TL_H(FVector2D(0, 0), FVector2D(BracketLen, BracketThick), BracketCol);
	TL_H.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(TL_H);
	FCanvasTileItem TL_V(FVector2D(0, 0), FVector2D(BracketThick, BracketLen), BracketCol);
	TL_V.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(TL_V);
	// Top-right
	FCanvasTileItem TR_H(FVector2D(W - BracketLen, 0), FVector2D(BracketLen, BracketThick), BracketCol);
	TR_H.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(TR_H);
	FCanvasTileItem TR_V(FVector2D(W - BracketThick, 0), FVector2D(BracketThick, BracketLen), BracketCol);
	TR_V.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(TR_V);
	// Bottom-left
	FCanvasTileItem BL_H(FVector2D(0, H - BracketThick), FVector2D(BracketLen, BracketThick), BracketCol);
	BL_H.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(BL_H);
	FCanvasTileItem BL_V(FVector2D(0, H - BracketLen), FVector2D(BracketThick, BracketLen), BracketCol);
	BL_V.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(BL_V);
	// Bottom-right
	FCanvasTileItem BR_H(FVector2D(W - BracketLen, H - BracketThick), FVector2D(BracketLen, BracketThick), BracketCol);
	BR_H.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(BR_H);
	FCanvasTileItem BR_V(FVector2D(W - BracketThick, H - BracketLen), FVector2D(BracketThick, BracketLen), BracketCol);
	BR_V.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(BR_V);

	// Scan line sweep (horizontal)
	if (Proximity01 > 0.3f)
	{
		float ScanY = FMath::Fmod(Time * 150.f, H);
		float ScanAlpha = Proximity01 * 0.15f;
		FCanvasTileItem ScanLine(FVector2D(0, ScanY), FVector2D(W, 2.f),
			FLinearColor(1.f, 0.2f, 0.1f, ScanAlpha));
		ScanLine.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ScanLine);
	}

	// "OUTSIDE ZONE" warning text when fully outside
	if (Proximity01 > 0.95f)
	{
		UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
		if (Font)
		{
			float WarnPulse = 0.5f + 0.5f * FMath::Sin(Time * 5.f);
			FString WarnText = TEXT("! OUTSIDE SAFE ZONE !");
			float TW, TH;
			Canvas->TextSize(Font, WarnText, TW, TH);
			TW *= 1.1f; TH *= 1.1f;

			float TX = W * 0.5f - TW * 0.5f;
			float TY = H * 0.25f;

			// Dark panel behind text
			FCanvasTileItem WarnBg(FVector2D(TX - 12.f, TY - 4.f),
				FVector2D(TW + 24.f, TH + 8.f),
				FLinearColor(0.15f, 0.f, 0.f, 0.5f * WarnPulse));
			WarnBg.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(WarnBg);

			FCanvasTextItem WarnItem(FVector2D(TX, TY),
				FText::FromString(WarnText), Font,
				FLinearColor(1.f, 0.15f, 0.1f, WarnPulse));
			WarnItem.Scale = FVector2D(1.1f, 1.1f);
			WarnItem.bOutlined = true;
			WarnItem.OutlineColor = FLinearColor::Black;
			Canvas->DrawItem(WarnItem);
		}
	}
}

void FExoZoneEffects::DrawDirectionalArrow(UCanvas* Canvas, FVector2D PlayerScreenPos,
	FVector2D ZoneScreenCenter)
{
	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;

	FVector2D Dir = ZoneScreenCenter - FVector2D(CX, CY);
	if (Dir.SizeSquared() < 1.f) return;
	Dir.Normalize();

	float Time = Canvas->GetWorld() ? Canvas->GetWorld()->GetTimeSeconds() : 0.f;
	float Pulse = 0.7f + 0.3f * FMath::Abs(FMath::Sin(Time * 3.f));

	// Arrow drawn 120px from screen center
	FVector2D ArrowTip = FVector2D(CX, CY) + Dir * 120.f;
	FVector2D ArrowBase = FVector2D(CX, CY) + Dir * 80.f;

	FVector2D Perp(-Dir.Y, Dir.X);
	FVector2D Wing1 = ArrowBase + Perp * 15.f;
	FVector2D Wing2 = ArrowBase - Perp * 15.f;

	FLinearColor ArrowColor(1.f, 0.2f, 0.2f, Pulse);

	// Shadow
	FLinearColor ShadowCol(0.f, 0.f, 0.f, Pulse * 0.4f);
	Canvas->K2_DrawLine(FVector2D(ArrowTip.X + 1.f, ArrowTip.Y + 1.f),
		FVector2D(Wing1.X + 1.f, Wing1.Y + 1.f), 3.f, ShadowCol);
	Canvas->K2_DrawLine(FVector2D(ArrowTip.X + 1.f, ArrowTip.Y + 1.f),
		FVector2D(Wing2.X + 1.f, Wing2.Y + 1.f), 3.f, ShadowCol);

	// Arrow lines
	Canvas->K2_DrawLine(ArrowTip, Wing1, 3.f, ArrowColor);
	Canvas->K2_DrawLine(ArrowTip, Wing2, 3.f, ArrowColor);
	Canvas->K2_DrawLine(Wing1, Wing2, 2.f, ArrowColor);

	// "ZONE" label with sci-fi panel
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (Font)
	{
		FVector2D LabelPos = ArrowBase - Dir * 25.f;
		FString ZoneLabel = TEXT("ZONE");
		float LW, LH;
		Canvas->TextSize(Font, ZoneLabel, LW, LH);

		float PX = LabelPos.X - LW * 0.5f - 6.f;
		float PY = LabelPos.Y - LH * 0.5f - 3.f;
		float PW = LW + 12.f;
		float PH = LH + 6.f;

		// Background panel
		FCanvasTileItem LabelBg(FVector2D(PX, PY), FVector2D(PW, PH),
			FLinearColor(0.f, 0.f, 0.f, 0.4f * Pulse));
		LabelBg.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(LabelBg);

		// Top accent line
		FCanvasTileItem AccLine(FVector2D(PX, PY), FVector2D(PW, 1.f),
			FLinearColor(1.f, 0.2f, 0.15f, 0.5f * Pulse));
		AccLine.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(AccLine);

		FCanvasTextItem LabelItem(
			FVector2D(LabelPos.X - LW * 0.5f, LabelPos.Y - LH * 0.5f),
			FText::FromString(ZoneLabel), Font, ArrowColor);
		LabelItem.Scale = FVector2D(0.8f, 0.8f);
		Canvas->DrawItem(LabelItem);
	}
}

void FExoZoneEffects::DrawShrinkCountdown(UCanvas* Canvas, float SecondsRemaining)
{
	float CX = Canvas->SizeX * 0.5f;

	int32 Secs = FMath::CeilToInt(SecondsRemaining);
	FString Text = FString::Printf(TEXT("ZONE CLOSING  %d:%02d"), Secs / 60, Secs % 60);

	float Time = Canvas->GetWorld() ? Canvas->GetWorld()->GetTimeSeconds() : 0.f;
	float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 3.f);
	FLinearColor TextColor(1.f, 0.15f, 0.15f, Pulse);

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font) return;

	float TextW, TextH;
	Canvas->TextSize(Font, Text, TextW, TextH);
	TextW *= 1.2f; TextH *= 1.2f;

	float DrawX = CX - TextW * 0.5f;
	float DrawY = 58.f;
	float PW = TextW + 30.f;
	float PH = TextH + 16.f;
	float PX = CX - PW * 0.5f;
	float PY = DrawY - 7.f;

	// Background panel
	FCanvasTileItem BgPanel(FVector2D(PX, PY), FVector2D(PW, PH),
		FLinearColor(0.1f, 0.f, 0.f, 0.5f));
	BgPanel.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BgPanel);

	// Top accent bar
	FCanvasTileItem AccBar(FVector2D(PX, PY), FVector2D(PW, 2.f),
		FLinearColor(1.f, 0.2f, 0.1f, 0.5f * Pulse));
	AccBar.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(AccBar);

	// Bottom accent
	FCanvasTileItem BottomBar(FVector2D(PX, PY + PH - 1.f), FVector2D(PW, 1.f),
		FLinearColor(1.f, 0.2f, 0.1f, 0.25f * Pulse));
	BottomBar.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BottomBar);

	// Corner bracket accents on countdown panel
	float BL = 8.f;
	FLinearColor CornerCol(1.f, 0.2f, 0.1f, 0.4f * Pulse);
	// TL
	FCanvasTileItem C1(FVector2D(PX, PY), FVector2D(BL, 1.f), CornerCol);
	C1.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(C1);
	FCanvasTileItem C2(FVector2D(PX, PY), FVector2D(1.f, BL), CornerCol);
	C2.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(C2);
	// BR
	FCanvasTileItem C3(FVector2D(PX + PW - BL, PY + PH - 1.f), FVector2D(BL, 1.f), CornerCol);
	C3.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(C3);
	FCanvasTileItem C4(FVector2D(PX + PW - 1.f, PY + PH - BL), FVector2D(1.f, BL), CornerCol);
	C4.BlendMode = SE_BLEND_Translucent; Canvas->DrawItem(C4);

	FCanvasTextItem TextItem(FVector2D(DrawX, DrawY), FText::FromString(Text), Font, TextColor);
	TextItem.Scale = FVector2D(1.2f, 1.2f);
	TextItem.bOutlined = true;
	TextItem.OutlineColor = FLinearColor::Black;
	Canvas->DrawItem(TextItem);

	// Urgency: if under 10 seconds, flash a warning icon
	if (SecondsRemaining < 10.f)
	{
		float FlashAlpha = 0.5f + 0.5f * FMath::Sin(Time * 8.f);
		FCanvasTextItem Urgent(FVector2D(PX + 6.f, DrawY),
			FText::FromString(TEXT("!")), Font,
			FLinearColor(1.f, 0.1f, 0.05f, FlashAlpha));
		Urgent.Scale = FVector2D(1.2f, 1.2f);
		Urgent.bOutlined = true;
		Urgent.OutlineColor = FLinearColor::Black;
		Canvas->DrawItem(Urgent);
	}
}
