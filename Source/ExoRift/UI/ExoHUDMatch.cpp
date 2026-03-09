// ExoHUDMatch.cpp — Match state HUD: alive count, kill feed, phase, zone, timers, stats
#include "UI/ExoHUD.h"
#include "Core/ExoGameSettings.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoKillStreakComponent.h"
#include "Map/ExoZoneSystem.h"
#include "UI/ExoCountdown.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"

void AExoHUD::DrawAliveCount()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	FString AliveText = FString::Printf(TEXT("Alive: %d / %d"), GS->AliveCount, GS->TotalPlayers);
	float X = Canvas->SizeX - 220.f;
	float Y = 30.f;

	DrawRect(ColorBgDark, X - 10.f, Y - 5.f, 200.f, 30.f);
	DrawText(AliveText, ColorWhite, X, Y, HUDFont, 1.f);
}

void AExoHUD::DrawKillFeed()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	float X = Canvas->SizeX - 350.f;
	float Y = 70.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (const FKillFeedEntry& Entry : GS->GetKillFeed())
	{
		float Age = CurrentTime - Entry.Timestamp;
		if (Age > 8.f) continue;

		float Alpha = FMath::Clamp(1.f - (Age - 5.f) / 3.f, 0.f, 1.f);

		// Background per entry
		DrawRect(FLinearColor(0.f, 0.f, 0.f, Alpha * 0.3f), X - 5.f, Y - 2.f, 340.f, 20.f);

		FLinearColor KillerColor(0.9f, 0.9f, 0.95f, Alpha);
		FLinearColor WeaponColor(0.6f, 0.7f, 0.8f, Alpha);
		FLinearColor VictimColor(0.9f, 0.5f, 0.5f, Alpha);

		float CurX = X;
		DrawText(Entry.KillerName, KillerColor, CurX, Y, HUDFont, 0.7f);
		float KW, KH;
		GetTextSize(Entry.KillerName, KW, KH, HUDFont, 0.7f);
		CurX += KW + 5.f;

		FString WeaponBracket = FString::Printf(TEXT("[%s]"), *Entry.WeaponName);
		DrawText(WeaponBracket, WeaponColor, CurX, Y, HUDFont, 0.65f);
		float WW, WH;
		GetTextSize(WeaponBracket, WW, WH, HUDFont, 0.65f);
		CurX += WW + 5.f;

		DrawText(Entry.VictimName, VictimColor, CurX, Y, HUDFont, 0.7f);
		Y += 24.f;
	}
}

void AExoHUD::DrawMatchPhase()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	switch (GS->MatchPhase)
	{
	case EBRMatchPhase::WaitingForPlayers:
		FExoCountdown::DrawPreMatchCountdown(this, Canvas, HUDFont,
			GS->WaitingTimeRemaining, GS->TotalPlayers, 1);
		break;
	case EBRMatchPhase::DropPhase:
		FExoCountdown::DrawDropPhaseHUD(this, Canvas, HUDFont,
			GS->DropPhaseTimeRemaining);
		break;
	case EBRMatchPhase::Playing:
		// Show "GO!" banner during the first 2 seconds
		FExoCountdown::DrawMatchStartBanner(this, Canvas, HUDFont,
			GS->MatchElapsedTime);
		break;
	case EBRMatchPhase::ZoneShrinking:
		break; // Zone shrinking is shown by DrawZoneTimer + DrawZoneWarning
	case EBRMatchPhase::EndGame:
		break; // Handled by match summary
	}
}

void AExoHUD::DrawZoneWarning()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	AExoZoneSystem* Zone = nullptr;
	for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
	{
		Zone = *It;
		break;
	}
	if (!Zone || Zone->IsInsideZone(Char->GetActorLocation())) return;

	float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f));
	FLinearColor WarningColor = ColorZoneWarning;
	WarningColor.A = 0.5f + Pulse * 0.5f;

	// Screen edge red tint
	float EdgeSize = 80.f;
	FLinearColor EdgeColor(1.f, 0.f, 0.f, 0.1f + Pulse * 0.1f);
	DrawRect(EdgeColor, 0.f, 0.f, EdgeSize, Canvas->SizeY);
	DrawRect(EdgeColor, Canvas->SizeX - EdgeSize, 0.f, EdgeSize, Canvas->SizeY);
	DrawRect(EdgeColor, 0.f, 0.f, Canvas->SizeX, EdgeSize);
	DrawRect(EdgeColor, 0.f, Canvas->SizeY - EdgeSize, Canvas->SizeX, EdgeSize);

	FString WarningText = TEXT("OUTSIDE SAFE ZONE — TAKING DAMAGE");
	float TextW, TextH;
	GetTextSize(WarningText, TextW, TextH, HUDFont, 1.1f);
	float WX = (Canvas->SizeX - TextW) * 0.5f;
	DrawText(WarningText, WarningColor, WX, Canvas->SizeY * 0.25f, HUDFont, 1.1f);
}

void AExoHUD::DrawKillCount()
{
	AExoPlayerState* PS = GetOwningPawn() ?
		GetOwningPawn()->GetPlayerState<AExoPlayerState>() : nullptr;
	if (!PS) return;

	// Kill count with background panel (top left, below minimap)
	float X = 30.f;
	float Y = 230.f; // Below minimap

	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, 100.f, 28.f);
	FString KillText = FString::Printf(TEXT("Kills: %d"), PS->Kills);
	DrawText(KillText, ColorWhite, X, Y, HUDFont, 1.f);
}

void AExoHUD::DrawKillStreak()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;
	UExoKillStreakComponent* SC = Char->GetKillStreakComponent();
	if (!SC || SC->GetCurrentStreak() < 3) return;

	int32 Streak = SC->GetCurrentStreak();
	FString StreakText = FString::Printf(TEXT("x%d %s"), Streak, *SC->GetStreakName());
	float TextW, TextH;
	GetTextSize(StreakText, TextW, TextH, HUDFont, 1.f);

	float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f));
	float A = 0.8f + Pulse * 0.2f;
	FLinearColor C = (Streak >= 8) ? FLinearColor(1.f, 0.2f, 0.2f, A)
		: (Streak >= 5) ? FLinearColor(1.f, 0.5f, 0.1f, A)
		: FLinearColor(1.f, 0.8f, 0.2f, A);

	float X = 30.f, Y = 262.f; // Below kill count
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), X - 5.f, Y - 3.f, TextW + 10.f, TextH + 6.f);
	DrawText(StreakText, C, X, Y, HUDFont, 1.f);
}

void AExoHUD::DrawFPS()
{
	UExoGameSettings* Settings = UExoGameSettings::Get(GetWorld());
	if (!Settings || !Settings->bShowFPS) return;

	float DeltaSec = GetWorld()->GetDeltaSeconds();
	float CurrentFPS = (DeltaSec > 0.f) ? (1.f / DeltaSec) : 0.f;
	SmoothedFPS = FMath::FInterpTo(SmoothedFPS, CurrentFPS, DeltaSec, 5.f);

	FString FPSText = FString::Printf(TEXT("FPS: %d"), FMath::RoundToInt(SmoothedFPS));
	FLinearColor FPSColor = (SmoothedFPS >= 55.f)
		? FLinearColor(0.2f, 1.f, 0.3f, 0.9f)
		: FLinearColor(1.f, 0.3f, 0.2f, 0.9f);

	float TextW, TextH;
	GetTextSize(FPSText, TextW, TextH, HUDFont, 0.85f);
	float X = Canvas->SizeX - TextW - 20.f;
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), X - 6.f, 8.f, TextW + 12.f, TextH + 4.f);
	DrawText(FPSText, FPSColor, X, 10.f, HUDFont, 0.85f);
}

void AExoHUD::DrawMatchTimer()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	// Only show during active match phases
	if (GS->MatchPhase != EBRMatchPhase::Playing && GS->MatchPhase != EBRMatchPhase::ZoneShrinking)
		return;

	int32 TotalSec = FMath::FloorToInt(GS->MatchElapsedTime);
	int32 Minutes = TotalSec / 60;
	int32 Seconds = TotalSec % 60;
	FString TimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

	float TW, TH;
	GetTextSize(TimeText, TW, TH, HUDFont, 1.2f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = 10.f;

	DrawRect(ColorBgDark, X - 12.f, Y - 4.f, TW + 24.f, TH + 8.f);
	DrawText(TimeText, ColorWhite, X, Y, HUDFont, 1.2f);
}

void AExoHUD::DrawZoneTimer()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	// Only show during active match
	if (GS->MatchPhase != EBRMatchPhase::Playing && GS->MatchPhase != EBRMatchPhase::ZoneShrinking)
		return;

	if (GS->CurrentZoneStage < 0) return;

	// Use replicated GameState data — works on all clients
	int32 Ring = GS->CurrentZoneStage + 1; // 1-indexed for display
	bool bShrinking = GS->bZoneShrinking;

	FString ZoneText;
	FString SubText;
	FLinearColor ZoneColor;
	FLinearColor SubColor = FLinearColor::White;

	if (bShrinking)
	{
		int32 ShrinkSec = FMath::CeilToInt(GS->ZoneShrinkTimeRemaining);
		ZoneText = TEXT("RING CLOSING");
		SubText = FString::Printf(TEXT("Ring %d  |  %ds remaining"), Ring, ShrinkSec);
		float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f));
		ZoneColor = FLinearColor(1.f, 0.3f + Pulse * 0.2f, 0.15f, 0.95f);
		SubColor = FLinearColor(1.f, 0.6f, 0.4f, 0.85f);
	}
	else
	{
		int32 HoldSec = FMath::CeilToInt(GS->ZoneHoldTimeRemaining);
		bool bWarning = HoldSec <= 30 && HoldSec > 0;

		ZoneText = FString::Printf(TEXT("Ring %d closing in %ds"), Ring + 1, HoldSec);
		if (bWarning)
		{
			float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 4.f));
			ZoneColor = FLinearColor(1.f, 0.7f + Pulse * 0.15f, 0.2f, 0.9f);
		}
		else
		{
			ZoneColor = FLinearColor(0.7f, 0.8f, 0.9f, 0.85f);
		}
	}

	// Position below the minimap area
	float X = MinimapConfig.ScreenX;
	float Y = MinimapConfig.ScreenY + MinimapConfig.Size + 8.f;

	// Main zone text
	float TW, TH;
	GetTextSize(ZoneText, TW, TH, HUDFont, 0.8f);
	float PanelW = FMath::Max(TW + 20.f, 200.f);
	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, PanelW, TH + 8.f);
	DrawText(ZoneText, ZoneColor, X, Y, HUDFont, 0.8f);

	// Sub-line for shrink progress
	if (!SubText.IsEmpty())
	{
		float SY = Y + TH + 6.f;
		float SW, SH;
		GetTextSize(SubText, SW, SH, HUDFont, 0.65f);
		DrawRect(ColorBgDark, X - 5.f, SY - 2.f, FMath::Max(SW + 20.f, 200.f), SH + 4.f);
		DrawText(SubText, SubColor, X, SY, HUDFont, 0.65f);
	}
}
