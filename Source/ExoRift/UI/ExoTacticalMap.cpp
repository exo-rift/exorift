#include "UI/ExoTacticalMap.h"
#include "UI/ExoLocationNames.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

bool FExoTacticalMap::bIsOpen = false;

FVector2D FExoTacticalMap::WorldToMap(const FVector& WorldPos,
	const FVector2D& MapCenter, float MapRadius, float WorldRadius)
{
	float X = WorldPos.X / WorldRadius * MapRadius;
	float Y = -WorldPos.Y / WorldRadius * MapRadius; // Flip Y for screen
	return MapCenter + FVector2D(X, Y);
}

void FExoTacticalMap::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	const FVector& PlayerPos, float ZoneRadius, const FVector& ZoneCenter,
	float NextZoneRadius, const FVector& NextZoneCenter)
{
	if (!Canvas || !Font) return;

	float SX = Canvas->SizeX;
	float SY = Canvas->SizeY;

	// Semi-transparent background
	HUD->DrawRect(FLinearColor(0.01f, 0.02f, 0.04f, 0.85f), 0.f, 0.f, SX, SY);

	// Map area: centered square
	float MapSize = FMath::Min(SX, SY) * 0.75f;
	float MapR = MapSize * 0.5f;
	FVector2D MC(SX * 0.5f, SY * 0.5f);
	float WorldR = 40000.f; // Map half-size in world units

	// Map border
	FLinearColor BorderCol(0.2f, 0.4f, 0.7f, 0.6f);
	HUD->DrawLine(MC.X - MapR, MC.Y - MapR, MC.X + MapR, MC.Y - MapR, BorderCol);
	HUD->DrawLine(MC.X + MapR, MC.Y - MapR, MC.X + MapR, MC.Y + MapR, BorderCol);
	HUD->DrawLine(MC.X + MapR, MC.Y + MapR, MC.X - MapR, MC.Y + MapR, BorderCol);
	HUD->DrawLine(MC.X - MapR, MC.Y + MapR, MC.X - MapR, MC.Y - MapR, BorderCol);

	// Grid lines
	FLinearColor GridCol(0.15f, 0.2f, 0.3f, 0.3f);
	int32 GridDivs = 8;
	for (int32 i = 1; i < GridDivs; i++)
	{
		float T = (float)i / GridDivs;
		float GX = MC.X - MapR + T * MapSize;
		float GY = MC.Y - MapR + T * MapSize;
		HUD->DrawLine(GX, MC.Y - MapR, GX, MC.Y + MapR, GridCol);
		HUD->DrawLine(MC.X - MapR, GY, MC.X + MapR, GY, GridCol);
	}

	// Zone circle (current)
	if (ZoneRadius > 0.f)
	{
		FLinearColor ZoneCol(0.2f, 0.5f, 1.f, 0.6f);
		int32 Segs = 48;
		float ZR = ZoneRadius / WorldR * MapR;
		FVector2D ZC = WorldToMap(ZoneCenter, MC, MapR, WorldR);
		for (int32 i = 0; i < Segs; i++)
		{
			float A1 = (2.f * PI * i) / Segs;
			float A2 = (2.f * PI * (i + 1)) / Segs;
			HUD->DrawLine(
				ZC.X + FMath::Cos(A1) * ZR, ZC.Y + FMath::Sin(A1) * ZR,
				ZC.X + FMath::Cos(A2) * ZR, ZC.Y + FMath::Sin(A2) * ZR,
				ZoneCol, 2.f);
		}
	}

	// Next zone circle
	if (NextZoneRadius > 0.f && NextZoneRadius < ZoneRadius)
	{
		FLinearColor NextCol(1.f, 1.f, 1.f, 0.3f);
		int32 Segs = 32;
		float NR = NextZoneRadius / WorldR * MapR;
		FVector2D NC = WorldToMap(NextZoneCenter, MC, MapR, WorldR);
		for (int32 i = 0; i < Segs; i++)
		{
			float A1 = (2.f * PI * i) / Segs;
			float A2 = (2.f * PI * (i + 1)) / Segs;
			HUD->DrawLine(
				NC.X + FMath::Cos(A1) * NR, NC.Y + FMath::Sin(A1) * NR,
				NC.X + FMath::Cos(A2) * NR, NC.Y + FMath::Sin(A2) * NR,
				NextCol, 1.f);
		}
	}

	// Named locations
	FLinearColor LocCol(0.6f, 0.7f, 0.85f, 0.7f);
	FLinearColor LocDotCol(0.4f, 0.6f, 0.9f, 0.5f);

	struct FMapLoc { FString Name; FVector Pos; };
	TArray<FMapLoc> Locs = {
		{TEXT("COMMAND CENTER"), FVector(0.f, 0.f, 0.f)},
		{TEXT("INDUSTRIAL"), FVector(0.f, 16000.f, 0.f)},
		{TEXT("RESEARCH LABS"), FVector(0.f, -16000.f, 0.f)},
		{TEXT("POWER STATION"), FVector(16000.f, 0.f, 0.f)},
		{TEXT("BARRACKS"), FVector(-16000.f, 0.f, 0.f)},
		{TEXT("OUTPOST A"), FVector(24000.f, 24000.f, 0.f)},
		{TEXT("OUTPOST B"), FVector(-24000.f, 24000.f, 0.f)},
		{TEXT("OUTPOST C"), FVector(24000.f, -24000.f, 0.f)},
		{TEXT("OUTPOST D"), FVector(-24000.f, -24000.f, 0.f)},
		{TEXT("CROSSROADS"), FVector(8000.f, 8000.f, 0.f)},
		{TEXT("THE RUINS"), FVector(-10000.f, 10000.f, 0.f)},
		{TEXT("SCRAPYARD"), FVector(12000.f, -8000.f, 0.f)},
	};

	for (const FMapLoc& L : Locs)
	{
		FVector2D SP = WorldToMap(L.Pos, MC, MapR, WorldR);
		// Skip if outside map bounds
		if (SP.X < MC.X - MapR || SP.X > MC.X + MapR) continue;
		if (SP.Y < MC.Y - MapR || SP.Y > MC.Y + MapR) continue;

		// Location dot
		HUD->DrawRect(LocDotCol, SP.X - 3.f, SP.Y - 3.f, 6.f, 6.f);

		// Label
		float TW, TH;
		HUD->GetTextSize(L.Name, TW, TH, Font, 0.6f);
		HUD->DrawText(L.Name, LocCol, SP.X - TW * 0.5f, SP.Y + 6.f, Font, 0.6f);
	}

	// Player position — bright triangle
	FVector2D PP = WorldToMap(PlayerPos, MC, MapR, WorldR);
	FLinearColor PlayerCol(0.2f, 1.f, 0.4f, 1.f);
	float PS = 8.f;
	HUD->DrawLine(PP.X, PP.Y - PS, PP.X - PS * 0.7f, PP.Y + PS * 0.5f, PlayerCol, 2.f);
	HUD->DrawLine(PP.X, PP.Y - PS, PP.X + PS * 0.7f, PP.Y + PS * 0.5f, PlayerCol, 2.f);
	HUD->DrawLine(PP.X - PS * 0.7f, PP.Y + PS * 0.5f,
		PP.X + PS * 0.7f, PP.Y + PS * 0.5f, PlayerCol, 2.f);

	// Title
	FString Title = TEXT("TACTICAL MAP");
	float TW, TH;
	HUD->GetTextSize(Title, TW, TH, Font, 1.2f);
	HUD->DrawText(Title, FLinearColor(0.7f, 0.85f, 1.f, 0.9f),
		(SX - TW) * 0.5f, MC.Y - MapR - TH - 15.f, Font, 1.2f);

	// Instructions
	FString Hint = TEXT("[M] Close Map");
	float HW, HH;
	HUD->GetTextSize(Hint, HW, HH, Font, 0.7f);
	HUD->DrawText(Hint, FLinearColor(0.5f, 0.6f, 0.7f, 0.6f),
		(SX - HW) * 0.5f, MC.Y + MapR + 10.f, Font, 0.7f);
}
