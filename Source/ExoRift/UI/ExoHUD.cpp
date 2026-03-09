#include "UI/ExoHUD.h"
#include "UI/ExoSettingsMenu.h"
#include "Core/ExoGameSettings.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoArmorComponent.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoSupplyDropManager.h"
#include "Map/ExoSupplyDrop.h"
#include "UI/ExoDamageNumbers.h"
#include "UI/ExoHitMarker.h"
#include "UI/ExoPingSystem.h"
#include "UI/ExoCommsWheel.h"
#include "UI/ExoMatchSummary.h"
#include "Player/ExoKillStreakComponent.h"
#include "Player/ExoAbilityComponent.h"
#include "Visual/ExoWeatherSystem.h"
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

	// Ping indicators
	{
		AExoCharacter* PingChar = Cast<AExoCharacter>(GetOwningPawn());
		FVector ViewLoc = PingChar ? PingChar->GetActorLocation() : FVector::ZeroVector;
		FExoPingSystem::DrawPings(this, Canvas, HUDFont, ViewLoc);
	}

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

	// FPS counter (drawn on top of gameplay HUD)
	DrawFPS();

	// Comms wheel overlay
	if (FExoCommsWheel::bIsOpen)
	{
		FExoCommsWheel::Draw(this, Canvas, HUDFont);
	}

	// Settings menu overlay — drawn last so it covers everything
	if (FExoSettingsMenu::bIsOpen)
	{
		FExoSettingsMenu::Draw(this, Canvas, HUDFont);
	}
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

void AExoHUD::DrawArmorIndicators()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetArmorComponent()) return;

	UExoArmorComponent* Armor = Char->GetArmorComponent();
	bool bHasHelmet = Armor->GetHelmetTier() != EArmorTier::None;
	bool bHasVest = Armor->GetVestTier() != EArmorTier::None;
	if (!bHasHelmet && !bHasVest) return;

	// Position: small indicators above the shield bar
	float X = 30.f;
	float Y = Canvas->SizeY - 108.f;
	float BarW = 115.f;
	float BarH = 10.f;

	// Tier color helper
	auto GetTierColor = [](EArmorTier T) -> FLinearColor
	{
		switch (T)
		{
		case EArmorTier::Light:  return FLinearColor(0.6f, 0.6f, 0.65f, 0.9f);
		case EArmorTier::Medium: return FLinearColor(0.3f, 0.55f, 1.f, 0.9f);
		case EArmorTier::Heavy:  return FLinearColor(1.f, 0.85f, 0.25f, 0.9f);
		default:                 return FLinearColor(0.4f, 0.4f, 0.4f, 0.5f);
		}
	};

	if (bHasHelmet)
	{
		FLinearColor Col = GetTierColor(Armor->GetHelmetTier());
		DrawText(TEXT("HELM"), Col, X, Y - 1.f, HUDFont, 0.6f);
		DrawProgressBar(X + 42.f, Y, BarW, BarH, Armor->GetHelmetPercent(), Col, ColorBgDark);
	}

	if (bHasVest)
	{
		float VY = bHasHelmet ? Y - 15.f : Y;
		FLinearColor Col = GetTierColor(Armor->GetVestTier());
		DrawText(TEXT("VEST"), Col, X, VY - 1.f, HUDFont, 0.6f);
		DrawProgressBar(X + 42.f, VY, BarW, BarH, Armor->GetVestPercent(), Col, ColorBgDark);
	}
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

	UExoInventoryComponent* Inv = Char->GetInventoryComponent();
	if (!Inv) return;

	const int32 Slots = Inv->GetSlotCount();
	const float SlotW = 120.f;
	const float SlotH = 42.f;
	const float SlotGap = 4.f;
	const float TotalH = Slots * SlotH + (Slots - 1) * SlotGap;
	const float BaseX = Canvas->SizeX - SlotW - 20.f;
	const float BaseY = Canvas->SizeY - TotalH - 100.f;

	const TCHAR* SlotLabels[] = { TEXT("[1] PRI"), TEXT("[2] SEC"), TEXT("[3] UTL") };
	const int32 ActiveSlot = Inv->GetCurrentSlotIndex();

	for (int32 i = 0; i < Slots; ++i)
	{
		float Y = BaseY + i * (SlotH + SlotGap);
		AExoWeaponBase* W = Inv->GetWeapon(i);
		bool bActive = (i == ActiveSlot);

		// Background: brighter for active slot, dimmed for others
		FLinearColor BgCol = bActive
			? FLinearColor(0.1f, 0.1f, 0.15f, 0.85f)
			: FLinearColor(0.03f, 0.03f, 0.05f, 0.5f);
		DrawRect(BgCol, BaseX, Y, SlotW, SlotH);

		if (W)
		{
			FLinearColor RarityCol = AExoWeaponBase::GetRarityColor(W->Rarity);
			if (!bActive) RarityCol.A = 0.5f;

			// Left rarity stripe (3px wide)
			DrawRect(RarityCol, BaseX, Y, 3.f, SlotH);

			// Slot label + weapon name
			DrawText(SlotLabels[i], RarityCol, BaseX + 8.f, Y + 2.f, HUDFont, 0.65f);
			FLinearColor NameCol = bActive ? RarityCol : (RarityCol * 0.7f);
			DrawText(W->GetWeaponName(), NameCol, BaseX + 8.f, Y + 20.f, HUDFont, 0.6f);
		}
		else
		{
			// Empty slot
			FLinearColor DimCol(0.25f, 0.25f, 0.3f, bActive ? 0.6f : 0.3f);
			DrawText(SlotLabels[i], DimCol, BaseX + 8.f, Y + 2.f, HUDFont, 0.65f);
			DrawText(TEXT("- empty -"), DimCol * 0.7f, BaseX + 8.f, Y + 20.f, HUDFont, 0.55f);
		}

		// Active slot border highlight
		if (bActive)
		{
			FLinearColor Border(0.6f, 0.8f, 1.f, 0.7f);
			DrawLine(BaseX, Y, BaseX + SlotW, Y, Border);
			DrawLine(BaseX, Y + SlotH, BaseX + SlotW, Y + SlotH, Border);
			DrawLine(BaseX, Y, BaseX, Y + SlotH, Border);
			DrawLine(BaseX + SlotW, Y, BaseX + SlotW, Y + SlotH, Border);
		}
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

void AExoHUD::DrawInteractionPrompt()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	UExoInteractionComponent* InterComp = Char->GetInteractionComponent();
	if (!InterComp) return;

	FString Prompt = InterComp->GetCurrentPrompt();
	if (Prompt.IsEmpty()) return;

	float TextW, TextH;
	GetTextSize(Prompt, TextW, TextH, HUDFont, 0.9f);
	float X = (Canvas->SizeX - TextW) * 0.5f;
	float Y = Canvas->SizeY * 0.6f;

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), X - 10.f, Y - 4.f, TextW + 20.f, TextH + 8.f);
	DrawText(Prompt, ColorWhite, X, Y, HUDFont, 0.9f);
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

void AExoHUD::DrawWeatherIndicator()
{
	AExoWeatherSystem* Weather = AExoWeatherSystem::Get(GetWorld());
	if (!Weather) return;

	// Position: top-left, below kill streak area (Y ~292)
	float X = 30.f;
	float Y = 294.f;

	FString Label;
	FLinearColor Col = ColorWhite;
	switch (Weather->GetCurrentWeather())
	{
	case EExoWeatherState::Clear:    Label = TEXT("CLEAR");    Col = FLinearColor(0.7f, 0.9f, 1.f, 0.8f); break;
	case EExoWeatherState::Overcast: Label = TEXT("OVERCAST"); Col = FLinearColor(0.6f, 0.6f, 0.65f, 0.8f); break;
	case EExoWeatherState::Rain:     Label = TEXT("RAIN");     Col = FLinearColor(0.3f, 0.5f, 0.9f, 0.9f); break;
	case EExoWeatherState::Storm:    Label = TEXT("STORM");    Col = FLinearColor(1.f, 0.4f, 0.2f, 0.9f); break;
	case EExoWeatherState::Fog:      Label = TEXT("FOG");      Col = FLinearColor(0.7f, 0.7f, 0.75f, 0.9f); break;
	}

	float TW, TH;
	GetTextSize(Label, TW, TH, HUDFont, 0.85f);
	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, TW + 10.f, TH + 6.f);
	DrawText(Label, Col, X, Y, HUDFont, 0.85f);
}

void AExoHUD::DrawAbilities()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetAbilityComponent()) return;
	const TArray<FExoAbility>& Abilities = Char->GetAbilityComponent()->GetAbilities();
	if (Abilities.Num() == 0) return;

	const float SlotW = 60.f, SlotH = 50.f, SlotGap = 8.f;
	float TotalW = Abilities.Num() * SlotW + (Abilities.Num() - 1) * SlotGap;
	float StartX = (Canvas->SizeX - TotalW) * 0.5f;
	float Y = Canvas->SizeY - 95.f;

	const TCHAR* Icons[] = { TEXT("D"), TEXT("S"), TEXT("B") };
	const FLinearColor ReadyColors[] = {
		FLinearColor(0.f, 0.9f, 1.f, 0.9f),  // Cyan - Dash
		FLinearColor(1.f, 0.9f, 0.1f, 0.9f),  // Yellow - Scan
		FLinearColor(0.2f, 0.5f, 1.f, 0.9f)   // Blue - Shield
	};
	const FLinearColor Grey(0.3f, 0.3f, 0.35f, 0.7f);

	for (int32 i = 0; i < Abilities.Num(); ++i)
	{
		const FExoAbility& Ab = Abilities[i];
		float X = StartX + i * (SlotW + SlotGap);
		bool bReady = Ab.bIsReady();
		FLinearColor SlotCol = bReady ? ReadyColors[i] : Grey;
		DrawRect(ColorBgDark, X, Y, SlotW, SlotH);
		DrawLine(X, Y, X + SlotW, Y, SlotCol); DrawLine(X, Y + SlotH, X + SlotW, Y + SlotH, SlotCol);
		DrawLine(X, Y, X, Y + SlotH, SlotCol); DrawLine(X + SlotW, Y, X + SlotW, Y + SlotH, SlotCol);
		DrawText(Icons[i], SlotCol, X + 22.f, Y + 4.f, HUDFont, 1.2f);
		if (!bReady)
		{
			float Pct = Ab.CooldownRemaining / Ab.Cooldown;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), X + 1.f, Y + 1.f, (SlotW - 2.f) * Pct, SlotH - 2.f);
			FString CdText = FString::Printf(TEXT("%.0f"), Ab.CooldownRemaining);
			DrawText(CdText, ColorWhite, X + 18.f, Y + 30.f, HUDFont, 0.7f);
		}
		else
		{
			float Glow = 0.3f + 0.15f * FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 2.f));
			FLinearColor GlowCol = SlotCol; GlowCol.A = Glow;
			DrawRect(GlowCol, X + 1.f, Y + 1.f, SlotW - 2.f, SlotH - 2.f);
		}
	}
}

void AExoHUD::DrawDBNOOverlay()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->IsDBNO()) return;
	float Pct = Char->GetDBNOHealth() / 100.f;
	float P = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f));
	// Red vignette intensifying as DBNO health drops
	FLinearColor VC(1.f, 0.f, 0.f, 0.15f + (1.f - Pct) * 0.25f + P * 0.05f);
	float E = 120.f + (1.f - Pct) * 80.f;
	DrawRect(VC, 0.f, 0.f, E, Canvas->SizeY); DrawRect(VC, Canvas->SizeX - E, 0.f, E, Canvas->SizeY);
	DrawRect(VC, 0.f, 0.f, Canvas->SizeX, E * 0.5f); DrawRect(VC, 0.f, Canvas->SizeY - E * 0.5f, Canvas->SizeX, E * 0.5f);
	// Pulsing downed text
	FString DT = TEXT("DOWNED  —  BLEEDING OUT");
	float TW, TH; GetTextSize(DT, TW, TH, HUDFont, 1.4f);
	float X = (Canvas->SizeX - TW) * 0.5f, Y = Canvas->SizeY * 0.35f;
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), X - 15.f, Y - 5.f, TW + 30.f, TH + 10.f);
	DrawText(DT, FLinearColor(1.f, 0.15f, 0.15f, 0.6f + P * 0.4f), X, Y, HUDFont, 1.4f);
	// DBNO blood bar over health bar position
	FLinearColor BC = FMath::Lerp(FLinearColor(0.5f, 0.f, 0.f, 0.9f), FLinearColor(0.9f, 0.1f, 0.1f, 0.9f), P * 0.5f);
	DrawProgressBar(30.f, Canvas->SizeY - 60.f, 250.f, 20.f, Pct, BC, ColorBgDark);
	DrawText(FString::Printf(TEXT("DBNO %d"), FMath::CeilToInt(Char->GetDBNOHealth())),
		FLinearColor(1.f, 0.3f, 0.3f, 1.f), 290.f, Canvas->SizeY - 59.f, HUDFont, 0.9f);
}

void AExoHUD::DrawEnergyBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetCurrentWeapon()) return;
	AExoWeaponBase* W = Char->GetCurrentWeapon();
	float Pct = W->GetEnergyPercent();
	bool bLow = Pct < 0.2f;
	FLinearColor EC(0.3f, 0.9f, 0.4f, 0.9f);
	if (bLow)
	{
		float F = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 5.f));
		EC = FMath::Lerp(FLinearColor(0.9f, 0.2f, 0.1f, 0.9f), FLinearColor(1.f, 0.6f, 0.1f, 1.f), F);
	}
	float BarW = 300.f, X = (Canvas->SizeX - BarW) * 0.5f, Y = Canvas->SizeY - 70.f;
	DrawProgressBar(X, Y, BarW, 10.f, Pct, EC, ColorBgDark);
	FString EL = FString::Printf(TEXT("ENERGY: %d/%d"), FMath::CeilToInt(W->GetCurrentEnergy()), FMath::CeilToInt(W->GetMaxEnergy()));
	float TW, TH; GetTextSize(EL, TW, TH, HUDFont, 0.7f);
	DrawText(EL, bLow ? EC : ColorWhite, X + (BarW - TW) * 0.5f, Y - TH - 2.f, HUDFont, 0.7f);
}

void AExoHUD::DrawSupplyDropMarkers()
{
	AExoSupplyDropManager* Manager = nullptr;
	for (TActorIterator<AExoSupplyDropManager> It(GetWorld()); It; ++It)
	{
		Manager = *It;
		break;
	}
	if (!Manager) return;

	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	FVector ViewLoc = Char ? Char->GetActorLocation() : FVector::ZeroVector;
	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const FLinearColor DropColor(1.f, 0.85f, 0.15f, 0.9f); // Yellow
	constexpr float EdgeMargin = 40.f;

	for (AExoSupplyDrop* Drop : Manager->GetActiveDrops())
	{
		if (!Drop) continue;
		ESupplyDropState State = Drop->GetState();
		if (State == ESupplyDropState::Depleted) continue;

		FVector DropLoc = Drop->GetActorLocation();
		float DistMeters = FVector::Dist(ViewLoc, DropLoc) / 100.f;

		FVector2D ScreenPos;
		bool bOnScreen = GetOwningPlayerController()->ProjectWorldLocationToScreen(DropLoc, ScreenPos, true);
		bool bInBounds = bOnScreen
			&& ScreenPos.X >= EdgeMargin && ScreenPos.X <= ScreenW - EdgeMargin
			&& ScreenPos.Y >= EdgeMargin && ScreenPos.Y <= ScreenH - EdgeMargin;

		if (bInBounds)
		{
			// Draw yellow diamond marker
			float Sz = 8.f;
			DrawLine(ScreenPos.X, ScreenPos.Y - Sz, ScreenPos.X + Sz, ScreenPos.Y, DropColor, 2.f);
			DrawLine(ScreenPos.X + Sz, ScreenPos.Y, ScreenPos.X, ScreenPos.Y + Sz, DropColor, 2.f);
			DrawLine(ScreenPos.X, ScreenPos.Y + Sz, ScreenPos.X - Sz, ScreenPos.Y, DropColor, 2.f);
			DrawLine(ScreenPos.X - Sz, ScreenPos.Y, ScreenPos.X, ScreenPos.Y - Sz, DropColor, 2.f);

			FString Label = (State == ESupplyDropState::Falling)
				? FString::Printf(TEXT("DROP [%dm]"), FMath::RoundToInt(DistMeters))
				: FString::Printf(TEXT("SUPPLY [%dm]"), FMath::RoundToInt(DistMeters));
			float TW, TH;
			GetTextSize(Label, TW, TH, HUDFont, 0.7f);
			float TX = ScreenPos.X - TW * 0.5f;
			float TY = ScreenPos.Y - 22.f;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), TX - 4.f, TY - 2.f, TW + 8.f, TH + 4.f);
			DrawText(Label, DropColor, TX, TY, HUDFont, 0.7f);
		}
		else
		{
			// Off-screen edge indicator
			FVector2D Center(ScreenW * 0.5f, ScreenH * 0.5f);
			FVector2D Dir = bOnScreen ? (ScreenPos - Center) : (Center - ScreenPos);
			if (Dir.IsNearlyZero()) Dir = FVector2D(0.f, -1.f);
			Dir.Normalize();

			float HalfW = ScreenW * 0.5f - EdgeMargin;
			float HalfH = ScreenH * 0.5f - EdgeMargin;
			float SX = (FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER) ? HalfW / FMath::Abs(Dir.X) : BIG_NUMBER;
			float SY = (FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER) ? HalfH / FMath::Abs(Dir.Y) : BIG_NUMBER;
			FVector2D EdgePos = Center + Dir * FMath::Min(FMath::Min(SX, SY), 1000.f);

			float Sz = 6.f;
			DrawLine(EdgePos.X, EdgePos.Y - Sz, EdgePos.X + Sz, EdgePos.Y, DropColor, 2.f);
			DrawLine(EdgePos.X + Sz, EdgePos.Y, EdgePos.X, EdgePos.Y + Sz, DropColor, 2.f);
			DrawLine(EdgePos.X, EdgePos.Y + Sz, EdgePos.X - Sz, EdgePos.Y, DropColor, 2.f);
			DrawLine(EdgePos.X - Sz, EdgePos.Y, EdgePos.X, EdgePos.Y - Sz, DropColor, 2.f);

			FString DT = FString::Printf(TEXT("%dm"), FMath::RoundToInt(DistMeters));
			float DW, DH;
			GetTextSize(DT, DW, DH, HUDFont, 0.6f);
			DrawText(DT, DropColor, EdgePos.X - DW * 0.5f, EdgePos.Y + 10.f, HUDFont, 0.6f);
		}
	}
}

void AExoHUD::DrawSupplyDropAnnouncement()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS || GS->AnnouncementTimer <= 0.f) return;

	GS->AnnouncementTimer -= GetWorld()->GetDeltaSeconds();

	float Alpha = FMath::Clamp(GS->AnnouncementTimer / 1.5f, 0.f, 1.f);
	FLinearColor AnnColor(1.f, 0.85f, 0.15f, Alpha);

	FString AnnText = GS->SupplyDropAnnouncement;
	float TW, TH;
	GetTextSize(AnnText, TW, TH, HUDFont, 1.3f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.2f;

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f * Alpha), X - 15.f, Y - 5.f, TW + 30.f, TH + 10.f);
	DrawText(AnnText, AnnColor, X, Y, HUDFont, 1.3f);

	if (GS->AnnouncementTimer <= 0.f)
	{
		GS->SupplyDropAnnouncement.Empty();
	}
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
