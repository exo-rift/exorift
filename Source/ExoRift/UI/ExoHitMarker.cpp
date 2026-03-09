#include "UI/ExoHitMarker.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"

TArray<FHitMarkerEntry> FExoHitMarker::HitMarkers;
TArray<FDamageIndicatorEntry> FExoHitMarker::DamageIndicators;

void FExoHitMarker::AddHitMarker(bool bKill, bool bHeadshot)
{
	FHitMarkerEntry Entry;
	Entry.SpawnTime = -1.f; // Will be set on first tick
	Entry.Lifetime = bKill ? 0.8f : 0.4f;
	Entry.bIsKill = bKill;
	Entry.bIsHeadshot = bHeadshot;
	HitMarkers.Add(Entry);
}

void FExoHitMarker::AddDamageIndicator(float Angle)
{
	FDamageIndicatorEntry Entry;
	Entry.SpawnTime = -1.f;
	Entry.Lifetime = 2.f;
	Entry.Angle = Angle;
	DamageIndicators.Add(Entry);
}

void FExoHitMarker::Tick(float DeltaTime)
{
	// Remove expired entries
	float Now = -1.f; // We'll use relative time via lifetime countdown

	for (int32 i = HitMarkers.Num() - 1; i >= 0; i--)
	{
		HitMarkers[i].Lifetime -= DeltaTime;
		if (HitMarkers[i].Lifetime <= 0.f)
			HitMarkers.RemoveAt(i);
	}

	for (int32 i = DamageIndicators.Num() - 1; i >= 0; i--)
	{
		DamageIndicators[i].Lifetime -= DeltaTime;
		if (DamageIndicators[i].Lifetime <= 0.f)
			DamageIndicators.RemoveAt(i);
	}
}

void FExoHitMarker::Draw(AHUD* HUD, UCanvas* Canvas)
{
	if (!HUD || !Canvas) return;
	DrawHitMarkers(HUD, Canvas);
	DrawDamageIndicators(HUD, Canvas);
}

void FExoHitMarker::DrawHitMarkers(AHUD* HUD, UCanvas* Canvas)
{
	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;

	for (const FHitMarkerEntry& Entry : HitMarkers)
	{
		float Alpha = FMath::Clamp(Entry.Lifetime / 0.4f, 0.f, 1.f);
		float Size = 10.f + (1.f - Alpha) * 5.f; // Expands slightly as it fades
		float Gap = 5.f;
		float Thickness = 2.f;

		FLinearColor Color;
		if (Entry.bIsKill)
			Color = FLinearColor(1.f, 0.2f, 0.2f, Alpha); // Red for kills
		else if (Entry.bIsHeadshot)
			Color = FLinearColor(1.f, 0.8f, 0.1f, Alpha); // Gold for headshots
		else
			Color = FLinearColor(1.f, 1.f, 1.f, Alpha);    // White for normal hits

		// Four diagonal lines forming an X around crosshair
		float Diag = 0.707f; // sin(45) = cos(45)
		for (int32 Quadrant = 0; Quadrant < 4; Quadrant++)
		{
			float DirX = (Quadrant < 2) ? 1.f : -1.f;
			float DirY = (Quadrant % 2 == 0) ? 1.f : -1.f;

			float X1 = CX + DirX * Diag * Gap;
			float Y1 = CY + DirY * Diag * Gap;
			float X2 = CX + DirX * Diag * (Gap + Size);
			float Y2 = CY + DirY * Diag * (Gap + Size);

			HUD->DrawLine(X1, Y1, X2, Y2, Color, Thickness);
		}

		// Kill confirmed: extra circle
		if (Entry.bIsKill)
		{
			int32 Segments = 16;
			float Radius = Gap + Size + 4.f;
			float AngleStep = 2.f * PI / Segments;
			for (int32 i = 0; i < Segments; i++)
			{
				float A1 = i * AngleStep;
				float A2 = (i + 1) * AngleStep;
				HUD->DrawLine(
					CX + FMath::Cos(A1) * Radius, CY + FMath::Sin(A1) * Radius,
					CX + FMath::Cos(A2) * Radius, CY + FMath::Sin(A2) * Radius,
					Color, 1.5f);
			}
		}
	}
}

bool FExoHitMarker::HasRecentHit()
{
	return HitMarkers.Num() > 0;
}

void FExoHitMarker::DrawDamageIndicators(AHUD* HUD, UCanvas* Canvas)
{
	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;
	float IndicatorDist = FMath::Min(Canvas->SizeX, Canvas->SizeY) * 0.15f;

	for (const FDamageIndicatorEntry& Entry : DamageIndicators)
	{
		float Alpha = FMath::Clamp(Entry.Lifetime / 2.f, 0.f, 1.f);
		FLinearColor Color(1.f, 0.1f, 0.1f, Alpha * 0.7f);

		float AngleRad = FMath::DegreesToRadians(Entry.Angle);
		float DirX = FMath::Sin(AngleRad);
		float DirY = -FMath::Cos(AngleRad);

		// Draw a small arc/wedge pointing toward damage source
		float ArcLen = 30.f;
		float ArcWidth = 8.f;

		FVector2D Center(CX + DirX * IndicatorDist, CY + DirY * IndicatorDist);
		FVector2D Perp(-DirY, DirX);

		FVector2D P1 = Center + FVector2D(DirX, DirY) * ArcLen * 0.5f;
		FVector2D P2 = Center - FVector2D(DirX, DirY) * ArcLen * 0.5f + Perp * ArcWidth;
		FVector2D P3 = Center - FVector2D(DirX, DirY) * ArcLen * 0.5f - Perp * ArcWidth;

		HUD->DrawLine(P1.X, P1.Y, P2.X, P2.Y, Color, 3.f);
		HUD->DrawLine(P1.X, P1.Y, P3.X, P3.Y, Color, 3.f);
		HUD->DrawLine(P2.X, P2.Y, P3.X, P3.Y, Color, 2.f);
	}
}
