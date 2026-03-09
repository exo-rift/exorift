// ExoHUDWeapons.cpp — Weapon HUD: crosshair, overheat bar, inventory slots
#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoInventoryComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Engine/Canvas.h"

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
