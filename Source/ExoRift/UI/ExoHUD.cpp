#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Map/ExoZoneSystem.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "UI/ExoMatchSummary.h"
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

	// Check if match is over — show summary instead
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (GS && GS->MatchPhase == EBRMatchPhase::EndGame)
	{
		FExoMatchSummary::Draw(this, Canvas, HUDFont);
		return;
	}

	DrawCrosshair();
	DrawHealthBar();
	DrawShieldBar();
	DrawOverheatBar();
	DrawWeaponIndicator();
	DrawAliveCount();
	DrawKillFeed();
	DrawMatchPhase();
	DrawZoneWarning();
	DrawKillCount();

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
}

void AExoHUD::DrawCrosshair()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (Char && Char->GetCurrentWeapon())
	{
		// Dynamic crosshair spread based on weapon state
		CrosshairSpread = FMath::FInterpTo(CrosshairSpread,
			Char->GetCurrentWeapon()->GetCurrentHeat() * 15.f,
			GetWorld()->GetDeltaSeconds(), 10.f);
	}

	FVector2D Center = GetScreenCenter();
	float Size = 12.f + CrosshairSpread;
	float Gap = 4.f + CrosshairSpread * 0.5f;
	float Thickness = 2.f;

	FLinearColor CrossColor = ColorWhite;
	CrossColor.A = 0.8f;

	// Top
	DrawLine(Center.X, Center.Y - Gap - Size, Center.X, Center.Y - Gap, CrossColor, Thickness);
	// Bottom
	DrawLine(Center.X, Center.Y + Gap, Center.X, Center.Y + Gap + Size, CrossColor, Thickness);
	// Left
	DrawLine(Center.X - Gap - Size, Center.Y, Center.X - Gap, Center.Y, CrossColor, Thickness);
	// Right
	DrawLine(Center.X + Gap, Center.Y, Center.X + Gap + Size, Center.Y, CrossColor, Thickness);

	// Center dot
	DrawRect(ColorWhite, Center.X - 1.f, Center.Y - 1.f, 2.f, 2.f);
}

void AExoHUD::DrawHealthBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	float HealthPct = Char->GetHealth() / Char->GetMaxHealth();
	FLinearColor HealthColor = FMath::Lerp(ColorHealthRed, ColorHealthGreen, HealthPct);

	// Pulse when low health
	if (HealthPct < 0.25f)
	{
		float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 4.f));
		HealthColor = FMath::Lerp(HealthColor, ColorHealthRed, Pulse * 0.5f);
	}

	float BarW = 250.f;
	float BarH = 20.f;
	float X = 30.f;
	float Y = Canvas->SizeY - 60.f;

	DrawProgressBar(X, Y, BarW, BarH, HealthPct, HealthColor, ColorBgDark);

	// Health icon + text
	FString HealthText = FString::Printf(TEXT("HP %d"), FMath::CeilToInt(Char->GetHealth()));
	DrawText(HealthText, ColorWhite, X + BarW + 10.f, Y + 1.f, HUDFont, 0.9f);
}

void AExoHUD::DrawShieldBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetShieldComponent()) return;

	UExoShieldComponent* Shield = Char->GetShieldComponent();
	float ShieldPct = Shield->GetShieldPercent();
	if (ShieldPct <= 0.f && !Shield->HasShield()) return; // Don't draw if no shield at all

	float BarW = 250.f;
	float BarH = 12.f;
	float X = 30.f;
	float Y = Canvas->SizeY - 85.f; // Above health bar

	FLinearColor ShieldColor = ColorShieldBlue;
	if (ShieldPct < 0.2f)
	{
		float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 5.f));
		ShieldColor.A = 0.5f + Pulse * 0.4f;
	}

	DrawProgressBar(X, Y, BarW, BarH, ShieldPct, ShieldColor, ColorBgDark);

	FString ShieldText = FString::Printf(TEXT("SH %d"), FMath::CeilToInt(Shield->GetCurrentShield()));
	DrawText(ShieldText, ColorShieldBlue, X + BarW + 10.f, Y - 1.f, HUDFont, 0.75f);
}

void AExoHUD::DrawOverheatBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetCurrentWeapon()) return;

	AExoWeaponBase* Weapon = Char->GetCurrentWeapon();
	float HeatPct = Weapon->GetCurrentHeat();
	bool bOverheated = Weapon->IsOverheated();

	FLinearColor HeatColor;
	if (bOverheated)
	{
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

void AExoHUD::DrawWeaponIndicator()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	// Show current weapon slot indicator in bottom right
	float X = Canvas->SizeX - 150.f;
	float Y = Canvas->SizeY - 80.f;

	DrawRect(ColorBgDark, X - 5.f, Y - 5.f, 130.f, 60.f);

	AExoWeaponBase* Weapon = Char->GetCurrentWeapon();
	if (Weapon)
	{
		FString TypeStr;
		switch (Weapon->GetWeaponType())
		{
		case EWeaponType::Rifle: TypeStr = TEXT("[1] RIFLE"); break;
		case EWeaponType::Pistol: TypeStr = TEXT("[2] PISTOL"); break;
		case EWeaponType::GrenadeLauncher: TypeStr = TEXT("[3] LAUNCHER"); break;
		}
		DrawText(TypeStr, ColorWhite, X, Y, HUDFont, 0.8f);
		DrawText(Weapon->GetWeaponName(), FLinearColor(0.6f, 0.7f, 0.8f, 0.8f), X, Y + 22.f, HUDFont, 0.7f);
	}
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

	FString PhaseText;
	FLinearColor PhaseColor = ColorWhite;

	switch (GS->MatchPhase)
	{
	case EBRMatchPhase::WaitingForPlayers:
		PhaseText = TEXT("WAITING FOR PLAYERS...");
		break;
	case EBRMatchPhase::DropPhase:
		PhaseText = TEXT("DEPLOYING DROP PODS");
		PhaseColor = FLinearColor(0.3f, 0.8f, 1.f, 1.f);
		break;
	case EBRMatchPhase::Playing:
		return; // No text during normal play
	case EBRMatchPhase::ZoneShrinking:
		PhaseText = TEXT("ZONE COLLAPSING");
		PhaseColor = FLinearColor(1.f, 0.5f, 0.2f, 1.f);
		break;
	case EBRMatchPhase::EndGame:
		return; // Handled by match summary
	}

	float TextW, TextH;
	GetTextSize(PhaseText, TextW, TextH, HUDFont, 1.5f);
	float X = (Canvas->SizeX - TextW) * 0.5f;

	// Subtle background
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), X - 15.f, 32.f, TextW + 30.f, TextH + 16.f);
	DrawText(PhaseText, PhaseColor, X, 40.f, HUDFont, 1.5f);
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
	float X = (Canvas->SizeX - TextW) * 0.5f;
	DrawText(WarningText, WarningColor, X, Canvas->SizeY * 0.25f, HUDFont, 1.1f);
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
