// ExoHUDVehicle.cpp — Vehicle HUD: speed, boost, controls
#include "UI/ExoHUD.h"
#include "Vehicles/ExoHoverVehicle.h"
#include "Engine/Canvas.h"

void AExoHUD::DrawVehicleHUD()
{
	AExoHoverVehicle* Vehicle = Cast<AExoHoverVehicle>(GetOwningPawn());
	if (!Vehicle) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	float Time = GetWorld()->GetTimeSeconds();

	// --- Speed indicator (bottom center-left) ---
	float SpeedKmh = Vehicle->GetVelocity().Size() * 0.036f; // UU/s to km/h
	FString SpeedText = FString::Printf(TEXT("%.0f"), SpeedKmh);
	FString UnitText = TEXT("KM/H");

	float SpeedX = W * 0.5f - 180.f;
	float SpeedY = H - 110.f;

	// Speed number
	float SW, SH;
	GetTextSize(SpeedText, SW, SH, HUDFont, 2.f);
	DrawRect(ColorBgDark, SpeedX - 10.f, SpeedY - 5.f, SW + 60.f, SH + 25.f);

	FLinearColor SpeedCol = FMath::Lerp(
		FLinearColor(0.3f, 0.8f, 1.f, 0.9f),
		FLinearColor(1.f, 0.5f, 0.1f, 1.f),
		FMath::Clamp(SpeedKmh / 200.f, 0.f, 1.f));
	DrawText(SpeedText, SpeedCol, SpeedX, SpeedY, HUDFont, 2.f);

	// Unit label
	float UW, UH;
	GetTextSize(UnitText, UW, UH, HUDFont, 0.6f);
	DrawText(UnitText, FLinearColor(0.5f, 0.6f, 0.7f, 0.7f),
		SpeedX + SW + 5.f, SpeedY + SH - UH - 2.f, HUDFont, 0.6f);

	// Accent line under speed
	DrawRect(SpeedCol, SpeedX - 10.f, SpeedY + SH + 18.f, SW + 60.f, 2.f);

	// --- Boost bar (bottom center-right) ---
	float BarX = W * 0.5f + 40.f;
	float BarY = H - 95.f;
	float BarW = 160.f;
	float BarH = 16.f;

	float BoostPct = Vehicle->GetBoostEnergy() / Vehicle->GetMaxBoostEnergy();
	bool bBoosting = Vehicle->IsBoosting();

	// Label
	FString BoostLabel = TEXT("BOOST");
	DrawText(BoostLabel, FLinearColor(0.5f, 0.6f, 0.7f, 0.7f),
		BarX, BarY - 16.f, HUDFont, 0.7f);

	// Background
	DrawRect(ColorBgDark, BarX, BarY, BarW, BarH);

	// Fill
	FLinearColor BoostCol = bBoosting
		? FLinearColor(1.f, 0.6f, 0.1f, 0.9f)   // Orange when active
		: FLinearColor(0.2f, 0.7f, 1.f, 0.8f);   // Blue when charging

	if (BoostPct < 0.2f)
	{
		// Flash red when low
		float Flash = 0.5f + 0.5f * FMath::Sin(Time * 8.f);
		BoostCol = FMath::Lerp(BoostCol, FLinearColor(1.f, 0.1f, 0.1f, 0.9f), Flash);
	}

	float FillW = (BarW - 2.f) * BoostPct;
	DrawRect(BoostCol, BarX + 1.f, BarY + 1.f, FillW, BarH - 2.f);

	// Bright edge
	if (FillW > 2.f && BoostPct < 0.98f)
	{
		FLinearColor EdgeCol = BoostCol;
		EdgeCol.A = FMath::Min(EdgeCol.A + 0.3f, 1.f);
		DrawRect(EdgeCol, BarX + FillW - 1.f, BarY + 1.f, 2.f, BarH - 2.f);
	}

	// Border
	FLinearColor BorderCol(0.25f, 0.3f, 0.38f, 0.5f);
	DrawLine(BarX, BarY, BarX + BarW, BarY, BorderCol);
	DrawLine(BarX, BarY + BarH, BarX + BarW, BarY + BarH, BorderCol);
	DrawLine(BarX, BarY, BarX, BarY + BarH, BorderCol);
	DrawLine(BarX + BarW, BarY, BarX + BarW, BarY + BarH, BorderCol);

	// Boost percentage text
	FString PctText = FString::Printf(TEXT("%.0f%%"), BoostPct * 100.f);
	float PW, PH;
	GetTextSize(PctText, PW, PH, HUDFont, 0.65f);
	DrawText(PctText, ColorWhite, BarX + BarW + 8.f, BarY, HUDFont, 0.65f);

	// --- Vehicle label (top) ---
	FString VehLabel = TEXT("HOVER BIKE");
	float LW, LH;
	GetTextSize(VehLabel, LW, LH, HUDFont, 0.75f);
	float LabelX = (W - LW) * 0.5f;
	float LabelY = H - 130.f;
	DrawRect(FLinearColor(0.03f, 0.03f, 0.06f, 0.5f),
		LabelX - 10.f, LabelY - 3.f, LW + 20.f, LH + 6.f);
	DrawText(VehLabel, FLinearColor(0.4f, 0.7f, 1.f, 0.7f),
		LabelX, LabelY, HUDFont, 0.75f);

	// --- Exit hint ---
	FString ExitHint = TEXT("[E] EXIT");
	float EW, EH;
	GetTextSize(ExitHint, EW, EH, HUDFont, 0.65f);
	DrawText(ExitHint, FLinearColor(0.6f, 0.6f, 0.65f, 0.6f),
		(W - EW) * 0.5f, H - 50.f, HUDFont, 0.65f);
}
