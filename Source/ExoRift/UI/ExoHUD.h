#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Core/ExoTypes.h"
#include "UI/ExoMinimap.h"
#include "ExoHUD.generated.h"

UCLASS()
class EXORIFT_API AExoHUD : public AHUD
{
	GENERATED_BODY()

public:
	AExoHUD();

	virtual void DrawHUD() override;

protected:
	void DrawCrosshair();
	void DrawHealthBar();
	void DrawShieldBar();
	void DrawOverheatBar();
	void DrawAliveCount();
	void DrawKillFeed();
	void DrawMatchPhase();
	void DrawZoneWarning();
	void DrawKillCount();
	void DrawWeaponIndicator();
	void DrawInteractionPrompt();

	// Layout helpers
	FVector2D GetScreenCenter() const;
	void DrawProgressBar(float X, float Y, float Width, float Height,
		float Pct, FLinearColor FillColor, FLinearColor BgColor);

	// Colors
	static const FLinearColor ColorHealthGreen;
	static const FLinearColor ColorHealthRed;
	static const FLinearColor ColorHeatCool;
	static const FLinearColor ColorHeatHot;
	static const FLinearColor ColorHeatOverheat;
	static const FLinearColor ColorZoneWarning;
	static const FLinearColor ColorBgDark;
	static const FLinearColor ColorWhite;
	static const FLinearColor ColorShieldBlue;

	float CrosshairSpread = 0.f;
	bool bShowZoneWarning = false;

	UPROPERTY()
	UFont* HUDFont;

	FMinimapConfig MinimapConfig;
};
