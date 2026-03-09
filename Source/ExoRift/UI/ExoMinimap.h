#pragma once

#include "CoreMinimal.h"
#include "ExoMinimap.generated.h"

class AHUD;
class UCanvas;
class UFont;
class AExoZoneSystem;

USTRUCT()
struct FMinimapConfig
{
	GENERATED_BODY()

	float ScreenX = 20.f;
	float ScreenY = 20.f;
	float Size = 200.f;
	float WorldRange = 200000.f; // How much of the world the minimap covers
	float PlayerDotSize = 4.f;
	float BorderWidth = 2.f;
};

// Static helper — not an actor, just drawing functions called from ExoHUD
class EXORIFT_API FExoMinimap
{
public:
	static void Draw(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config);

private:
	static FVector2D WorldToMinimap(const FVector& WorldPos, const FVector& CenterPos,
		const FMinimapConfig& Config);
	static void DrawZoneCircle(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config,
		const FVector& CenterPos, AExoZoneSystem* Zone);
};
