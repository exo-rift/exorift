// ExoHUD.cpp — Core HUD dispatch, constructor, public API, utilities
// Draw functions are split across ExoHUDPlayer, ExoHUDWeapons, ExoHUDMatch, ExoHUDWorld.
#include "UI/ExoHUD.h"
#include "UI/ExoSettingsMenu.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "UI/ExoPingSystem.h"
#include "UI/ExoCommsWheel.h"
#include "UI/ExoMatchSummary.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "EngineUtils.h"

const FLinearColor AExoHUD::ColorHealthGreen = FLinearColor(0.1f, 0.9f, 0.2f, 0.9f);
const FLinearColor AExoHUD::ColorHealthRed = FLinearColor(0.9f, 0.1f, 0.1f, 0.9f);
const FLinearColor AExoHUD::ColorHeatCool = FLinearColor(0.2f, 0.4f, 0.9f, 0.9f);
const FLinearColor AExoHUD::ColorHeatHot = FLinearColor(0.9f, 0.6f, 0.1f, 0.9f);
const FLinearColor AExoHUD::ColorHeatOverheat = FLinearColor(1.f, 0.1f, 0.1f, 1.f);
const FLinearColor AExoHUD::ColorZoneWarning = FLinearColor(1.f, 0.3f, 0.3f, 1.f);
const FLinearColor AExoHUD::ColorBgDark = FLinearColor(0.05f, 0.05f, 0.08f, 0.7f);
const FLinearColor AExoHUD::ColorWhite = FLinearColor(0.9f, 0.92f, 0.95f, 1.f);
const FLinearColor AExoHUD::ColorShieldBlue = FLinearColor(0.2f, 0.5f, 1.f, 0.9f);

AExoHUD::AExoHUD()
{
	static ConstructorHelpers::FObjectFinder<UFont> FontFinder(
		TEXT("/Engine/EngineFonts/Roboto"));
	if (FontFinder.Succeeded())
	{
		HUDFont = FontFinder.Object;
	}
}

void AExoHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Loading screen takes priority over everything
	if (bShowLoadingScreen)
	{
		LoadingScreen.Tick(DeltaTime);
		LoadingScreen.Draw(this, Canvas, HUDFont);
		return;
	}

	// Death cam overlay — show when local player is dead (not DBNO)
	if (DeathCam.IsActive())
	{
		DeathCam.Tick(DeltaTime);
		DeathCam.Draw(this, Canvas, HUDFont);
		return;
	}

	// Check if match is over — show summary instead
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (GS && GS->MatchPhase == EBRMatchPhase::EndGame)
	{
		FExoMatchSummary::Draw(this, Canvas, HUDFont);
		return;
	}

	// --- Gameplay HUD ---
	DrawCrosshair();
	DrawHealthBar();
	DrawShieldBar();
	DrawArmorIndicators();
	DrawOverheatBar();
	DrawEnergyBar();
	DrawWeaponIndicator();
	DrawDBNOOverlay();
	DrawAliveCount();
	DrawKillFeed();
	DrawMatchPhase();
	DrawZoneWarning();
	DrawKillCount();
	DrawKillStreak();
	DrawWeatherIndicator();
	DrawAbilities();
	DrawInteractionPrompt();

	// Compass bar at top of screen
	{
		float PlayerYaw = GetOwningPawn() ? GetOwningPawn()->GetControlRotation().Yaw : 0.f;
		Compass.Tick(DeltaTime);
		Compass.Draw(Canvas, HUDFont, PlayerYaw);
	}

	// Toast notifications (right side)
	Notifications.Tick(DeltaTime);
	Notifications.Draw(Canvas, HUDFont);

	// Ping indicators
	{
		AExoCharacter* PingChar = Cast<AExoCharacter>(GetOwningPawn());
		FVector ViewLoc = PingChar ? PingChar->GetActorLocation() : FVector::ZeroVector;
		FExoPingSystem::DrawPings(this, Canvas, HUDFont, ViewLoc);
	}

	// POI location name
	DrawPOIIndicator();

	// Supply drop world markers
	DrawSupplyDropMarkers();
	DrawSupplyDropAnnouncement();

	// Hit markers & damage indicators
	FExoHitMarker::Draw(this, Canvas);

	// Floating damage numbers
	AExoDamageNumbers* DmgNums = AExoDamageNumbers::Get(GetWorld());
	if (DmgNums)
	{
		DmgNums->DrawNumbers(this, Canvas, HUDFont);
	}

	// Minimap
	FExoMinimap::Draw(this, Canvas, MinimapConfig);

	// Match timer & zone timer (during active match)
	DrawMatchTimer();
	DrawZoneTimer();

	// FPS counter (drawn on top of gameplay HUD)
	DrawFPS();

	// Comms wheel overlay
	if (FExoCommsWheel::bIsOpen)
	{
		FExoCommsWheel::Draw(this, Canvas, HUDFont);
	}

	// Scoreboard overlay (TAB held)
	if (GetOwningPlayerController() && GetOwningPlayerController()->IsInputKeyDown(EKeys::Tab))
	{
		AExoGameState* ScoreGS = GetWorld()->GetGameState<AExoGameState>();
		if (ScoreGS)
		{
			TArray<AExoPlayerState*> SortedPlayers;
			for (APlayerState* PS : ScoreGS->PlayerArray)
			{
				if (AExoPlayerState* ExoPS = Cast<AExoPlayerState>(PS))
					SortedPlayers.Add(ExoPS);
			}
			SortedPlayers.Sort([](const AExoPlayerState& A, const AExoPlayerState& B)
			{
				return A.Kills > B.Kills;
			});
			Scoreboard.Draw(Canvas, HUDFont, SortedPlayers,
				ScoreGS->MatchElapsedTime, ScoreGS->AliveCount, ScoreGS->TotalPlayers);
		}
	}

	// Settings menu overlay — drawn last so it covers everything
	if (FExoSettingsMenu::bIsOpen)
	{
		FExoSettingsMenu::Draw(this, Canvas, HUDFont);
	}
}

// --- Public API ---

void AExoHUD::ShowDeathScreen(const FString& KillerName, const FString& WeaponName,
	int32 Placement, int32 TotalPlayers)
{
	DeathCam.Init(KillerName, WeaponName, Placement, TotalPlayers);
}

void AExoHUD::ShowLoadingScreen()
{
	bShowLoadingScreen = true;
	LoadingScreen.Activate();
}

void AExoHUD::HideLoadingScreen()
{
	bShowLoadingScreen = false;
}

// --- Layout Utilities ---

FVector2D AExoHUD::GetScreenCenter() const
{
	return FVector2D(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f);
}

void AExoHUD::DrawProgressBar(float X, float Y, float Width, float Height,
	float Pct, FLinearColor FillColor, FLinearColor BgColor)
{
	DrawRect(BgColor, X, Y, Width, Height);
	DrawRect(FillColor, X + 1.f, Y + 1.f, (Width - 2.f) * FMath::Clamp(Pct, 0.f, 1.f), Height - 2.f);

	FLinearColor BorderColor(0.3f, 0.35f, 0.4f, 0.6f);
	DrawLine(X, Y, X + Width, Y, BorderColor);
	DrawLine(X, Y + Height, X + Width, Y + Height, BorderColor);
	DrawLine(X, Y, X, Y + Height, BorderColor);
	DrawLine(X + Width, Y, X + Width, Y + Height, BorderColor);
}
