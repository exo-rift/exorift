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
		float MaxLife = Entry.bIsKill ? 0.8f : 0.4f;
		float Alpha = FMath::Clamp(Entry.Lifetime / MaxLife, 0.f, 1.f);
		float Size = 10.f + (1.f - Alpha) * 5.f;
		float Gap = 5.f;
		float Thickness = 2.f;

		FLinearColor Color;
		if (Entry.bIsKill)
			Color = FLinearColor(1.f, 0.2f, 0.2f, Alpha);
		else if (Entry.bIsHeadshot)
			Color = FLinearColor(1.f, 0.8f, 0.1f, Alpha);
		else
			Color = FLinearColor(1.f, 1.f, 1.f, Alpha);

		// Shadow behind X for contrast
		FLinearColor Shadow(0.f, 0.f, 0.f, Alpha * 0.35f);
		float Diag = 0.707f;
		for (int32 Q = 0; Q < 4; Q++)
		{
			float DX = (Q < 2) ? 1.f : -1.f;
			float DY = (Q % 2 == 0) ? 1.f : -1.f;
			float X1 = CX + DX * Diag * Gap;
			float Y1 = CY + DY * Diag * Gap;
			float X2 = CX + DX * Diag * (Gap + Size);
			float Y2 = CY + DY * Diag * (Gap + Size);

			HUD->DrawLine(X1 + 1.f, Y1 + 1.f, X2 + 1.f, Y2 + 1.f, Shadow, Thickness + 1.f);
			HUD->DrawLine(X1, Y1, X2, Y2, Color, Thickness);
		}

		// Headshot: radial burst lines (star pattern)
		if (Entry.bIsHeadshot)
		{
			float BurstAlpha = Alpha * FMath::Clamp(Entry.Lifetime / 0.3f, 0.f, 1.f);
			float BurstRadius = Gap + Size + 8.f + (1.f - BurstAlpha) * 12.f;
			float BurstLen = 10.f + (1.f - BurstAlpha) * 6.f;
			FLinearColor BurstCol(1.f, 0.9f, 0.3f, BurstAlpha * 0.7f);
			for (int32 i = 0; i < 8; i++)
			{
				float A = i * (PI / 4.f);
				float IX = FMath::Cos(A);
				float IY = FMath::Sin(A);
				HUD->DrawLine(
					CX + IX * BurstRadius, CY + IY * BurstRadius,
					CX + IX * (BurstRadius + BurstLen), CY + IY * (BurstRadius + BurstLen),
					BurstCol, 1.5f);
			}
		}

		// Kill confirmed: glow ring expanding outward
		if (Entry.bIsKill)
		{
			float ExpandFrac = 1.f - Alpha;
			float GlowRadius = Gap + Size + 4.f + ExpandFrac * 8.f;

			// Outer glow (wide, dim)
			int32 Segments = 24;
			float AngleStep = 2.f * PI / Segments;
			FLinearColor GlowCol(1.f, 0.15f, 0.1f, Alpha * 0.2f);
			for (int32 i = 0; i < Segments; i++)
			{
				float A1 = i * AngleStep;
				float A2 = (i + 1) * AngleStep;
				HUD->DrawLine(
					CX + FMath::Cos(A1) * GlowRadius, CY + FMath::Sin(A1) * GlowRadius,
					CX + FMath::Cos(A2) * GlowRadius, CY + FMath::Sin(A2) * GlowRadius,
					GlowCol, 4.f);
			}

			// Inner kill ring
			FLinearColor RingCol(1.f, 0.2f, 0.15f, Alpha * 0.8f);
			for (int32 i = 0; i < Segments; i++)
			{
				float A1 = i * AngleStep;
				float A2 = (i + 1) * AngleStep;
				HUD->DrawLine(
					CX + FMath::Cos(A1) * GlowRadius, CY + FMath::Sin(A1) * GlowRadius,
					CX + FMath::Cos(A2) * GlowRadius, CY + FMath::Sin(A2) * GlowRadius,
					RingCol, 1.5f);
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
