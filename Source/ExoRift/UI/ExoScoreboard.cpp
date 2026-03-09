#include "UI/ExoScoreboard.h"
#include "Core/ExoPlayerState.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

static void ScoreboardDrawRect(UCanvas* Canvas, float X, float Y, float W, float H, const FLinearColor& Color)
{
	Canvas->SetDrawColor(
		FMath::Clamp((int32)(Color.R * 255), 0, 255),
		FMath::Clamp((int32)(Color.G * 255), 0, 255),
		FMath::Clamp((int32)(Color.B * 255), 0, 255),
		FMath::Clamp((int32)(Color.A * 255), 0, 255));
	Canvas->DrawTile(Canvas->DefaultTexture, X, Y, W, H, 0, 0, 1, 1);
}

void FExoScoreboard::Draw(UCanvas* Canvas, UFont* Font, const TArray<AExoPlayerState*>& Players,
	float MatchTime, int32 AliveCount, int32 TotalPlayers)
{
	if (!Canvas || !Font || Players.Num() == 0) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const float PanelW = ScreenW * PanelWidthRatio;
	const float PanelX = (ScreenW - PanelW) * 0.5f;
	float PanelH = TitleHeight + HeaderHeight + Players.Num() * RowHeight + Padding * 2.f;
	float PanelY = (ScreenH - PanelH) * 0.5f;

	// Background
	ScoreboardDrawRect(Canvas, PanelX, PanelY, PanelW, PanelH, FLinearColor(0.02f, 0.02f, 0.06f, 0.85f));

	// Border
	FLinearColor BorderColor(0.0f, 0.6f, 1.0f, 0.6f);
	ScoreboardDrawRect(Canvas, PanelX, PanelY, PanelW, 2.f, BorderColor);
	ScoreboardDrawRect(Canvas, PanelX, PanelY + PanelH - 2.f, PanelW, 2.f, BorderColor);
	ScoreboardDrawRect(Canvas, PanelX, PanelY, 2.f, PanelH, BorderColor);
	ScoreboardDrawRect(Canvas, PanelX + PanelW - 2.f, PanelY, 2.f, PanelH, BorderColor);

	float CurY = PanelY + Padding;

	// Title
	int32 Minutes = (int32)MatchTime / 60;
	int32 Seconds = (int32)MatchTime % 60;
	FString Title = FString::Printf(TEXT("EXORIFT  |  %02d:%02d  |  %d / %d ALIVE"), Minutes, Seconds, AliveCount, TotalPlayers);
	float TitleW, TitleH;
	Canvas->TextSize(Font, Title, TitleW, TitleH);
	Canvas->SetDrawColor(0, 180, 255, 255);
	Canvas->DrawText(Font, Title, PanelX + (PanelW - TitleW) * 0.5f, CurY, 1.0f, 1.0f);
	CurY += TitleHeight;

	// Column header
	DrawHeader(Canvas, Font, PanelX + Padding, CurY, PanelW - Padding * 2.f);
	CurY += HeaderHeight;

	// Separator
	ScoreboardDrawRect(Canvas, PanelX + Padding, CurY - 4.f, PanelW - Padding * 2.f, 1.f,
		FLinearColor(0.3f, 0.3f, 0.4f, 0.8f));

	// Player rows
	for (int32 i = 0; i < Players.Num(); i++)
	{
		if (!Players[i]) continue;
		bool bLocal = Players[i]->GetOwningController() &&
			Players[i]->GetOwningController() == Players[i]->GetWorld()->GetFirstPlayerController();
		DrawPlayerRow(Canvas, Font, Players[i], PanelX + Padding, CurY, PanelW - Padding * 2.f, bLocal, i + 1);
		CurY += RowHeight;
	}
}

void FExoScoreboard::DrawHeader(UCanvas* Canvas, UFont* Font, float X, float Y, float Width)
{
	Canvas->SetDrawColor(150, 150, 170, 200);
	Canvas->DrawText(Font, TEXT("#"), X, Y);
	Canvas->DrawText(Font, TEXT("PLAYER"), X + Width * 0.06f, Y);
	Canvas->DrawText(Font, TEXT("KILLS"), X + Width * 0.45f, Y);
	Canvas->DrawText(Font, TEXT("DMG"), X + Width * 0.58f, Y);
	Canvas->DrawText(Font, TEXT("ASSISTS"), X + Width * 0.70f, Y);
	Canvas->DrawText(Font, TEXT("STATUS"), X + Width * 0.85f, Y);
}

void FExoScoreboard::DrawPlayerRow(UCanvas* Canvas, UFont* Font, AExoPlayerState* PS,
	float X, float Y, float Width, bool bIsLocalPlayer, int32 Rank)
{
	if (!PS) return;

	// Highlight local player
	if (bIsLocalPlayer)
		ScoreboardDrawRect(Canvas, X - 4.f, Y, Width + 8.f, RowHeight - 2.f, FLinearColor(0.0f, 0.4f, 0.8f, 0.2f));

	// Alternating row tint
	if (Rank % 2 == 0)
		ScoreboardDrawRect(Canvas, X - 4.f, Y, Width + 8.f, RowHeight - 2.f, FLinearColor(0.1f, 0.1f, 0.15f, 0.15f));

	// Color based on alive
	if (PS->bIsAlive) Canvas->SetDrawColor(220, 220, 230, 255);
	else Canvas->SetDrawColor(120, 60, 60, 180);

	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), Rank), X, Y);

	if (bIsLocalPlayer) Canvas->SetDrawColor(0, 200, 255, 255);
	Canvas->DrawText(Font, PS->DisplayName, X + Width * 0.06f, Y);

	if (PS->bIsAlive) Canvas->SetDrawColor(220, 220, 230, 255);
	else Canvas->SetDrawColor(120, 60, 60, 180);

	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->Kills), X + Width * 0.45f, Y);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->DamageDealt), X + Width * 0.58f, Y);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->Assists), X + Width * 0.70f, Y);

	FString Status = PS->bIsAlive ? TEXT("ALIVE") : FString::Printf(TEXT("#%d"), PS->Placement);
	if (PS->bIsAlive) Canvas->SetDrawColor(50, 220, 80, 255);
	else Canvas->SetDrawColor(180, 60, 60, 200);
	Canvas->DrawText(Font, Status, X + Width * 0.85f, Y);
}
