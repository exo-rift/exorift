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

// Helper: set canvas draw color from FLinearColor
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
	DrawFilledRect(Canvas, BarX, BarY, BarWidth, BarHeight, FLinearColor(0.02f, 0.02f, 0.06f, 0.7f));

	// Center tick
	DrawFilledRect(Canvas, ScreenW * 0.5f - 1.f, BarY, 2.f, BarHeight, FLinearColor(1.f, 1.f, 1.f, 0.6f));

	// Helper: convert world yaw to screen X position relative to player yaw
	auto YawToScreenX = [&](float WorldYaw) -> float
	{
		float Delta = FMath::FindDeltaAngleDegrees(PlayerYaw, WorldYaw);
		return ScreenW * 0.5f + (Delta / FOVDegrees) * BarWidth;
	};

	// Draw cardinal directions
	for (const auto& Dir : CardinalDirs)
	{
		float X = YawToScreenX(Dir.Yaw);
		if (X < BarX - 20.f || X > BarX + BarWidth + 20.f) continue;

		bool bCardinal = (Dir.Label[1] == '\0');
		if (bCardinal)
		{
			Canvas->SetDrawColor(255, 255, 255, 230);
			DrawFilledRect(Canvas, X, BarY, 1.f, BarHeight, FLinearColor(0.8f, 0.8f, 0.8f, 0.5f));
		}
		else
		{
			Canvas->SetDrawColor(160, 160, 180, 160);
		}

		float TW, TH;
		Canvas->TextSize(Font, Dir.Label, TW, TH);
		Canvas->DrawText(Font, Dir.Label, X - TW * 0.5f, BarY + (BarHeight - TH) * 0.5f, 0.85f, 0.85f);
	}

	// Bearing number at center
	int32 Bearing = ((int32)FMath::Fmod(PlayerYaw + 360.f, 360.f));
	FString BearingText = FString::Printf(TEXT("%03d"), Bearing);
	float BW, BH;
	Canvas->TextSize(Font, BearingText, BW, BH);
	Canvas->SetDrawColor(200, 200, 220, 200);
	Canvas->DrawText(Font, BearingText, ScreenW * 0.5f - BW * 0.5f, BarY + BarHeight + 2.f, 0.75f, 0.75f);

	// Zone direction marker
	if (bShowZoneDir)
	{
		float ZX = YawToScreenX(ZoneDirectionYaw);
		if (ZX >= BarX && ZX <= BarX + BarWidth)
			DrawFilledRect(Canvas, ZX - 3.f, BarY, 6.f, BarHeight, FLinearColor(0.f, 0.6f, 1.f, 0.6f));
	}

	// Custom markers
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
			SetColor(Canvas, C);
			Canvas->DrawText(Font, M.Label, MX - LW * 0.5f, BarY - LH - 2.f, 0.7f, 0.7f);
		}
	}
}
