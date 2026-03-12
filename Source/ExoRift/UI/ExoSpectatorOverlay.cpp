// ExoSpectatorOverlay.cpp — Spectator HUD overlay: target info, bars, controls, vignette
#include "UI/ExoSpectatorOverlay.h"
#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Core/ExoPlayerState.h"
#include "Core/ExoGameState.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

namespace
{
	// Colors — sci-fi cyan/blue aesthetic
	const FLinearColor CyanAccent(0.3f, 0.8f, 1.f, 1.f);
	const FLinearColor WhiteSoft(0.9f, 0.92f, 0.95f, 0.85f);
	const FLinearColor GrayHint(0.6f, 0.62f, 0.65f, 0.7f);
	const FLinearColor HealthGreen(0.1f, 0.9f, 0.2f, 0.85f);
	const FLinearColor ShieldBlue(0.2f, 0.5f, 1.f, 0.85f);
	const FLinearColor RedBadge(0.85f, 0.15f, 0.15f, 0.85f);
	const FLinearColor RedBadgeBg(0.7f, 0.05f, 0.05f, 0.6f);
	const FLinearColor BarBg(0.08f, 0.08f, 0.12f, 0.6f);
	const FLinearColor VignetteBlack(0.f, 0.f, 0.f, 0.15f);

	void DrawSpectatingLabel(UCanvas* Canvas, UFont* Font, float CX, float TopY)
	{
		const FString Label = TEXT("SPECTATING");
		float TW, TH;
		Canvas->TextSize(Font, Label, TW, TH, 1.f, 1.f);

		// Scale for medium-sized text
		float Scale = FMath::Max(Canvas->SizeY / 1080.f, 0.6f);
		TW *= Scale;
		TH *= Scale;

		FCanvasTextItem TextItem(
			FVector2D(CX - TW * 0.5f, TopY),
			FText::FromString(Label),
			Font,
			WhiteSoft
		);
		TextItem.Scale = FVector2D(Scale, Scale);
		TextItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.5f));
		Canvas->DrawItem(TextItem);
	}

	void DrawTargetName(UCanvas* Canvas, UFont* Font, float CX, float Y,
		const FString& Name)
	{
		if (Name.IsEmpty()) return;

		float Scale = FMath::Max(Canvas->SizeY / 1080.f, 0.6f) * 1.4f;
		float TW, TH;
		Canvas->TextSize(Font, Name, TW, TH, 1.f, 1.f);
		TW *= Scale;
		TH *= Scale;

		FCanvasTextItem TextItem(
			FVector2D(CX - TW * 0.5f, Y),
			FText::FromString(Name),
			Font,
			CyanAccent
		);
		TextItem.Scale = FVector2D(Scale, Scale);
		TextItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.6f));
		Canvas->DrawItem(TextItem);
	}

	float DrawTargetBars(UCanvas* Canvas, float CX, float Y, AExoCharacter* Char)
	{
		if (!Char) return Y;

		const float BarW = 200.f;
		const float BarH = 4.f;
		const float BarX = CX - BarW * 0.5f;
		float CurY = Y;

		// Health bar
		float HealthPct = (Char->GetMaxHealth() > 0.f)
			? FMath::Clamp(Char->GetHealth() / Char->GetMaxHealth(), 0.f, 1.f)
			: 0.f;

		Canvas->SetDrawColor(
			FMath::RoundToInt32(BarBg.R * 255.f),
			FMath::RoundToInt32(BarBg.G * 255.f),
			FMath::RoundToInt32(BarBg.B * 255.f),
			FMath::RoundToInt32(BarBg.A * 255.f));
		Canvas->DrawTile(Canvas->DefaultTexture, BarX, CurY, BarW, BarH, 0, 0, 1, 1);

		if (HealthPct > 0.f)
		{
			Canvas->SetDrawColor(
				FMath::RoundToInt32(HealthGreen.R * 255.f),
				FMath::RoundToInt32(HealthGreen.G * 255.f),
				FMath::RoundToInt32(HealthGreen.B * 255.f),
				FMath::RoundToInt32(HealthGreen.A * 255.f));
			Canvas->DrawTile(Canvas->DefaultTexture, BarX, CurY,
				BarW * HealthPct, BarH, 0, 0, 1, 1);
		}
		CurY += BarH + 2.f;

		// Shield bar (only if target has a shield component with shield)
		UExoShieldComponent* Shield = Char->GetShieldComponent();
		if (Shield && Shield->HasShield())
		{
			float ShieldPct = Shield->GetShieldPercent();

			Canvas->SetDrawColor(
				FMath::RoundToInt32(BarBg.R * 255.f),
				FMath::RoundToInt32(BarBg.G * 255.f),
				FMath::RoundToInt32(BarBg.B * 255.f),
				FMath::RoundToInt32(BarBg.A * 255.f));
			Canvas->DrawTile(Canvas->DefaultTexture, BarX, CurY, BarW, BarH, 0, 0, 1, 1);

			if (ShieldPct > 0.f)
			{
				Canvas->SetDrawColor(
					FMath::RoundToInt32(ShieldBlue.R * 255.f),
					FMath::RoundToInt32(ShieldBlue.G * 255.f),
					FMath::RoundToInt32(ShieldBlue.B * 255.f),
					FMath::RoundToInt32(ShieldBlue.A * 255.f));
				Canvas->DrawTile(Canvas->DefaultTexture, BarX, CurY,
					BarW * ShieldPct, BarH, 0, 0, 1, 1);
			}
			CurY += BarH + 2.f;
		}

		return CurY;
	}

	void DrawControlsHint(UCanvas* Canvas, UFont* Font, float CX, float BottomY)
	{
		const FString Hint = TEXT("LMB Next  |  Q Prev  |  SPACE Free Look");
		float Scale = FMath::Max(Canvas->SizeY / 1080.f, 0.5f) * 0.8f;
		float TW, TH;
		Canvas->TextSize(Font, Hint, TW, TH, 1.f, 1.f);
		TW *= Scale;
		TH *= Scale;

		FCanvasTextItem TextItem(
			FVector2D(CX - TW * 0.5f, BottomY - TH),
			FText::FromString(Hint),
			Font,
			GrayHint
		);
		TextItem.Scale = FVector2D(Scale, Scale);
		TextItem.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.35f));
		Canvas->DrawItem(TextItem);
	}

	void DrawEliminatedBadge(UCanvas* Canvas, UFont* Font,
		int32 Placement, int32 TotalPlayers)
	{
		if (Placement <= 0) return;

		const FString BadgeText = FString::Printf(TEXT("ELIMINATED  #%d of %d"),
			Placement, TotalPlayers);
		float Scale = FMath::Max(Canvas->SizeY / 1080.f, 0.5f) * 0.85f;
		float TW, TH;
		Canvas->TextSize(Font, BadgeText, TW, TH, 1.f, 1.f);
		TW *= Scale;
		TH *= Scale;

		float PadX = 12.f;
		float PadY = 6.f;
		float BoxW = TW + PadX * 2.f;
		float BoxH = TH + PadY * 2.f;
		float BoxX = Canvas->SizeX - BoxW - 20.f;
		float BoxY = 20.f;

		// Background box
		Canvas->SetDrawColor(
			FMath::RoundToInt32(RedBadgeBg.R * 255.f),
			FMath::RoundToInt32(RedBadgeBg.G * 255.f),
			FMath::RoundToInt32(RedBadgeBg.B * 255.f),
			FMath::RoundToInt32(RedBadgeBg.A * 255.f));
		Canvas->DrawTile(Canvas->DefaultTexture,
			BoxX, BoxY, BoxW, BoxH, 0, 0, 1, 1);

		// Text
		FCanvasTextItem TextItem(
			FVector2D(BoxX + PadX, BoxY + PadY),
			FText::FromString(BadgeText),
			Font,
			RedBadge
		);
		TextItem.Scale = FVector2D(Scale, Scale);
		Canvas->DrawItem(TextItem);
	}

	void DrawVignette(UCanvas* Canvas)
	{
		const float SW = Canvas->SizeX;
		const float SH = Canvas->SizeY;
		const float EdgeW = SW * 0.08f;
		const float EdgeH = SH * 0.08f;

		Canvas->SetDrawColor(
			FMath::RoundToInt32(VignetteBlack.R * 255.f),
			FMath::RoundToInt32(VignetteBlack.G * 255.f),
			FMath::RoundToInt32(VignetteBlack.B * 255.f),
			FMath::RoundToInt32(VignetteBlack.A * 255.f));

		// Left edge
		Canvas->DrawTile(Canvas->DefaultTexture, 0.f, 0.f, EdgeW, SH, 0, 0, 1, 1);
		// Right edge
		Canvas->DrawTile(Canvas->DefaultTexture, SW - EdgeW, 0.f, EdgeW, SH, 0, 0, 1, 1);
		// Top edge
		Canvas->DrawTile(Canvas->DefaultTexture, EdgeW, 0.f, SW - EdgeW * 2.f, EdgeH, 0, 0, 1, 1);
		// Bottom edge
		Canvas->DrawTile(Canvas->DefaultTexture, EdgeW, SH - EdgeH, SW - EdgeW * 2.f, EdgeH, 0, 0, 1, 1);
	}
}

void FExoSpectatorOverlay::Draw(UCanvas* Canvas, UFont* Font,
	AExoSpectatorPawn* Spectator)
{
	if (!Canvas || !Font || !Spectator) return;
	if (Spectator->IsInDeathCam()) return;

	const float CX = Canvas->SizeX * 0.5f;
	const float SH = Canvas->SizeY;

	// Subtle vignette to distinguish spectator mode
	DrawVignette(Canvas);

	// "SPECTATING" label at ~15% from top (below compass area)
	float LabelY = SH * 0.15f;
	DrawSpectatingLabel(Canvas, Font, CX, LabelY);

	// Target player name below the label
	FString TargetName = Spectator->GetSpectateTargetName();
	float NameY = LabelY + SH * 0.035f;
	DrawTargetName(Canvas, Font, CX, NameY, TargetName);

	// Health + shield bars for the target character
	APawn* Target = Spectator->GetCurrentSpectateTarget();
	AExoCharacter* TargetChar = Cast<AExoCharacter>(Target);
	float BarsY = NameY + SH * 0.045f;
	DrawTargetBars(Canvas, CX, BarsY, TargetChar);

	// Controls hint at bottom center
	float HintY = SH * 0.92f;
	DrawControlsHint(Canvas, Font, CX, HintY);

	// "ELIMINATED" badge — show local player's placement
	APlayerController* PC = Spectator->GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		AExoPlayerState* PS = PC->GetPlayerState<AExoPlayerState>();
		AExoGameState* GS = Spectator->GetWorld()->GetGameState<AExoGameState>();
		int32 Placement = PS ? PS->Placement : 0;
		int32 Total = GS ? GS->TotalPlayers : 0;
		DrawEliminatedBadge(Canvas, Font, Placement, Total);
	}
}
