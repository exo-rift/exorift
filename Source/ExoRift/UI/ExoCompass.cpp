#include "UI/ExoCompass.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

static const struct { float Yaw; const TCHAR* Label; } CardinalDirs[] = {
	{   0.f, TEXT("N")  },
	{  45.f, TEXT("NE") },
	{  90.f, TEXT("E")  },
	{ 135.f, TEXT("SE") },
	{ 180.f, TEXT("S")  },
	{ 225.f, TEXT("SW") },
	{ 270.f, TEXT("W")  },
	{ 315.f, TEXT("NW") },
};

static void SetColor(UCanvas* C, const FLinearColor& Col)
{
	C->SetDrawColor(
		FMath::Clamp((int32)(Col.R * 255), 0, 255),
		FMath::Clamp((int32)(Col.G * 255), 0, 255),
		FMath::Clamp((int32)(Col.B * 255), 0, 255),
		FMath::Clamp((int32)(Col.A * 255), 0, 255));
}

static void DrawFilledRect(UCanvas* Canvas, float X, float Y, float W, float H, const FLinearColor& Color)
{
	SetColor(Canvas, Color);
	Canvas->DrawTile(Canvas->DefaultTexture, X, Y, W, H, 0, 0, 1, 1);
}

void FExoCompass::Tick(float DeltaTime)
{
	for (int32 i = Markers.Num() - 1; i >= 0; i--)
	{
		if (Markers[i].Duration > 0.f)
		{
			Markers[i].Elapsed += DeltaTime;
			if (Markers[i].Elapsed >= Markers[i].Duration)
				Markers.RemoveAt(i);
		}
	}
}

void FExoCompass::AddMarker(float WorldYaw, const FString& Label, FLinearColor Color, float Duration)
{
	FCompassMarker M;
	M.WorldYaw = WorldYaw;
	M.Label = Label;
	M.Color = Color;
	M.Duration = Duration;
	Markers.Add(M);
}

void FExoCompass::Draw(UCanvas* Canvas, UFont* Font, float PlayerYaw)
{
	if (!Canvas || !Font) return;

	const float ScreenW = Canvas->SizeX;
	const float BarX = (ScreenW - BarWidth) * 0.5f;
	const float BarY = TopMargin;

	// Background bar
	DrawFilledRect(Canvas, BarX, BarY, BarWidth, BarHeight,
		FLinearColor(0.02f, 0.02f, 0.06f, 0.75f));

	// Top accent line
	DrawFilledRect(Canvas, BarX, BarY, BarWidth, 1.f,
		FLinearColor(0.15f, 0.25f, 0.4f, 0.4f));
	// Bottom accent line
	DrawFilledRect(Canvas, BarX, BarY + BarHeight - 1.f, BarWidth, 1.f,
		FLinearColor(0.15f, 0.25f, 0.4f, 0.3f));

	// End cap brackets
	float BL = 8.f;
	FLinearColor BrackCol(0.f, 0.5f, 0.8f, 0.35f);
	DrawFilledRect(Canvas, BarX, BarY, BL, 1.f, BrackCol);
	DrawFilledRect(Canvas, BarX, BarY, 1.f, BL, BrackCol);
	DrawFilledRect(Canvas, BarX + BarWidth - BL, BarY, BL, 1.f, BrackCol);
	DrawFilledRect(Canvas, BarX + BarWidth - 1.f, BarY, 1.f, BL, BrackCol);
	DrawFilledRect(Canvas, BarX, BarY + BarHeight - 1.f, BL, 1.f, BrackCol);
	DrawFilledRect(Canvas, BarX, BarY + BarHeight - BL, 1.f, BL, BrackCol);
	DrawFilledRect(Canvas, BarX + BarWidth - BL, BarY + BarHeight - 1.f, BL, 1.f, BrackCol);
	DrawFilledRect(Canvas, BarX + BarWidth - 1.f, BarY + BarHeight - BL, 1.f, BL, BrackCol);

	// Center tick (inverted triangle indicator)
	float CenterX = ScreenW * 0.5f;
	DrawFilledRect(Canvas, CenterX - 1.f, BarY, 2.f, BarHeight,
		FLinearColor(0.8f, 0.9f, 1.f, 0.5f));
	// Small triangle at bottom of center
	DrawFilledRect(Canvas, CenterX - 3.f, BarY + BarHeight, 6.f, 2.f,
		FLinearColor(0.f, 0.6f, 1.f, 0.5f));

	auto YawToScreenX = [&](float WorldYaw) -> float
	{
		float Delta = FMath::FindDeltaAngleDegrees(PlayerYaw, WorldYaw);
		return ScreenW * 0.5f + (Delta / FOVDegrees) * BarWidth;
	};

	// Degree tick marks (every 15 degrees)
	for (int32 Deg = 0; Deg < 360; Deg += 15)
	{
		float X = YawToScreenX((float)Deg);
		if (X < BarX || X > BarX + BarWidth) continue;
		bool bMajor = (Deg % 45 == 0);
		float TickH = bMajor ? 6.f : 3.f;
		float TickAlpha = bMajor ? 0.25f : 0.12f;
		DrawFilledRect(Canvas, X, BarY + BarHeight - TickH, 1.f, TickH,
			FLinearColor(0.5f, 0.6f, 0.7f, TickAlpha));
	}

	// Draw cardinal directions
	for (const auto& Dir : CardinalDirs)
	{
		float X = YawToScreenX(Dir.Yaw);
		if (X < BarX - 20.f || X > BarX + BarWidth + 20.f) continue;

		bool bCardinal = (Dir.Label[1] == '\0');
		bool bNorth = (Dir.Yaw == 0.f);

		if (bCardinal)
		{
			// Vertical tick line
			FLinearColor TickCol = bNorth
				? FLinearColor(1.f, 0.3f, 0.2f, 0.6f)
				: FLinearColor(0.6f, 0.6f, 0.65f, 0.35f);
			DrawFilledRect(Canvas, X, BarY, 1.f, BarHeight, TickCol);
		}

		// Text color: N = red, cardinal = bright, intercardinal = dim
		FLinearColor TextCol;
		if (bNorth)
			TextCol = FLinearColor(1.f, 0.35f, 0.25f, 0.95f);
		else if (bCardinal)
			TextCol = FLinearColor(0.85f, 0.88f, 0.92f, 0.9f);
		else
			TextCol = FLinearColor(0.55f, 0.58f, 0.65f, 0.65f);

		float TW, TH;
		Canvas->TextSize(Font, Dir.Label, TW, TH);

		// Shadow
		SetColor(Canvas, FLinearColor(0.f, 0.f, 0.f, TextCol.A * 0.4f));
		Canvas->DrawText(Font, Dir.Label,
			X - TW * 0.5f + 1.f, BarY + (BarHeight - TH) * 0.5f + 1.f, 0.85f, 0.85f);
		// Label
		SetColor(Canvas, TextCol);
		Canvas->DrawText(Font, Dir.Label,
			X - TW * 0.5f, BarY + (BarHeight - TH) * 0.5f, 0.85f, 0.85f);
	}

	// Bearing number below center
	int32 Bearing = ((int32)FMath::Fmod(PlayerYaw + 360.f, 360.f));
	FString BearingText = FString::Printf(TEXT("%03d"), Bearing);
	float BW, BH;
	Canvas->TextSize(Font, BearingText, BW, BH);
	// Small background for bearing
	DrawFilledRect(Canvas, CenterX - BW * 0.5f - 4.f, BarY + BarHeight + 2.f,
		BW + 8.f, BH + 2.f, FLinearColor(0.02f, 0.02f, 0.05f, 0.5f));
	SetColor(Canvas, FLinearColor(0.7f, 0.75f, 0.85f, 0.8f));
	Canvas->DrawText(Font, BearingText,
		CenterX - BW * 0.5f, BarY + BarHeight + 3.f, 0.75f, 0.75f);

	// Zone direction marker
	if (bShowZoneDir)
	{
		float ZX = YawToScreenX(ZoneDirectionYaw);
		if (ZX >= BarX && ZX <= BarX + BarWidth)
		{
			DrawFilledRect(Canvas, ZX - 2.f, BarY, 4.f, BarHeight,
				FLinearColor(0.f, 0.5f, 1.f, 0.4f));
			// Diamond indicator at top
			DrawFilledRect(Canvas, ZX - 3.f, BarY - 4.f, 6.f, 4.f,
				FLinearColor(0.f, 0.6f, 1.f, 0.6f));
		}
	}

	// Custom markers (pings, threats)
	for (const FCompassMarker& M : Markers)
	{
		float MX = YawToScreenX(M.WorldYaw);
		if (MX < BarX || MX > BarX + BarWidth) continue;

		float Alpha = (M.Duration > 0.f) ? FMath::Max(1.f - M.Elapsed / M.Duration, 0.f) : 1.f;
		FLinearColor C = M.Color;
		C.A *= Alpha;
		DrawFilledRect(Canvas, MX - 2.f, BarY, 4.f, BarHeight, C);

		if (!M.Label.IsEmpty())
		{
			float LW, LH;
			Canvas->TextSize(Font, M.Label, LW, LH);
			// Background for marker label
			DrawFilledRect(Canvas, MX - LW * 0.5f - 3.f, BarY - LH - 4.f,
				LW + 6.f, LH + 2.f, FLinearColor(0.f, 0.f, 0.f, 0.4f * Alpha));
			SetColor(Canvas, C);
			Canvas->DrawText(Font, M.Label, MX - LW * 0.5f, BarY - LH - 3.f, 0.7f, 0.7f);
		}
	}
}
