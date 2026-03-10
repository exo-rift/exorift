// ExoHUDPlayer.cpp — Player status elements: health, shield, armor, energy, DBNO
#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoArmorComponent.h"
#include "Player/ExoInteractionComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Engine/Canvas.h"

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

void AExoHUD::DrawEnergyBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetCurrentWeapon()) return;
	AExoWeaponBase* W = Char->GetCurrentWeapon();
	float Pct = W->GetEnergyPercent();
	bool bLow = Pct < 0.2f;
	bool bEmpty = Pct < 0.01f;
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

	// Dramatic "NO ENERGY" warning when empty
	if (bEmpty)
	{
		float Pulse = FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 6.f));
		FLinearColor Warn(1.f, 0.15f, 0.1f, 0.5f + 0.5f * Pulse);
		FString NoAmmo = TEXT("NO ENERGY — SWAP WEAPON");
		float NW, NH; GetTextSize(NoAmmo, NW, NH, HUDFont, 1.1f);
		float NX = (Canvas->SizeX - NW) * 0.5f;
		float NY = Canvas->SizeY * 0.6f;
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f * Pulse), NX - 10.f, NY - 4.f, NW + 20.f, NH + 8.f);
		DrawText(NoAmmo, Warn, NX, NY, HUDFont, 1.1f);
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

void AExoHUD::DrawSprintLines()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float TargetAlpha = Char->IsSprinting() ? 1.f : 0.f;
	SprintLineAlpha = FMath::FInterpTo(SprintLineAlpha, TargetAlpha, DeltaTime, 6.f);
	if (SprintLineAlpha < 0.02f) return;

	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;
	float MaxR = FMath::Sqrt(CX * CX + CY * CY);
	float Time = GetWorld()->GetTimeSeconds();

	// Draw radial lines emanating from screen edges toward center
	int32 NumLines = 24;
	for (int32 i = 0; i < NumLines; i++)
	{
		float Angle = (float)i / (float)NumLines * 2.f * PI;
		// Slight drift over time for motion feel
		Angle += Time * 0.3f;

		float Cos = FMath::Cos(Angle);
		float Sin = FMath::Sin(Angle);

		// Lines exist in the outer 30% of screen radius
		float InnerR = MaxR * 0.70f;
		float OuterR = MaxR * 1.05f;

		// Animate: lines slide outward over time
		float Slide = FMath::Fmod(Time * 200.f + i * 37.f, OuterR - InnerR);
		float R0 = InnerR + Slide;
		float R1 = FMath::Min(R0 + 40.f + FMath::RandRange(0.f, 20.f), OuterR);

		float X0 = CX + Cos * R0;
		float Y0 = CY + Sin * R0;
		float X1 = CX + Cos * R1;
		float Y1 = CY + Sin * R1;

		// Fade based on distance from edge and overall alpha
		float EdgeFade = (R0 - InnerR) / (OuterR - InnerR);
		float Alpha = SprintLineAlpha * EdgeFade * 0.35f;

		FLinearColor LineCol(0.7f, 0.85f, 1.f, Alpha);
		DrawLine(X0, Y0, X1, Y1, LineCol);
	}
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
	float PanelW = TextW + 40.f;
	float PanelH = TextH + 16.f;
	float X = (Canvas->SizeX - PanelW) * 0.5f;
	float Y = Canvas->SizeY * 0.6f;
	float Time = GetWorld()->GetTimeSeconds();

	// Panel background
	DrawRect(FLinearColor(0.02f, 0.03f, 0.06f, 0.75f), X, Y, PanelW, PanelH);

	// Top accent line (pulsing cyan)
	float Pulse = 0.5f + 0.3f * FMath::Sin(Time * 3.f);
	FLinearColor AccentCol(0.f, 0.6f, 1.f, 0.6f * Pulse);
	DrawRect(AccentCol, X, Y, PanelW, 2.f);

	// Corner brackets
	float BLen = 10.f;
	FLinearColor BCol(0.f, 0.7f, 1.f, 0.4f);
	DrawLine(X, Y, X + BLen, Y, BCol);
	DrawLine(X, Y, X, Y + BLen, BCol);
	DrawLine(X + PanelW, Y, X + PanelW - BLen, Y, BCol);
	DrawLine(X + PanelW, Y, X + PanelW, Y + BLen, BCol);

	// Prompt text
	DrawText(Prompt, FLinearColor(0.8f, 0.9f, 1.f, 0.95f),
		X + (PanelW - TextW) * 0.5f, Y + (PanelH - TextH) * 0.5f, HUDFont, 0.9f);
}
