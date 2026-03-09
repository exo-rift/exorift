#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Core/ExoTypes.h"
#include "UI/ExoMinimap.h"
#include "UI/ExoDeathCam.h"
#include "UI/ExoLoadingScreen.h"
#include "UI/ExoScoreboard.h"
#include "UI/ExoCompass.h"
#include "UI/ExoNotificationSystem.h"
#include "ExoHUD.generated.h"

UCLASS()
class EXORIFT_API AExoHUD : public AHUD
{
	GENERATED_BODY()

public:
	AExoHUD();

	virtual void DrawHUD() override;

	// Death screen
	void ShowDeathScreen(const FString& KillerName, const FString& WeaponName,
		int32 Placement, int32 TotalPlayers);

	// Loading screen
	void ShowLoadingScreen();
	void HideLoadingScreen();

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
	void DrawRainOverlay();
	void DrawSupplyDropMarkers();
	void DrawSupplyDropAnnouncement();
	void DrawMatchTimer();
	void DrawZoneTimer();
	void DrawVehicleHUD();

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

	// Death cam overlay
	FExoDeathCam DeathCam;

	// Loading screen
	FExoLoadingScreen LoadingScreen;
	bool bShowLoadingScreen = false;

	// Scoreboard (TAB held)
	FExoScoreboard Scoreboard;

	// Compass bar (top of screen)
	FExoCompass Compass;

	// Toast notifications
	FExoNotificationSystem Notifications;

public:
	FExoNotificationSystem& GetNotifications() { return Notifications; }
};
