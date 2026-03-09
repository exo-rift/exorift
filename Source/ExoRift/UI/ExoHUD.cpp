#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Weapons/ExoWeaponBase.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Map/ExoZoneSystem.h"
#include "UI/ExoDamageNumbers.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

const FLinearColor AExoHUD::ColorHealthGreen = FLinearColor(0.1f, 0.9f, 0.2f, 0.9f);
const FLinearColor AExoHUD::ColorHealthRed = FLinearColor(0.9f, 0.1f, 0.1f, 0.9f);
const FLinearColor AExoHUD::ColorHeatCool = FLinearColor(0.2f, 0.4f, 0.9f, 0.9f);
const FLinearColor AExoHUD::ColorHeatHot = FLinearColor(0.9f, 0.6f, 0.1f, 0.9f);
const FLinearColor AExoHUD::ColorHeatOverheat = FLinearColor(1.f, 0.1f, 0.1f, 1.f);
const FLinearColor AExoHUD::ColorZoneWarning = FLinearColor(1.f, 0.3f, 0.3f, 1.f);
const FLinearColor AExoHUD::ColorBgDark = FLinearColor(0.05f, 0.05f, 0.08f, 0.7f);
const FLinearColor AExoHUD::ColorWhite = FLinearColor(0.9f, 0.92f, 0.95f, 1.f);

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

	DrawCrosshair();
	DrawHealthBar();
	DrawOverheatBar();
	DrawAliveCount();
	DrawKillFeed();
	DrawMatchPhase();
	DrawZoneWarning();
	DrawKillCount();

	// Floating damage numbers
	AExoDamageNumbers* DmgNums = AExoDamageNumbers::Get(GetWorld());
	if (DmgNums)
	{
		DmgNums->DrawNumbers(this, Canvas, HUDFont);
	}

	// Minimap
	FExoMinimap::Draw(this, Canvas, MinimapConfig);
}

void AExoHUD::DrawCrosshair()
{
	FVector2D Center = GetScreenCenter();
	float Size = 12.f + CrosshairSpread;
	float Gap = 4.f + CrosshairSpread * 0.5f;
	float Thickness = 2.f;

	Canvas->SetDrawColor(220, 235, 240, 200);

	// Top
	DrawLine(Center.X, Center.Y - Gap - Size, Center.X, Center.Y - Gap, FLinearColor::White, Thickness);
	// Bottom
	DrawLine(Center.X, Center.Y + Gap, Center.X, Center.Y + Gap + Size, FLinearColor::White, Thickness);
	// Left
	DrawLine(Center.X - Gap - Size, Center.Y, Center.X - Gap, Center.Y, FLinearColor::White, Thickness);
	// Right
	DrawLine(Center.X + Gap, Center.Y, Center.X + Gap + Size, Center.Y, FLinearColor::White, Thickness);

	// Center dot
	DrawRect(ColorWhite, Center.X - 1.f, Center.Y - 1.f, 2.f, 2.f);
}

void AExoHUD::DrawHealthBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	float HealthPct = Char->GetHealth() / Char->GetMaxHealth();
	FLinearColor HealthColor = FMath::Lerp(ColorHealthRed, ColorHealthGreen, HealthPct);

	float BarW = 250.f;
	float BarH = 20.f;
	float X = 30.f;
	float Y = Canvas->SizeY - 60.f;

	DrawProgressBar(X, Y, BarW, BarH, HealthPct, HealthColor, ColorBgDark);

	// Health text
	FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Char->GetHealth()));
	Canvas->SetDrawColor(255, 255, 255, 220);
	DrawText(HealthText, ColorWhite, X + BarW + 10.f, Y + 1.f, HUDFont, 1.f);
}

void AExoHUD::DrawOverheatBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetCurrentWeapon()) return;

	AExoWeaponBase* Weapon = Char->GetCurrentWeapon();
	float HeatPct = Weapon->GetCurrentHeat();
	bool bOverheated = Weapon->IsOverheated();

	// Heat color: blue -> orange -> red
	FLinearColor HeatColor;
	if (bOverheated)
	{
		// Pulsing red when overheated
		float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 4.f));
		HeatColor = FMath::Lerp(ColorHeatOverheat, FLinearColor(1.f, 0.5f, 0.2f, 1.f), Pulse);
	}
	else if (HeatPct > 0.7f)
	{
		HeatColor = FMath::Lerp(ColorHeatHot, ColorHeatOverheat, (HeatPct - 0.7f) / 0.3f);
	}
	else
	{
		HeatColor = FMath::Lerp(ColorHeatCool, ColorHeatHot, HeatPct / 0.7f);
	}

	float BarW = 300.f;
	float BarH = 16.f;
	float X = (Canvas->SizeX - BarW) * 0.5f;
	float Y = Canvas->SizeY - 50.f;

	DrawProgressBar(X, Y, BarW, BarH, HeatPct, HeatColor, ColorBgDark);

	// Weapon name above bar
	FString WeaponLabel = Weapon->GetWeaponName();
	if (bOverheated) WeaponLabel += TEXT(" [OVERHEATED]");
	float TextW, TextH;
	GetTextSize(WeaponLabel, TextW, TextH, HUDFont, 0.8f);
	DrawText(WeaponLabel, ColorWhite, X + (BarW - TextW) * 0.5f, Y - TextH - 4.f, HUDFont, 0.8f);
}

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
		FLinearColor TextColor = ColorWhite;
		TextColor.A = Alpha;

		FString KillText = FString::Printf(TEXT("%s [%s] %s"),
			*Entry.KillerName, *Entry.WeaponName, *Entry.VictimName);
		DrawText(KillText, TextColor, X, Y, HUDFont, 0.75f);
		Y += 22.f;
	}
}

void AExoHUD::DrawMatchPhase()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	FString PhaseText;
	switch (GS->MatchPhase)
	{
	case EBRMatchPhase::WaitingForPlayers: PhaseText = TEXT("WAITING FOR PLAYERS..."); break;
	case EBRMatchPhase::DropPhase: PhaseText = TEXT("DROP!"); break;
	case EBRMatchPhase::Playing: return; // No text during normal play
	case EBRMatchPhase::ZoneShrinking: PhaseText = TEXT("ZONE SHRINKING"); break;
	case EBRMatchPhase::EndGame: PhaseText = TEXT("MATCH OVER"); break;
	}

	float TextW, TextH;
	GetTextSize(PhaseText, TextW, TextH, HUDFont, 1.5f);
	float X = (Canvas->SizeX - TextW) * 0.5f;
	DrawText(PhaseText, ColorWhite, X, 40.f, HUDFont, 1.5f);
}

void AExoHUD::DrawZoneWarning()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	// Check if outside zone
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

	FString WarningText = TEXT("OUTSIDE SAFE ZONE");
	float TextW, TextH;
	GetTextSize(WarningText, TextW, TextH, HUDFont, 1.2f);
	float X = (Canvas->SizeX - TextW) * 0.5f;
	DrawText(WarningText, WarningColor, X, Canvas->SizeY * 0.3f, HUDFont, 1.2f);
}

void AExoHUD::DrawKillCount()
{
	AExoPlayerState* PS = GetOwningPawn() ?
		GetOwningPawn()->GetPlayerState<AExoPlayerState>() : nullptr;
	if (!PS) return;

	FString KillText = FString::Printf(TEXT("Kills: %d"), PS->Kills);
	DrawText(KillText, ColorWhite, 30.f, 30.f, HUDFont, 1.f);
}

FVector2D AExoHUD::GetScreenCenter() const
{
	return FVector2D(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f);
}

void AExoHUD::DrawProgressBar(float X, float Y, float Width, float Height,
	float Pct, FLinearColor FillColor, FLinearColor BgColor)
{
	// Background
	DrawRect(BgColor, X, Y, Width, Height);
	// Fill
	DrawRect(FillColor, X + 1.f, Y + 1.f, (Width - 2.f) * FMath::Clamp(Pct, 0.f, 1.f), Height - 2.f);
	// Border
	FLinearColor BorderColor(0.3f, 0.35f, 0.4f, 0.6f);
	DrawLine(X, Y, X + Width, Y, BorderColor);
	DrawLine(X, Y + Height, X + Width, Y + Height, BorderColor);
	DrawLine(X, Y, X, Y + Height, BorderColor);
	DrawLine(X + Width, Y, X + Width, Y + Height, BorderColor);
}
