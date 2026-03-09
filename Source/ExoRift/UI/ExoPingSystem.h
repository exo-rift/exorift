#pragma once

#include "CoreMinimal.h"

class AHUD;
class UCanvas;
class UFont;

struct FPingEntry
{
	FVector WorldLocation = FVector::ZeroVector;
	float SpawnTime = 0.f;
	float Lifetime = 5.f;
	FLinearColor Color = FLinearColor::White;
	FString Label;
};

/**
 * Static ping system drawn on the HUD canvas.
 * Plain C++ — no UObject, no .generated.h.
 */
class EXORIFT_API FExoPingSystem
{
public:
	static void AddPing(const FVector& Location, const FString& Label, FLinearColor Color);
	static void Tick(float DeltaTime);
	static void DrawPings(AHUD* HUD, UCanvas* Canvas, UFont* Font, const FVector& ViewerLocation);

	static TArray<FPingEntry> ActivePings;

private:
	/** Project world location to screen. Returns false if behind camera. */
	static bool ProjectToScreen(AHUD* HUD, const FVector& WorldPos, FVector2D& OutScreen);

	/** Clamp a screen position to the edges with margin. */
	static FVector2D ClampToScreenEdge(const FVector2D& ScreenPos, float ScreenW, float ScreenH, float Margin);

	/** Draw a small diamond shape at screen position. */
	static void DrawDiamond(AHUD* HUD, const FVector2D& Pos, float Size, const FLinearColor& Color);

	/** Draw an arrow pointing toward a ping that is off-screen. */
	static void DrawArrow(AHUD* HUD, const FVector2D& Pos, const FVector2D& Direction, float Size, const FLinearColor& Color);

	static constexpr float FadeOutDuration = 1.5f;
	static constexpr float EdgeMargin = 40.f;
};
