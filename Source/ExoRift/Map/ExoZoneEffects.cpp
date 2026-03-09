#include "Map/ExoZoneEffects.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void FExoZoneEffects::Draw(UCanvas* Canvas, FVector2D ZoneScreenCenter,
	float ZoneScreenRadius, FVector2D PlayerScreenPos,
	float DistToEdge, bool bZoneShrinking, float ShrinkCountdown)
{
	if (!Canvas) return;

	// Proximity: 1.0 = at edge, 0.0 = far inside zone. Warning starts within 500 units.
	const float WarningDistance = 500.f;
	float Proximity01 = 0.f;

	if (DistToEdge < 0.f)
	{
		// Outside the zone -- full intensity
		Proximity01 = 1.f;
	}
	else if (DistToEdge < WarningDistance)
	{
		Proximity01 = 1.f - (DistToEdge / WarningDistance);
	}

	if (Proximity01 > 0.01f)
	{
		DrawEdgeWarning(Canvas, Proximity01);
	}

	// Arrow toward zone center when outside
	if (DistToEdge < 0.f)
	{
		DrawDirectionalArrow(Canvas, PlayerScreenPos, ZoneScreenCenter);
	}

	// Shrink countdown
	if (bZoneShrinking && ShrinkCountdown > 0.f)
	{
		DrawShrinkCountdown(Canvas, ShrinkCountdown);
	}
}

void FExoZoneEffects::DrawEdgeWarning(UCanvas* Canvas, float Proximity01)
{
	float W = Canvas->SizeX;
	float H = Canvas->SizeY;

	// Pulsing red tint overlay that intensifies near the edge
	float Time = Canvas->GetWorld() ? Canvas->GetWorld()->GetTimeSeconds() : 0.f;
	float Pulse = 0.5f + 0.5f * FMath::Sin(Time * 4.f);
	float Alpha = Proximity01 * (0.15f + 0.1f * Pulse);

	FLinearColor RedTint(1.f, 0.05f, 0.05f, Alpha);

	// Draw border strips on all four edges
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
}

void FExoZoneEffects::DrawDirectionalArrow(UCanvas* Canvas, FVector2D PlayerScreenPos,
	FVector2D ZoneScreenCenter)
{
	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;

	// Direction from screen center toward zone center
	FVector2D Dir = ZoneScreenCenter - FVector2D(CX, CY);
	if (Dir.SizeSquared() < 1.f) return;
	Dir.Normalize();

	// Arrow drawn 120px from screen center
	FVector2D ArrowTip = FVector2D(CX, CY) + Dir * 120.f;
	FVector2D ArrowBase = FVector2D(CX, CY) + Dir * 80.f;

	// Perpendicular for arrowhead wings
	FVector2D Perp(-Dir.Y, Dir.X);
	FVector2D Wing1 = ArrowBase + Perp * 15.f;
	FVector2D Wing2 = ArrowBase - Perp * 15.f;

	FLinearColor ArrowColor(1.f, 0.2f, 0.2f, 0.9f);
	Canvas->K2_DrawLine(ArrowTip, Wing1, 3.f, ArrowColor);
	Canvas->K2_DrawLine(ArrowTip, Wing2, 3.f, ArrowColor);
	Canvas->K2_DrawLine(Wing1, Wing2, 2.f, ArrowColor);
}

void FExoZoneEffects::DrawShrinkCountdown(UCanvas* Canvas, float SecondsRemaining)
{
	float CX = Canvas->SizeX * 0.5f;

	int32 Secs = FMath::CeilToInt(SecondsRemaining);

	FString Text = FString::Printf(TEXT("ZONE CLOSING  %d:%02d"),
		Secs / 60, Secs % 60);

	float Time = Canvas->GetWorld() ? Canvas->GetWorld()->GetTimeSeconds() : 0.f;
	float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 3.f);
	FLinearColor TextColor(1.f, 0.15f, 0.15f, Pulse);

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font) return;

	float TextW, TextH;
	Canvas->TextSize(Font, Text, TextW, TextH);

	float DrawX = CX - TextW * 0.5f;
	float DrawY = 60.f;

	FCanvasTextItem TextItem(FVector2D(DrawX, DrawY), FText::FromString(Text), Font, TextColor);
	TextItem.Scale = FVector2D(1.2f, 1.2f);
	TextItem.bOutlined = true;
	TextItem.OutlineColor = FLinearColor::Black;
	Canvas->DrawItem(TextItem);
}
