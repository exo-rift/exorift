// ExoHUDWeaponsHeat.cpp — Weapon HUD: overheat bar, weapon name/fire mode, grenade indicator
#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoGrenadeComponent.h"
#include "Engine/Canvas.h"

void AExoHUD::DrawOverheatBar()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetCurrentWeapon()) return;

	AExoWeaponBase* Weapon = Char->GetCurrentWeapon();
	float HeatPct = Weapon->GetCurrentHeat();
	bool bOverheated = Weapon->IsOverheated();
	float Time = GetWorld()->GetTimeSeconds();

	FLinearColor HeatColor;
	if (bOverheated)
	{
		float Pulse = FMath::Abs(FMath::Sin(Time * 4.f));
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

	// --- Numerical heat % on the right, "HEAT" label on the left ---
	FString HeatText = FString::Printf(TEXT("%d%%"), FMath::RoundToInt32(HeatPct * 100.f));
	float HeatTW, HeatTH;
	GetTextSize(HeatText, HeatTW, HeatTH, HUDFont, 0.75f);
	FLinearColor HeatTextCol = HeatColor; HeatTextCol.A = 1.f;
	float HeatTextY = Y + (BarH - HeatTH) * 0.5f;
	DrawText(HeatText, FLinearColor(0.f, 0.f, 0.f, 0.6f), X + BarW + 9.f, HeatTextY + 1.f, HUDFont, 0.75f);
	DrawText(HeatText, HeatTextCol, X + BarW + 8.f, HeatTextY, HUDFont, 0.75f);

	float LblW, LblH;
	GetTextSize(TEXT("HEAT"), LblW, LblH, HUDFont, 0.6f);
	DrawText(TEXT("HEAT"), FLinearColor(0.5f, 0.55f, 0.65f, 0.7f),
		X - LblW - 8.f, Y + (BarH - LblH) * 0.5f, HUDFont, 0.6f);

	// --- Weapon name with rarity color accent ---
	FLinearColor RarityCol = AExoWeaponBase::GetRarityColor(Weapon->Rarity);
	FString WepName = Weapon->GetWeaponName();
	float NameW, NameH;
	GetTextSize(WepName, NameW, NameH, HUDFont, 0.85f);
	float NameX = X + (BarW - NameW) * 0.5f;
	float NameY = Y - NameH - 18.f;
	float PanelPad = 8.f;
	DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.5f),
		NameX - PanelPad, NameY - 2.f, NameW + PanelPad * 2.f, NameH + 4.f);
	DrawRect(RarityCol * 0.8f,
		NameX - PanelPad, NameY + NameH + 1.f, NameW + PanelPad * 2.f, 2.f);
	DrawText(WepName, RarityCol, NameX, NameY, HUDFont, 0.85f);
	// Fire mode indicator / overheated warning
	FString ModeText; FLinearColor ModeCol = FLinearColor::White;
	if (bOverheated)
	{
		float P = FMath::Abs(FMath::Sin(Time * 6.f));
		ModeText = TEXT("OVERHEATED");
		ModeCol = FLinearColor(1.f, 0.2f, 0.1f, 0.7f + 0.3f * P);
	}
	else if (AExoWeaponRifle* Rifle = Cast<AExoWeaponRifle>(Weapon))
	{
		ModeText = (Rifle->GetFireMode() == ERifleFireMode::Burst) ? TEXT("BURST") : TEXT("AUTO");
		ModeCol = FLinearColor(0.6f, 0.8f, 1.f, 0.8f);
	}
	else if (Weapon->GetWeaponType() == EWeaponType::SMG)
	{
		ModeText = TEXT("AUTO");
		ModeCol = FLinearColor(0.6f, 0.8f, 1.f, 0.8f);
	}
	else if (Weapon->GetWeaponType() == EWeaponType::Sniper
		|| Weapon->GetWeaponType() == EWeaponType::Shotgun
		|| Weapon->GetWeaponType() == EWeaponType::Pistol)
	{
		ModeText = TEXT("SEMI");
		ModeCol = FLinearColor(0.6f, 0.75f, 0.9f, 0.7f);
	}
	if (!ModeText.IsEmpty())
	{
		float ModeTW, ModeTH;
		GetTextSize(ModeText, ModeTW, ModeTH, HUDFont, 0.65f);
		float ModeX = X + BarW - ModeTW;
		float ModeY = Y - ModeTH - 4.f;
		DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.45f),
			ModeX - 4.f, ModeY - 1.f, ModeTW + 8.f, ModeTH + 2.f);
		DrawText(ModeText, ModeCol, ModeX, ModeY, HUDFont, 0.65f);
	}
}

void AExoHUD::DrawGrenadeIndicator()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	UExoGrenadeComponent* GrenComp = Char->GetGrenadeComponent();
	if (!GrenComp) return;

	int32 Count = GrenComp->GetCurrentGrenades();
	if (Count <= 0) return;

	EGrenadeType Type = GrenComp->GetGrenadeType();

	// --- Type color and label ---
	FLinearColor TypeColor;
	FString TypeLabel;
	switch (Type)
	{
	case EGrenadeType::Frag:
		TypeColor = FLinearColor(1.f, 0.55f, 0.1f, 0.95f);   // Orange
		TypeLabel = TEXT("FRAG");
		break;
	case EGrenadeType::EMP:
		TypeColor = FLinearColor(0.2f, 0.55f, 1.f, 0.95f);    // Blue
		TypeLabel = TEXT("EMP");
		break;
	case EGrenadeType::Smoke:
		TypeColor = FLinearColor(0.65f, 0.65f, 0.68f, 0.95f);  // Grey
		TypeLabel = TEXT("SMOKE");
		break;
	default:
		TypeColor = FLinearColor(1.f, 0.55f, 0.1f, 0.95f);
		TypeLabel = TEXT("GREN");
		break;
	}

	// --- Position: below the weapon inventory slots (lower-right) ---
	// Mirror the weapon indicator layout to align horizontally
	const float SlotW = 120.f;
	const float PanelW = SlotW;
	const float PanelH = 32.f;
	const float BaseX = Canvas->SizeX - PanelW - 20.f;
	const float BaseY = Canvas->SizeY - 88.f; // Below weapon slots, above bottom

	// Background panel
	DrawRect(FLinearColor(0.03f, 0.03f, 0.05f, 0.65f), BaseX, BaseY, PanelW, PanelH);

	// Left color stripe (type accent)
	DrawRect(TypeColor, BaseX, BaseY, 3.f, PanelH);

	// --- Diamond icon (small colored diamond shape) ---
	float DiamX = BaseX + 16.f;
	float DiamY = BaseY + PanelH * 0.5f;
	float DiamR = 6.f;
	// Draw diamond as 4 lines
	DrawLine(DiamX, DiamY - DiamR, DiamX + DiamR, DiamY, TypeColor, 1.5f);
	DrawLine(DiamX + DiamR, DiamY, DiamX, DiamY + DiamR, TypeColor, 1.5f);
	DrawLine(DiamX, DiamY + DiamR, DiamX - DiamR, DiamY, TypeColor, 1.5f);
	DrawLine(DiamX - DiamR, DiamY, DiamX, DiamY - DiamR, TypeColor, 1.5f);
	// Fill center dot
	DrawRect(TypeColor, DiamX - 1.5f, DiamY - 1.5f, 3.f, 3.f);

	// --- Type label ---
	float LblW, LblH;
	GetTextSize(TypeLabel, LblW, LblH, HUDFont, 0.6f);
	float LblX = BaseX + 28.f;
	float LblY = BaseY + 2.f;
	DrawText(TypeLabel, TypeColor, LblX, LblY, HUDFont, 0.6f);

	// --- Count text (e.g. "x2") ---
	FString CountText = FString::Printf(TEXT("x%d"), Count);
	float CntW, CntH;
	GetTextSize(CountText, CntW, CntH, HUDFont, 0.85f);
	float CntX = BaseX + PanelW - CntW - 8.f;
	float CntY = BaseY + (PanelH - CntH) * 0.5f;
	// Shadow for readability
	DrawText(CountText, FLinearColor(0.f, 0.f, 0.f, 0.5f), CntX + 1.f, CntY + 1.f, HUDFont, 0.85f);
	DrawText(CountText, ColorWhite, CntX, CntY, HUDFont, 0.85f);

	// --- Keybind hint ---
	float HintW, HintH;
	GetTextSize(TEXT("[G]"), HintW, HintH, HUDFont, 0.5f);
	DrawText(TEXT("[G]"), FLinearColor(0.5f, 0.55f, 0.6f, 0.6f),
		LblX, BaseY + PanelH - HintH - 2.f, HUDFont, 0.5f);

	// --- Border highlight ---
	FLinearColor BorderCol(TypeColor.R, TypeColor.G, TypeColor.B, 0.3f);
	DrawLine(BaseX, BaseY, BaseX + PanelW, BaseY, BorderCol);
	DrawLine(BaseX, BaseY + PanelH, BaseX + PanelW, BaseY + PanelH, BorderCol);
	DrawLine(BaseX, BaseY, BaseX, BaseY + PanelH, BorderCol);
	DrawLine(BaseX + PanelW, BaseY, BaseX + PanelW, BaseY + PanelH, BorderCol);
}
