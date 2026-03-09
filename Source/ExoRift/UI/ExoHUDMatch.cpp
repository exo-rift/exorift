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

	float X = Canvas->SizeX - 230.f;
	float Y = 30.f;
	float PW = 210.f, PH = 32.f;

	// Panel background
	DrawRect(ColorBgDark, X - 10.f, Y - 5.f, PW, PH);

	// Left accent stripe (cyan)
	FLinearColor AccCol(0.f, 0.6f, 1.f, 0.5f);
	DrawRect(AccCol, X - 10.f, Y - 5.f, 2.f, PH);

	// Icon: simple person silhouette as text
	DrawText(TEXT("A"), AccCol, X - 2.f, Y - 1.f, HUDFont, 0.8f);

	FString AliveText = FString::Printf(TEXT("Alive: %d / %d"), GS->AliveCount, GS->TotalPlayers);
	DrawText(AliveText, ColorWhite, X + 18.f, Y, HUDFont, 1.f);
}

void AExoHUD::DrawKillFeed()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	float X = Canvas->SizeX - 360.f;
	float Y = 70.f;
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (const FKillFeedEntry& Entry : GS->GetKillFeed())
	{
		float Age = CurrentTime - Entry.Timestamp;
		if (Age > 8.f) continue;

		float Alpha = FMath::Clamp(1.f - (Age - 5.f) / 3.f, 0.f, 1.f);

		// Slide-in from right during first 0.3s
		float SlideOffset = 0.f;
		if (Age < 0.3f)
			SlideOffset = (1.f - Age / 0.3f) * 50.f;

		float EX = X + SlideOffset;

		// Background per entry
		DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, Alpha * 0.5f),
			EX - 5.f, Y - 2.f, 350.f, 22.f);

		// Left accent — red stripe for kills
		DrawRect(FLinearColor(0.8f, 0.15f, 0.1f, Alpha * 0.6f),
			EX - 5.f, Y - 2.f, 2.f, 22.f);

		FLinearColor KillerColor(0.9f, 0.9f, 0.95f, Alpha);
		FLinearColor WeaponColor(0.5f, 0.6f, 0.7f, Alpha * 0.8f);
		FLinearColor VictimColor(0.9f, 0.4f, 0.4f, Alpha);
		FLinearColor ArrowColor(0.45f, 0.5f, 0.6f, Alpha * 0.7f);

		float CurX = EX;
		DrawText(Entry.KillerName, KillerColor, CurX, Y, HUDFont, 0.7f);
		float KW, KH;
		GetTextSize(Entry.KillerName, KW, KH, HUDFont, 0.7f);
		CurX += KW + 6.f;

		// Arrow indicator instead of brackets
		DrawText(TEXT(">"), ArrowColor, CurX, Y - 1.f, HUDFont, 0.65f);
		float AW, AH;
		GetTextSize(TEXT(">"), AW, AH, HUDFont, 0.65f);
		CurX += AW + 3.f;

		FString WeaponBracket = FString::Printf(TEXT("[%s]"), *Entry.WeaponName);
		DrawText(WeaponBracket, WeaponColor, CurX, Y, HUDFont, 0.65f);
		float WW, WH;
		GetTextSize(WeaponBracket, WW, WH, HUDFont, 0.65f);
		CurX += WW + 3.f;

		DrawText(TEXT(">"), ArrowColor, CurX, Y - 1.f, HUDFont, 0.65f);
		CurX += AW + 6.f;

		DrawText(Entry.VictimName, VictimColor, CurX, Y, HUDFont, 0.7f);
		Y += 26.f;
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
		FExoCountdown::DrawMatchStartBanner(this, Canvas, HUDFont,
			GS->MatchElapsedTime);
		break;
	case EBRMatchPhase::ZoneShrinking:
		break;
	case EBRMatchPhase::EndGame:
		break;
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

	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = FMath::Abs(FMath::Sin(Time * 3.f));

	// Red edge vignette — pulsing intensity
	float EdgeSize = 80.f + Pulse * 20.f;
	float EdgeAlpha = 0.08f + Pulse * 0.08f;
	FLinearColor EdgeColor(1.f, 0.f, 0.f, EdgeAlpha);
	DrawRect(EdgeColor, 0.f, 0.f, EdgeSize, Canvas->SizeY);
	DrawRect(EdgeColor, Canvas->SizeX - EdgeSize, 0.f, EdgeSize, Canvas->SizeY);
	DrawRect(EdgeColor, 0.f, 0.f, Canvas->SizeX, EdgeSize * 0.5f);
	DrawRect(EdgeColor, 0.f, Canvas->SizeY - EdgeSize * 0.5f, Canvas->SizeX, EdgeSize * 0.5f);

	// Warning banner
	FString WarningText = TEXT("OUTSIDE SAFE ZONE");
	float TextW, TextH;
	GetTextSize(WarningText, TextW, TextH, HUDFont, 1.2f);
	float PW = TextW + 50.f;
	float pH = TextH + 16.f;
	float WX = (Canvas->SizeX - PW) * 0.5f;
	float WY = Canvas->SizeY * 0.24f;

	// Banner background
	DrawRect(FLinearColor(0.15f, 0.f, 0.f, 0.5f + Pulse * 0.15f), WX, WY, PW, pH);
	// Top and bottom accent bars
	FLinearColor BarCol(1.f, 0.2f, 0.1f, 0.5f + Pulse * 0.3f);
	DrawRect(BarCol, WX, WY, PW, 2.f);
	DrawRect(FLinearColor(BarCol.R, BarCol.G, BarCol.B, BarCol.A * 0.5f),
		WX, WY + pH - 1.f, PW, 1.f);

	// Corner brackets
	float BL = 12.f;
	DrawLine(WX, WY, WX + BL, WY, BarCol, 2.f);
	DrawLine(WX, WY, WX, WY + BL, BarCol, 2.f);
	DrawLine(WX + PW, WY + pH, WX + PW - BL, WY + pH, BarCol, 2.f);
	DrawLine(WX + PW, WY + pH, WX + PW, WY + pH - BL, BarCol, 2.f);

	// Warning icon
	float DX = WX + 16.f, DY = WY + pH * 0.5f, DS = 5.f;
	DrawLine(DX, DY - DS, DX + DS, DY, BarCol, 1.5f);
	DrawLine(DX + DS, DY, DX, DY + DS, BarCol, 1.5f);
	DrawLine(DX, DY + DS, DX - DS, DY, BarCol, 1.5f);
	DrawLine(DX - DS, DY, DX, DY - DS, BarCol, 1.5f);

	// Main text
	FLinearColor WarningColor = ColorZoneWarning;
	WarningColor.A = 0.7f + Pulse * 0.3f;
	DrawText(WarningText, WarningColor,
		WX + (PW - TextW) * 0.5f, WY + (pH - TextH) * 0.5f, HUDFont, 1.2f);

	// Sub-text: "TAKING DAMAGE"
	FString SubText = TEXT("TAKING DAMAGE");
	float SW, SH;
	GetTextSize(SubText, SW, SH, HUDFont, 0.75f);
	DrawText(SubText, FLinearColor(1.f, 0.5f, 0.4f, 0.6f + Pulse * 0.2f),
		(Canvas->SizeX - SW) * 0.5f, WY + pH + 6.f, HUDFont, 0.75f);
}

void AExoHUD::DrawKillCount()
{
	AExoPlayerState* PS = GetOwningPawn() ?
		GetOwningPawn()->GetPlayerState<AExoPlayerState>() : nullptr;
	if (!PS) return;

	float X = 30.f;
	float Y = 230.f;
	float PW = 110.f, PH = 28.f;

	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, PW, PH);

	// Left accent stripe (orange for kills)
	FLinearColor Acc(1.f, 0.5f, 0.1f, 0.5f);
	DrawRect(Acc, X - 5.f, Y - 3.f, 2.f, PH);

	FString KillText = FString::Printf(TEXT("Kills: %d"), PS->Kills);
	DrawText(KillText, ColorWhite, X + 2.f, Y, HUDFont, 1.f);
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

	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = FMath::Abs(FMath::Sin(Time * 3.f));
	float A = 0.8f + Pulse * 0.2f;
	FLinearColor C = (Streak >= 8) ? FLinearColor(1.f, 0.2f, 0.2f, A)
		: (Streak >= 5) ? FLinearColor(1.f, 0.5f, 0.1f, A)
		: FLinearColor(1.f, 0.8f, 0.2f, A);

	float X = 30.f, Y = 262.f;
	float PW = TextW + 20.f, PH = TextH + 8.f;
	DrawRect(FLinearColor(0.02f, 0.01f, 0.f, 0.6f), X - 5.f, Y - 3.f, PW, PH);

	// Left accent stripe in streak color
	DrawRect(FLinearColor(C.R, C.G, C.B, 0.6f), X - 5.f, Y - 3.f, 2.f, PH);
	// Top accent line
	DrawRect(FLinearColor(C.R, C.G, C.B, 0.3f), X - 5.f, Y - 3.f, PW, 1.f);

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

	if (GS->MatchPhase != EBRMatchPhase::Playing && GS->MatchPhase != EBRMatchPhase::ZoneShrinking)
		return;

	int32 TotalSec = FMath::FloorToInt(GS->MatchElapsedTime);
	int32 Minutes = TotalSec / 60;
	int32 Seconds = TotalSec % 60;
	FString TimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

	float TW, TH;
	GetTextSize(TimeText, TW, TH, HUDFont, 1.2f);
	float PW = TW + 30.f;
	float PH = TH + 10.f;
	float X = (Canvas->SizeX - PW) * 0.5f;
	float Y = 8.f;

	// Panel
	DrawRect(ColorBgDark, X, Y, PW, PH);
	// Top accent bar (cyan)
	DrawRect(FLinearColor(0.f, 0.5f, 0.8f, 0.35f), X, Y, PW, 1.f);
	// Bottom accent
	DrawRect(FLinearColor(0.f, 0.5f, 0.8f, 0.2f), X, Y + PH - 1.f, PW, 1.f);

	DrawText(TimeText, ColorWhite, X + (PW - TW) * 0.5f, Y + (PH - TH) * 0.5f, HUDFont, 1.2f);
}

void AExoHUD::DrawZoneTimer()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	if (GS->MatchPhase != EBRMatchPhase::Playing && GS->MatchPhase != EBRMatchPhase::ZoneShrinking)
		return;

	if (GS->CurrentZoneStage < 0) return;

	int32 Ring = GS->CurrentZoneStage + 1;
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

	float X = MinimapConfig.ScreenX;
	float Y = MinimapConfig.ScreenY + MinimapConfig.Size + 8.f;

	float TW, TH;
	GetTextSize(ZoneText, TW, TH, HUDFont, 0.8f);
	float PanelW = FMath::Max(TW + 20.f, 200.f);
	float PanelH = TH + 8.f;

	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, PanelW, PanelH);

	// Left accent stripe matching zone color
	DrawRect(FLinearColor(ZoneColor.R, ZoneColor.G, ZoneColor.B, 0.5f),
		X - 5.f, Y - 3.f, 2.f, PanelH);

	DrawText(ZoneText, ZoneColor, X, Y, HUDFont, 0.8f);

	if (!SubText.IsEmpty())
	{
		float SY = Y + TH + 6.f;
		float SW, SH;
		GetTextSize(SubText, SW, SH, HUDFont, 0.65f);
		float SubPW = FMath::Max(SW + 20.f, 200.f);
		DrawRect(ColorBgDark, X - 5.f, SY - 2.f, SubPW, SH + 4.f);
		DrawText(SubText, SubColor, X, SY, HUDFont, 0.65f);
	}
}
