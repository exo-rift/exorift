// ExoHUDWeapons.cpp — Weapon HUD: crosshair, overheat bar, inventory slots
#include "UI/ExoHUD.h"
#include "UI/ExoHitMarker.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoInventoryComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Engine/Canvas.h"

void AExoHUD::DrawCrosshair()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	float Heat = 0.f;
	bool bOverheated = false;
	if (Char && Char->GetCurrentWeapon())
	{
		Heat = Char->GetCurrentWeapon()->GetCurrentHeat();
		bOverheated = Char->GetCurrentWeapon()->IsOverheated();
		CrosshairSpread = FMath::FInterpTo(CrosshairSpread,
			Heat * 15.f, GetWorld()->GetDeltaSeconds(), 10.f);
	}

	float Time = GetWorld()->GetTimeSeconds();
	FVector2D C = GetScreenCenter();
	float Spread = CrosshairSpread;
	float Gap = 6.f + Spread * 0.5f;
	float ArmLen = 10.f + Spread;
	float BracketTurn = 5.f; // L-bracket perpendicular length
	bool bHit = FExoHitMarker::HasRecentHit();

	// --- Color: white → cyan-orange → pulsing red with heat ---
	FLinearColor BaseCol;
	if (bOverheated)
	{
		float Pulse = FMath::Abs(FMath::Sin(Time * 6.f));
		BaseCol = FMath::Lerp(FLinearColor(1.f, 0.15f, 0.08f, 0.95f),
			FLinearColor(1.f, 0.5f, 0.2f, 0.95f), Pulse);
	}
	else if (Heat > 0.5f)
	{
		float T = (Heat - 0.5f) * 2.f;
		BaseCol = FMath::Lerp(FLinearColor(0.7f, 0.92f, 1.f, 0.88f),
			FLinearColor(1.f, 0.45f, 0.1f, 0.92f), T);
	}
	else
	{
		BaseCol = FLinearColor(0.7f, 0.92f, 1.f, 0.88f);
	}

	// Shadow color for contrast
	FLinearColor Shadow(0.f, 0.f, 0.f, BaseCol.A * 0.4f);
	float ST = 2.5f; // Shadow thickness
	float LT = 1.5f; // Line thickness

	// --- Corner brackets (L-shaped) at each cardinal direction ---
	// Top bracket
	DrawLine(C.X, C.Y - Gap - ArmLen, C.X, C.Y - Gap, Shadow, ST);
	DrawLine(C.X, C.Y - Gap - ArmLen, C.X, C.Y - Gap, BaseCol, LT);
	DrawLine(C.X, C.Y - Gap - ArmLen, C.X + BracketTurn, C.Y - Gap - ArmLen, Shadow, ST);
	DrawLine(C.X, C.Y - Gap - ArmLen, C.X + BracketTurn, C.Y - Gap - ArmLen, BaseCol, LT);

	// Bottom bracket
	DrawLine(C.X, C.Y + Gap, C.X, C.Y + Gap + ArmLen, Shadow, ST);
	DrawLine(C.X, C.Y + Gap, C.X, C.Y + Gap + ArmLen, BaseCol, LT);
	DrawLine(C.X, C.Y + Gap + ArmLen, C.X - BracketTurn, C.Y + Gap + ArmLen, Shadow, ST);
	DrawLine(C.X, C.Y + Gap + ArmLen, C.X - BracketTurn, C.Y + Gap + ArmLen, BaseCol, LT);

	// Left bracket
	DrawLine(C.X - Gap - ArmLen, C.Y, C.X - Gap, C.Y, Shadow, ST);
	DrawLine(C.X - Gap - ArmLen, C.Y, C.X - Gap, C.Y, BaseCol, LT);
	DrawLine(C.X - Gap - ArmLen, C.Y, C.X - Gap - ArmLen, C.Y - BracketTurn, Shadow, ST);
	DrawLine(C.X - Gap - ArmLen, C.Y, C.X - Gap - ArmLen, C.Y - BracketTurn, BaseCol, LT);

	// Right bracket
	DrawLine(C.X + Gap, C.Y, C.X + Gap + ArmLen, C.Y, Shadow, ST);
	DrawLine(C.X + Gap, C.Y, C.X + Gap + ArmLen, C.Y, BaseCol, LT);
	DrawLine(C.X + Gap + ArmLen, C.Y, C.X + Gap + ArmLen, C.Y + BracketTurn, Shadow, ST);
	DrawLine(C.X + Gap + ArmLen, C.Y, C.X + Gap + ArmLen, C.Y + BracketTurn, BaseCol, LT);

	// --- Diagonal tick marks at 45-degree angles ---
	float DiagGap = Gap * 0.9f;
	float DiagLen = 5.f + Spread * 0.3f;
	float D = 0.707f; // cos(45)
	FLinearColor DiagCol = BaseCol;
	DiagCol.A *= 0.5f;
	for (int32 Q = 0; Q < 4; Q++)
	{
		float DX = (Q < 2) ? 1.f : -1.f;
		float DY = (Q % 2 == 0) ? -1.f : 1.f;
		float X1 = C.X + DX * D * DiagGap;
		float Y1 = C.Y + DY * D * DiagGap;
		float X2 = C.X + DX * D * (DiagGap + DiagLen);
		float Y2 = C.Y + DY * D * (DiagGap + DiagLen);
		DrawLine(X1, Y1, X2, Y2, DiagCol, 1.f);
	}

	// --- Center diamond (rotates slightly on hit) ---
	float DotR = bHit ? 3.f : 2.f;
	float DotRot = bHit ? Time * 8.f : 0.f;
	FLinearColor DotCol = bHit ? FLinearColor(1.f, 0.25f, 0.1f, 1.f) : BaseCol;
	DotCol.A = 1.f;
	float CR = FMath::Cos(DotRot);
	float SR = FMath::Sin(DotRot);
	// Diamond: 4 points rotated by DotRot
	FVector2D Pts[4] = {
		{C.X + DotR * CR, C.Y + DotR * SR},
		{C.X - DotR * SR, C.Y + DotR * CR},
		{C.X - DotR * CR, C.Y - DotR * SR},
		{C.X + DotR * SR, C.Y - DotR * CR}
	};
	for (int32 i = 0; i < 4; i++)
	{
		int32 j = (i + 1) % 4;
		DrawLine(Pts[i].X, Pts[i].Y, Pts[j].X, Pts[j].Y, DotCol, 1.5f);
	}

	// --- Rotating outer arcs (4 segments, heat-responsive) ---
	float ArcR = Gap + ArmLen + 6.f;
	float ArcSpan = FMath::DegreesToRadians(55.f); // Each arc spans 55 degrees
	float RotSpeed = 15.f + Heat * 30.f; // Faster rotation at higher heat
	float BaseAngle = FMath::DegreesToRadians(Time * RotSpeed);
	int32 ArcSegments = 8;
	FLinearColor ArcCol = BaseCol;
	ArcCol.A *= (0.15f + Heat * 0.5f); // More visible at higher heat

	for (int32 a = 0; a < 4; a++)
	{
		float ArcStart = BaseAngle + a * (PI * 0.5f);
		for (int32 s = 0; s < ArcSegments; s++)
		{
			float A1 = ArcStart + (ArcSpan * s) / ArcSegments;
			float A2 = ArcStart + (ArcSpan * (s + 1)) / ArcSegments;
			DrawLine(
				C.X + FMath::Cos(A1) * ArcR, C.Y + FMath::Sin(A1) * ArcR,
				C.X + FMath::Cos(A2) * ArcR, C.Y + FMath::Sin(A2) * ArcR,
				ArcCol, 1.f);
		}
	}

	// --- Heat tick marks (8 ticks around crosshair, fill up with heat) ---
	if (Heat > 0.05f)
	{
		float TickR = Gap + ArmLen + 2.f;
		float TickLen = 3.f;
		int32 FilledTicks = FMath::RoundToInt32(Heat * 8.f);
		for (int32 t = 0; t < 8; t++)
		{
			float Angle = (2.f * PI * t) / 8.f - PI * 0.5f; // Start from top
			float X1 = C.X + FMath::Cos(Angle) * TickR;
			float Y1 = C.Y + FMath::Sin(Angle) * TickR;
			float X2 = C.X + FMath::Cos(Angle) * (TickR + TickLen);
			float Y2 = C.Y + FMath::Sin(Angle) * (TickR + TickLen);
			FLinearColor TickCol = (t < FilledTicks)
				? FLinearColor(1.f, 0.4f, 0.1f, 0.8f)
				: FLinearColor(0.4f, 0.4f, 0.45f, 0.2f);
			DrawLine(X1, Y1, X2, Y2, TickCol, 1.5f);
		}
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
