#pragma once

#include "CoreMinimal.h"

class UCanvas;
class UFont;

/**
 * Horizontal compass bar drawn at the top of the screen.
 * Shows cardinal/intercardinal directions and bearing degrees.
 * Includes markers for pings, enemies spotted, and zone center.
 */
class FExoCompass
{
public:
	struct FCompassMarker
	{
		float WorldYaw;        // Direction in world space
		FString Label;
		FLinearColor Color;
		float Duration = 0.f;  // 0 = permanent
		float Elapsed = 0.f;
	};

	/** Draw compass bar. PlayerLocation used for POI landmark bearing calculation. */
	void Draw(UCanvas* Canvas, UFont* Font, float PlayerYaw, const FVector& PlayerLocation);
	void Tick(float DeltaTime);
	void AddMarker(float WorldYaw, const FString& Label, FLinearColor Color, float Duration = 5.f);
	void SetZoneDirection(float Yaw) { ZoneDirectionYaw = Yaw; bShowZoneDir = true; }
	void ClearZoneDirection() { bShowZoneDir = false; }

	/** POI landmark shown on compass when within range. */
	struct FPOILandmark
	{
		const TCHAR* Name;
		FVector WorldLocation;
	};

	/** Get the static list of POI landmarks for compass display. */
	static const TArray<FPOILandmark>& GetPOILandmarks();

private:
	TArray<FCompassMarker> Markers;
	float ZoneDirectionYaw = 0.f;
	bool bShowZoneDir = false;

	/** Max distance (unreal units) at which a POI label is shown on compass. */
	static constexpr float POIMaxDistance = 50000.f;

	static constexpr float BarWidth = 500.f;
	static constexpr float BarHeight = 24.f;
	static constexpr float TopMargin = 8.f;
	static constexpr float FOVDegrees = 90.f; // Visible yaw range
};
