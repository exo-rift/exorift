// ExoScoreboard.cpp — TAB scoreboard overlay with sci-fi styling
#include "UI/ExoScoreboard.h"
#include "Core/ExoPlayerState.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

static void SBDrawRect(UCanvas* C, float X, float Y, float W, float H, const FLinearColor& Col)
{
	C->SetDrawColor(
		FMath::Clamp((int32)(Col.R * 255), 0, 255),
		FMath::Clamp((int32)(Col.G * 255), 0, 255),
		FMath::Clamp((int32)(Col.B * 255), 0, 255),
		FMath::Clamp((int32)(Col.A * 255), 0, 255));
	C->DrawTile(C->DefaultTexture, X, Y, W, H, 0, 0, 1, 1);
}

static void SBDrawLine(UCanvas* C, float X1, float Y1, float X2, float Y2, const FLinearColor& Col)
{
	FCanvasLineItem Line(FVector2D(X1, Y1), FVector2D(X2, Y2));
	Line.SetColor(Col);
	Line.LineThickness = 1.f;
	C->DrawItem(Line);
}

void FExoScoreboard::Draw(UCanvas* Canvas, UFont* Font, const TArray<AExoPlayerState*>& Players,
	float MatchTime, int32 AliveCount, int32 TotalPlayers)
{
	if (!Canvas || !Font || Players.Num() == 0) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const float PanelW = ScreenW * PanelWidthRatio;
	const float PanelX = (ScreenW - PanelW) * 0.5f;
	float PanelH = TitleHeight + HeaderHeight + Players.Num() * RowHeight + Padding * 2.f + 4.f;
	float PanelY = (ScreenH - PanelH) * 0.5f;

	// Dim background overlay
	SBDrawRect(Canvas, 0.f, 0.f, ScreenW, ScreenH, FLinearColor(0.f, 0.f, 0.f, 0.3f));

	// Panel background with subtle gradient
	SBDrawRect(Canvas, PanelX, PanelY, PanelW, PanelH, FLinearColor(0.015f, 0.015f, 0.04f, 0.92f));

	// Top accent glow
	SBDrawRect(Canvas, PanelX, PanelY, PanelW, 3.f, FLinearColor(0.f, 0.6f, 1.f, 0.8f));
	SBDrawRect(Canvas, PanelX, PanelY + 3.f, PanelW, 6.f, FLinearColor(0.f, 0.3f, 0.6f, 0.15f));

	// Side borders
	FLinearColor BorderCol(0.f, 0.4f, 0.7f, 0.3f);
	SBDrawLine(Canvas, PanelX, PanelY, PanelX, PanelY + PanelH, BorderCol);
	SBDrawLine(Canvas, PanelX + PanelW, PanelY, PanelX + PanelW, PanelY + PanelH, BorderCol);
	// Bottom border
	SBDrawRect(Canvas, PanelX, PanelY + PanelH - 2.f, PanelW, 2.f, FLinearColor(0.f, 0.4f, 0.7f, 0.4f));

	float CurY = PanelY + Padding;

	// Title bar
	int32 Minutes = (int32)MatchTime / 60;
	int32 Seconds = (int32)MatchTime % 60;

	// Match time (left)
	FString TimeStr = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	Canvas->SetDrawColor(0, 180, 255, 200);
	Canvas->DrawText(Font, TimeStr, PanelX + Padding, CurY, 1.0f, 1.0f);

	// Title (center)
	FString Title = TEXT("EXORIFT SCOREBOARD");
	float TitleW, TitleH;
	Canvas->TextSize(Font, Title, TitleW, TitleH);
	Canvas->SetDrawColor(0, 200, 255, 255);
	Canvas->DrawText(Font, Title, PanelX + (PanelW - TitleW) * 0.5f, CurY, 1.0f, 1.0f);

	// Alive count (right)
	FString AliveStr = FString::Printf(TEXT("%d / %d ALIVE"), AliveCount, TotalPlayers);
	float AW, AH;
	Canvas->TextSize(Font, AliveStr, AW, AH);
	Canvas->SetDrawColor(100, 200, 100, 200);
	Canvas->DrawText(Font, AliveStr, PanelX + PanelW - Padding - AW, CurY, 1.0f, 1.0f);

	CurY += TitleHeight;

	// Header separator
	SBDrawRect(Canvas, PanelX + Padding, CurY - 6.f, PanelW - Padding * 2.f, 1.f,
		FLinearColor(0.f, 0.5f, 0.8f, 0.4f));

	// Column headers
	DrawHeader(Canvas, Font, PanelX + Padding, CurY, PanelW - Padding * 2.f);
	CurY += HeaderHeight;

	// Header-to-rows separator
	SBDrawRect(Canvas, PanelX + Padding, CurY - 4.f, PanelW - Padding * 2.f, 1.f,
		FLinearColor(0.2f, 0.25f, 0.35f, 0.6f));

	// Player rows
	for (int32 i = 0; i < Players.Num(); i++)
	{
		if (!Players[i]) continue;
		bool bLocal = Players[i]->GetOwningController() &&
			Players[i]->GetOwningController() == Players[i]->GetWorld()->GetFirstPlayerController();
		DrawPlayerRow(Canvas, Font, Players[i], PanelX + Padding, CurY,
			PanelW - Padding * 2.f, bLocal, i + 1);
		CurY += RowHeight;
	}
}

void FExoScoreboard::DrawHeader(UCanvas* Canvas, UFont* Font, float X, float Y, float Width)
{
	Canvas->SetDrawColor(100, 110, 140, 180);
	Canvas->DrawText(Font, TEXT("#"), X, Y, 0.85f, 0.85f);
	Canvas->DrawText(Font, TEXT("PLAYER"), X + Width * 0.06f, Y, 0.85f, 0.85f);
	Canvas->DrawText(Font, TEXT("KILLS"), X + Width * 0.42f, Y, 0.85f, 0.85f);
	Canvas->DrawText(Font, TEXT("DAMAGE"), X + Width * 0.54f, Y, 0.85f, 0.85f);
	Canvas->DrawText(Font, TEXT("ASSISTS"), X + Width * 0.68f, Y, 0.85f, 0.85f);
	Canvas->DrawText(Font, TEXT("STATUS"), X + Width * 0.82f, Y, 0.85f, 0.85f);
}

void FExoScoreboard::DrawPlayerRow(UCanvas* Canvas, UFont* Font, AExoPlayerState* PS,
	float X, float Y, float Width, bool bIsLocalPlayer, int32 Rank)
{
	if (!PS) return;

	// Local player highlight
	if (bIsLocalPlayer)
	{
		SBDrawRect(Canvas, X - 4.f, Y, Width + 8.f, RowHeight - 2.f,
			FLinearColor(0.f, 0.3f, 0.6f, 0.2f));
		// Left accent stripe
		SBDrawRect(Canvas, X - 4.f, Y, 3.f, RowHeight - 2.f,
			FLinearColor(0.f, 0.7f, 1.f, 0.7f));
	}

	// Alternating row tint
	if (Rank % 2 == 0)
	{
		SBDrawRect(Canvas, X - 4.f, Y, Width + 8.f, RowHeight - 2.f,
			FLinearColor(0.06f, 0.06f, 0.1f, 0.15f));
	}

	// Rank number — gold/silver/bronze for top 3
	if (Rank == 1)      Canvas->SetDrawColor(255, 215, 50, 255);
	else if (Rank == 2) Canvas->SetDrawColor(200, 200, 210, 230);
	else if (Rank == 3) Canvas->SetDrawColor(200, 140, 60, 220);
	else if (PS->bIsAlive) Canvas->SetDrawColor(180, 180, 195, 200);
	else Canvas->SetDrawColor(100, 50, 50, 150);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), Rank), X, Y);

	// Player name
	if (bIsLocalPlayer) Canvas->SetDrawColor(0, 210, 255, 255);
	else if (PS->bIsAlive) Canvas->SetDrawColor(210, 215, 225, 240);
	else Canvas->SetDrawColor(110, 55, 55, 160);
	Canvas->DrawText(Font, PS->DisplayName, X + Width * 0.06f, Y);

	// Stats — dimmer if dead
	if (PS->bIsAlive) Canvas->SetDrawColor(210, 215, 225, 220);
	else Canvas->SetDrawColor(110, 55, 55, 160);

	// Kills — highlight if high
	if (PS->Kills >= 5 && PS->bIsAlive) Canvas->SetDrawColor(255, 200, 50, 255);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->Kills), X + Width * 0.42f, Y);

	if (PS->bIsAlive) Canvas->SetDrawColor(210, 215, 225, 220);
	else Canvas->SetDrawColor(110, 55, 55, 160);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->DamageDealt), X + Width * 0.54f, Y);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->Assists), X + Width * 0.68f, Y);

	// Status badge
	if (PS->bIsAlive)
	{
		float StatusX = X + Width * 0.82f;
		// Green "ALIVE" badge background
		SBDrawRect(Canvas, StatusX - 2.f, Y + 2.f, 52.f, RowHeight - 6.f,
			FLinearColor(0.f, 0.4f, 0.1f, 0.3f));
		Canvas->SetDrawColor(50, 220, 80, 255);
		Canvas->DrawText(Font, TEXT("ALIVE"), StatusX, Y);
	}
	else
	{
		float StatusX = X + Width * 0.82f;
		FString PlaceStr = FString::Printf(TEXT("#%d"), PS->Placement);
		SBDrawRect(Canvas, StatusX - 2.f, Y + 2.f, 42.f, RowHeight - 6.f,
			FLinearColor(0.4f, 0.05f, 0.05f, 0.25f));
		Canvas->SetDrawColor(180, 60, 60, 200);
		Canvas->DrawText(Font, PlaceStr, StatusX, Y);
	}
}
