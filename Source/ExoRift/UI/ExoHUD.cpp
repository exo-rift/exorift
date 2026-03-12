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
#include "UI/ExoHitDirectionIndicator.h"
#include "UI/ExoPickupNotification.h"
#include "UI/ExoPingSystem.h"
#include "UI/ExoCommsWheel.h"
#include "UI/ExoMatchSummary.h"
#include "UI/ExoSpectatorOverlay.h"
#include "UI/ExoTacticalMap.h"
#include "Player/ExoSpectatorPawn.h"
#include "Map/ExoZoneSystem.h"
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

	// Spectator overlay — shown when local player is spectating (not in death cam)
	AExoSpectatorPawn* SpectatorPawn = Cast<AExoSpectatorPawn>(
		GetOwningPlayerController() ? GetOwningPlayerController()->GetPawn() : nullptr);
	if (SpectatorPawn)
	{
		FExoSpectatorOverlay::Draw(Canvas, HUDFont, SpectatorPawn);
	}

	// Check if match is over — show summary instead
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (GS && GS->MatchPhase == EBRMatchPhase::EndGame)
	{
		FExoMatchSummary::Draw(this, Canvas, HUDFont);
		return;
	}

	// During drop phase, only show match state HUD (no gameplay elements)
	bool bInDropPhase = GS && GS->MatchPhase == EBRMatchPhase::DropPhase;

	// --- Gameplay HUD (hidden during drop phase) ---
	if (!bInDropPhase)
	{
		DrawCrosshair();
		DrawHealthBar();
		DrawShieldBar();
		DrawArmorIndicators();
		DrawOverheatBar();
		DrawEnergyBar();
		DrawWeaponIndicator();
		DrawGrenadeIndicator();
		DrawDBNOOverlay();
		DrawKillCount();
		DrawKillStreak();
		DrawAbilities();
		DrawGrenadeWarning();
		DrawInteractionPrompt();
		DrawZiplinePrompt();
		DrawVehicleHUD();
		DrawLocationBanner();
		DrawSprintLines();
	}

	// Always-visible HUD (match info, weather)
	DrawAliveCount();
	DrawKillFeed();
	DrawMatchPhase();
	DrawZoneWarning();
	DrawWeatherIndicator();
	DrawRainOverlay();

	// Compass bar at top of screen
	{
		float PlayerYaw = GetOwningPawn() ? GetOwningPawn()->GetControlRotation().Yaw : 0.f;
		FVector PlayerLoc = GetOwningPawn() ? GetOwningPawn()->GetActorLocation() : FVector::ZeroVector;
		Compass.Tick(DeltaTime);
		Compass.Draw(Canvas, HUDFont, PlayerYaw, PlayerLoc);
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

	// Supply drop world markers
	DrawSupplyDropMarkers();
	DrawSupplyDropAnnouncement();

	// Kill announcements (First Blood, Double Kill, etc.)
	KillAnnouncer.Tick(DeltaTime);
	KillAnnouncer.Draw(Canvas, HUDFont);

	// Hit markers & damage indicators
	FExoHitMarker::Draw(this, Canvas);
	FExoHitDirectionIndicator::Draw(this, Canvas);

	// Pickup & elimination notifications
	FExoPickupNotification::Tick(DeltaTime);
	FExoPickupNotification::Draw(this, Canvas, HUDFont);

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

	// Tactical map overlay (M key)
	if (FExoTacticalMap::bIsOpen)
	{
		FVector MapPlayerPos = GetOwningPawn() ? GetOwningPawn()->GetActorLocation() : FVector::ZeroVector;
		float ZR = 40000.f;
		FVector ZC = FVector::ZeroVector;
		float NZR = 0.f;
		FVector NZC = FVector::ZeroVector;
		for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
		{
			ZR = (*It)->GetCurrentRadius();
			FVector2D ZC2D = (*It)->GetCurrentCenter();
			ZC = FVector(ZC2D.X, ZC2D.Y, 0.f);
			NZR = (*It)->GetTargetRadius();
			FVector2D NZC2D = (*It)->GetTargetCenter();
			NZC = FVector(NZC2D.X, NZC2D.Y, 0.f);
			break;
		}
		FExoTacticalMap::Draw(this, Canvas, HUDFont, MapPlayerPos, ZR, ZC, NZR, NZC);
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
	// Populate stats from player state
	AExoPlayerState* PS = GetOwningPawn()
		? GetOwningPawn()->GetPlayerState<AExoPlayerState>() : nullptr;
	if (PS)
	{
		DeathCam.CachedKills = PS->Kills;
		DeathCam.CachedDamage = PS->DamageDealt;
		int32 Accuracy = (PS->ShotsFired > 0)
			? FMath::RoundToInt32(100.f * PS->ShotsHit / PS->ShotsFired) : 0;
		DeathCam.CachedAccuracy = Accuracy;

		AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
		int32 TimeSec = GS ? FMath::FloorToInt(GS->MatchElapsedTime) : 0;
		DeathCam.SurvivalTime = FString::Printf(TEXT("%d:%02d"), TimeSec / 60, TimeSec % 60);
	}

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
	float ClampPct = FMath::Clamp(Pct, 0.f, 1.f);

	// Background
	DrawRect(BgColor, X, Y, Width, Height);

	// Fill bar
	float FillW = (Width - 2.f) * ClampPct;
	if (FillW > 0.f)
	{
		DrawRect(FillColor, X + 1.f, Y + 1.f, FillW, Height - 2.f);

		// Bright edge highlight at the fill tip
		if (ClampPct < 0.98f && FillW > 2.f)
		{
			FLinearColor EdgeCol = FillColor;
			EdgeCol.A = FMath::Min(EdgeCol.A + 0.3f, 1.f);
			DrawRect(EdgeCol, X + FillW - 1.f, Y + 1.f, 2.f, Height - 2.f);
		}

		// Scanline overlay — horizontal lines for holographic feel
		if (Height >= 10.f)
		{
			FLinearColor ScanCol(0.f, 0.f, 0.f, 0.12f);
			for (float SY = Y + 3.f; SY < Y + Height - 1.f; SY += 3.f)
			{
				DrawRect(ScanCol, X + 1.f, SY, FillW, 1.f);
			}
		}

		// Animated shimmer — bright band that sweeps across the bar
		float Time = GetWorld()->GetTimeSeconds();
		float ShimmerPos = FMath::Fmod(Time * 80.f, Width + 40.f) - 20.f;
		if (ShimmerPos > 0.f && ShimmerPos < FillW)
		{
			float ShimW = FMath::Min(20.f, FillW - ShimmerPos);
			FLinearColor ShimCol = FillColor;
			ShimCol.A = 0.25f;
			DrawRect(ShimCol, X + 1.f + ShimmerPos, Y + 1.f, ShimW, Height - 2.f);
		}
	}

	// Border with subtle glow at fill level
	FLinearColor BorderCol(0.25f, 0.3f, 0.38f, 0.5f);
	DrawLine(X, Y, X + Width, Y, BorderCol);
	DrawLine(X, Y + Height, X + Width, Y + Height, BorderCol);
	DrawLine(X, Y, X, Y + Height, BorderCol);
	DrawLine(X + Width, Y, X + Width, Y + Height, BorderCol);

	// Corner bracket accents (small)
	float BL = FMath::Min(6.f, Width * 0.05f);
	FLinearColor AccCol(FillColor.R, FillColor.G, FillColor.B, 0.35f);
	DrawLine(X, Y, X + BL, Y, AccCol);
	DrawLine(X, Y, X, Y + BL, AccCol);
	DrawLine(X + Width, Y + Height, X + Width - BL, Y + Height, AccCol);
	DrawLine(X + Width, Y + Height, X + Width, Y + Height - BL, AccCol);
}
