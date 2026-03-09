#pragma once

#include "CoreMinimal.h"

struct FHitMarkerEntry
{
	float SpawnTime = 0.f;
	float Lifetime = 0.4f;
	bool bIsKill = false;
	bool bIsHeadshot = false;
};

struct FDamageIndicatorEntry
{
	float SpawnTime = 0.f;
	float Lifetime = 2.f;
	float Angle = 0.f; // Direction damage came from relative to player facing
};

// Static helper drawn from HUD
class EXORIFT_API FExoHitMarker
{
public:
	static void AddHitMarker(bool bKill, bool bHeadshot);
	static void AddDamageIndicator(float Angle);
	static void Tick(float DeltaTime);
	static void Draw(AHUD* HUD, UCanvas* Canvas);

private:
	static void DrawHitMarkers(AHUD* HUD, UCanvas* Canvas);
	static void DrawDamageIndicators(AHUD* HUD, UCanvas* Canvas);

	static TArray<FHitMarkerEntry> HitMarkers;
	static TArray<FDamageIndicatorEntry> DamageIndicators;
};
