// ExoHUDMatchStats.cpp — Match stat panels: elimination counter, kill streak, FPS
#include "UI/ExoHUD.h"
#include "Core/ExoGameSettings.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoKillStreakComponent.h"
#include "Engine/Canvas.h"

// Helper: draw octagonal circle (8-segment approximation)
static void DrawOctCircle(AHUD* HUD, float CX, float CY, float R, const FLinearColor& Col, float Thick = 1.5f)
{
	constexpr int32 Segs = 8;
	for (int32 i = 0; i < Segs; ++i)
	{
		float A1 = 2.f * PI * i / Segs;
		float A2 = 2.f * PI * (i + 1) / Segs;
		HUD->DrawLine(CX + R * FMath::Cos(A1), CY + R * FMath::Sin(A1),
			CX + R * FMath::Cos(A2), CY + R * FMath::Sin(A2), Col, Thick);
	}
}

void AExoHUD::DrawAliveCount()
{
	// Unified elimination counter — kills, alive, damage in upper-right panel
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	AExoPlayerState* PS = GetOwningPawn()
		? GetOwningPawn()->GetPlayerState<AExoPlayerState>() : nullptr;

	int32 KillsVal = PS ? PS->Kills : 0;
	int32 DmgVal = PS ? PS->DamageDealt : 0;
	int32 AliveVal = GS->AliveCount;

	const float PanelW = 160.f, RowH = 38.f, Gap = 4.f;
	const float PanelH = RowH * 2 + Gap + 30.f;
	const float PX = Canvas->SizeX - PanelW - 20.f;
	const float PY = 30.f;

	// Panel background + border
	DrawRect(FLinearColor(0.03f, 0.03f, 0.06f, 0.75f), PX, PY, PanelW, PanelH);
	FLinearColor BorderCol(0.15f, 0.2f, 0.3f, 0.4f);
	DrawLine(PX, PY, PX + PanelW, PY, BorderCol);
	DrawLine(PX, PY + PanelH, PX + PanelW, PY + PanelH, BorderCol);
	DrawLine(PX, PY, PX, PY + PanelH, BorderCol);
	DrawLine(PX + PanelW, PY, PX + PanelW, PY + PanelH, BorderCol);

	// Corner bracket accents
	FLinearColor CornerCol(0.f, 0.6f, 1.f, 0.4f);
	float BL = 10.f;
	DrawLine(PX, PY, PX + BL, PY, CornerCol, 1.5f);
	DrawLine(PX, PY, PX, PY + BL, CornerCol, 1.5f);
	DrawLine(PX + PanelW, PY + PanelH, PX + PanelW - BL, PY + PanelH, CornerCol, 1.5f);
	DrawLine(PX + PanelW, PY + PanelH, PX + PanelW, PY + PanelH - BL, CornerCol, 1.5f);

	float RowY = PY + 5.f;
	float IconX = PX + 12.f;
	float NumX = IconX + 22.f;
	FLinearColor LabelCol(0.55f, 0.6f, 0.7f, 0.8f);

	// === ROW 1: KILLS ===
	FLinearColor SkullCol(1.f, 0.35f, 0.2f, 0.9f);
	DrawRect(SkullCol, PX, RowY, 2.f, RowH); // left accent

	// Skull icon: octagonal head + eye X-marks + crossbones
	float SCX = IconX, SCY = RowY + RowH * 0.5f - 2.f;
	DrawOctCircle(this, SCX, SCY, 7.f, SkullCol);
	float EO = 3.f, ES = 1.5f; // eye offset, eye size
	DrawLine(SCX-EO-ES, SCY-1.f-ES, SCX-EO+ES, SCY-1.f+ES, SkullCol, 1.f);
	DrawLine(SCX-EO+ES, SCY-1.f-ES, SCX-EO-ES, SCY-1.f+ES, SkullCol, 1.f);
	DrawLine(SCX+EO-ES, SCY-1.f-ES, SCX+EO+ES, SCY-1.f+ES, SkullCol, 1.f);
	DrawLine(SCX+EO+ES, SCY-1.f-ES, SCX+EO-ES, SCY-1.f+ES, SkullCol, 1.f);
	float BoneY = SCY + 10.f;
	DrawLine(SCX-5.f, BoneY-2.f, SCX+5.f, BoneY+2.f, SkullCol, 1.f);
	DrawLine(SCX+5.f, BoneY-2.f, SCX-5.f, BoneY+2.f, SkullCol, 1.f);

	// Kill number + label
	FString KillNumText = FString::Printf(TEXT("%d"), KillsVal);
	float KW, KH;
	GetTextSize(KillNumText, KW, KH, HUDFont, 1.6f);
	DrawText(KillNumText, ColorWhite, NumX, RowY + (RowH - KH) * 0.5f, HUDFont, 1.6f);
	float LW, LH;
	GetTextSize(TEXT("KILLS"), LW, LH, HUDFont, 0.7f);
	DrawText(TEXT("KILLS"), LabelCol, NumX + KW + 6.f, RowY + (RowH - LH) * 0.5f + 2.f, HUDFont, 0.7f);

	// Separator
	DrawLine(PX + 10.f, RowY + RowH + Gap * 0.5f, PX + PanelW - 10.f, RowY + RowH + Gap * 0.5f,
		FLinearColor(0.2f, 0.25f, 0.35f, 0.4f));

	// === ROW 2: ALIVE ===
	float Row2Y = RowY + RowH + Gap;
	FLinearColor PersonCol(0.f, 0.7f, 1.f, 0.9f);
	DrawRect(PersonCol, PX, Row2Y, 2.f, RowH); // left accent

	// Person icon: head circle + shoulders + torso + hips
	float PCY = Row2Y + RowH * 0.5f - 4.f;
	DrawOctCircle(this, IconX, PCY, 4.f, PersonCol);
	float ShY = PCY + 6.f;
	DrawLine(IconX-6.f, ShY, IconX+6.f, ShY, PersonCol, 1.5f);
	DrawLine(IconX, ShY, IconX, ShY+8.f, PersonCol, 1.5f);
	DrawLine(IconX-5.f, ShY+8.f, IconX+5.f, ShY+8.f, PersonCol, 1.f);

	// Alive number + label
	FString AliveNumText = FString::Printf(TEXT("%d"), AliveVal);
	float AW, AH;
	GetTextSize(AliveNumText, AW, AH, HUDFont, 1.6f);
	DrawText(AliveNumText, ColorWhite, NumX, Row2Y + (RowH - AH) * 0.5f, HUDFont, 1.6f);
	float ALW, ALH;
	GetTextSize(TEXT("ALIVE"), ALW, ALH, HUDFont, 0.7f);
	DrawText(TEXT("ALIVE"), LabelCol, NumX + AW + 6.f, Row2Y + (RowH - ALH) * 0.5f + 2.f, HUDFont, 0.7f);

	// === Sub-row: Damage dealt ===
	FString DmgText = FString::Printf(TEXT("DMG  %d"), DmgVal);
	DrawText(DmgText, FLinearColor(0.7f, 0.5f, 0.2f, 0.65f), PX + 14.f, Row2Y + RowH + 4.f, HUDFont, 0.6f);
}

void AExoHUD::DrawKillCount()
{
	// Drawn as part of the unified panel in DrawAliveCount().
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
