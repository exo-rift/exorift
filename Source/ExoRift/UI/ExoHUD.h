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
	void DrawArmorIndicators();
	void DrawOverheatBar();
	void DrawAliveCount();
	void DrawKillFeed();
	void DrawMatchPhase();
	void DrawZoneWarning();
	void DrawKillCount();
	void DrawWeaponIndicator();
	void DrawInteractionPrompt();
	void DrawKillStreak();
	void DrawAbilities();
	void DrawDBNOOverlay();
	void DrawEnergyBar();
	void DrawWeatherIndicator();
	void DrawSupplyDropMarkers();
	void DrawSupplyDropAnnouncement();

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

	void DrawFPS();

	float CrosshairSpread = 0.f;
	bool bShowZoneWarning = false;
	float SmoothedFPS = 60.f;

	UPROPERTY()
	UFont* HUDFont;

	FMinimapConfig MinimapConfig;
};
