#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;
class AExoZoneSystem;

struct FMinimapConfig
{
	float ScreenX = 20.f;
	float ScreenY = 20.f;
	float Size = 220.f;
	float WorldRange = 60000.f; // Local range centered on player
	float PlayerDotSize = 4.f;
	float BorderWidth = 2.f;
};

// Static helper — drawing functions called from ExoHUD
class EXORIFT_API FExoMinimap
{
public:
	static void Draw(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config);

private:
	static FVector2D WorldToMinimap(const FVector& WorldPos, const FVector& CenterPos,
		float PlayerYaw, const FMinimapConfig& Config);
	static void DrawZoneCircle(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config,
		const FVector& CenterPos, float PlayerYaw, AExoZoneSystem* Zone);
	static void DrawFOVCone(AHUD* HUD, float CenterX, float CenterY,
		const FMinimapConfig& Config);
	static void DrawCardinals(AHUD* HUD, UFont* Font, float CenterX, float CenterY,
		float PlayerYaw, const FMinimapConfig& Config);
	static void DrawGridLines(AHUD* HUD, float CenterX, float CenterY,
		const FMinimapConfig& Config);
	static void DrawPOILabels(AHUD* HUD, UFont* Font, const FMinimapConfig& Config,
		const FVector& CenterPos, float PlayerYaw);
	static void DrawCurrentLocation(AHUD* HUD, UFont* Font, const FMinimapConfig& Config,
		const FVector& PlayerPos);
};
