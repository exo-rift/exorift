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

static void SBDrawLine(UCanvas* C, float X1, float Y1, float X2, float Y2,
	const FLinearColor& Col, float Thick = 1.f)
{
	FCanvasLineItem Line(FVector2D(X1, Y1), FVector2D(X2, Y2));
	Line.SetColor(Col);
	Line.LineThickness = Thick;
	C->DrawItem(Line);
}

static void SBCornerBrackets(UCanvas* C, float X, float Y, float W, float H,
	const FLinearColor& Col, float Len = 16.f)
{
	SBDrawLine(C, X, Y, X + Len, Y, Col);
	SBDrawLine(C, X, Y, X, Y + Len, Col);
	SBDrawLine(C, X + W, Y, X + W - Len, Y, Col);
	SBDrawLine(C, X + W, Y, X + W, Y + Len, Col);
	SBDrawLine(C, X, Y + H, X + Len, Y + H, Col);
	SBDrawLine(C, X, Y + H, X, Y + H - Len, Col);
	SBDrawLine(C, X + W, Y + H, X + W - Len, Y + H, Col);
	SBDrawLine(C, X + W, Y + H, X + W, Y + H - Len, Col);
}

void FExoScoreboard::Draw(UCanvas* Canvas, UFont* Font, const TArray<AExoPlayerState*>& Players,
	float MatchTime, int32 AliveCount, int32 TotalPlayers)
{
	if (!Canvas || !Font || Players.Num() == 0) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const float PanelW = ScreenW * PanelWidthRatio;
	const float PanelX = (ScreenW - PanelW) * 0.5f;
	float PanelH = TitleHeight + HeaderHeight + Players.Num() * RowHeight + Padding * 2.f + 8.f;
	float PanelY = (ScreenH - PanelH) * 0.5f;

	FLinearColor Cyan(0.f, 0.7f, 1.f, 0.8f);
	FLinearColor CyanDim(0.f, 0.4f, 0.7f, 0.3f);

	// Dim background overlay
	SBDrawRect(Canvas, 0.f, 0.f, ScreenW, ScreenH, FLinearColor(0.f, 0.f, 0.f, 0.4f));

	// Panel background
	SBDrawRect(Canvas, PanelX, PanelY, PanelW, PanelH,
		FLinearColor(0.015f, 0.02f, 0.045f, 0.94f));

	// Corner brackets
	SBCornerBrackets(Canvas, PanelX, PanelY, PanelW, PanelH, Cyan, 20.f);

	// Top accent bar
	SBDrawRect(Canvas, PanelX, PanelY, PanelW, 2.f, Cyan);
	SBDrawRect(Canvas, PanelX, PanelY + 2.f, PanelW, 6.f,
		FLinearColor(0.f, 0.3f, 0.6f, 0.12f));

	// Bottom accent bar
	SBDrawRect(Canvas, PanelX, PanelY + PanelH - 2.f, PanelW, 2.f, CyanDim);

	// Left accent stripe
	SBDrawRect(Canvas, PanelX, PanelY, 3.f, TitleHeight, Cyan);

	float CurY = PanelY + Padding;

	// Title bar
	int32 Minutes = (int32)MatchTime / 60;
	int32 Seconds = (int32)MatchTime % 60;

	// Match time (left) with panel
	FString TimeStr = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	SBDrawRect(Canvas, PanelX + Padding, CurY - 2.f, 62.f, 22.f,
		FLinearColor(0.f, 0.15f, 0.3f, 0.3f));
	Canvas->SetDrawColor(0, 200, 255, 220);
	Canvas->DrawText(Font, TimeStr, PanelX + Padding + 5.f, CurY, 1.0f, 1.0f);

	// Title (center) with shadow
	FString Title = TEXT("SCOREBOARD");
	float TitleW, TitleH;
	Canvas->TextSize(Font, Title, TitleW, TitleH);
	float TitleX = PanelX + (PanelW - TitleW) * 0.5f;
	Canvas->SetDrawColor(0, 0, 0, 100);
	Canvas->DrawText(Font, Title, TitleX + 1.f, CurY + 1.f, 1.0f, 1.0f);
	Canvas->SetDrawColor(0, 210, 255, 255);
	Canvas->DrawText(Font, Title, TitleX, CurY, 1.0f, 1.0f);

	// Alive count (right)
	FString AliveStr = FString::Printf(TEXT("%d / %d ALIVE"), AliveCount, TotalPlayers);
	float AW, AH;
	Canvas->TextSize(Font, AliveStr, AW, AH);
	SBDrawRect(Canvas, PanelX + PanelW - Padding - AW - 10.f, CurY - 2.f,
		AW + 14.f, 22.f, FLinearColor(0.f, 0.2f, 0.05f, 0.25f));
	Canvas->SetDrawColor(80, 210, 80, 220);
	Canvas->DrawText(Font, AliveStr, PanelX + PanelW - Padding - AW, CurY, 1.0f, 1.0f);

	CurY += TitleHeight;

	// Header separator with accent
	SBDrawRect(Canvas, PanelX + Padding, CurY - 6.f, PanelW - Padding * 2.f, 1.f, CyanDim);
	SBDrawRect(Canvas, PanelX + Padding, CurY - 6.f, 80.f, 1.f, Cyan);

	// Column headers
	DrawHeader(Canvas, Font, PanelX + Padding, CurY, PanelW - Padding * 2.f);
	CurY += HeaderHeight;

	// Header-to-rows separator
	SBDrawRect(Canvas, PanelX + Padding, CurY - 4.f, PanelW - Padding * 2.f, 1.f,
		FLinearColor(0.15f, 0.2f, 0.3f, 0.5f));

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
	Canvas->SetDrawColor(80, 100, 130, 180);
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

	// Alternating row tint
	if (Rank % 2 == 0)
	{
		SBDrawRect(Canvas, X - 4.f, Y, Width + 8.f, RowHeight - 2.f,
			FLinearColor(0.04f, 0.04f, 0.08f, 0.2f));
	}

	// Local player highlight
	if (bIsLocalPlayer)
	{
		SBDrawRect(Canvas, X - 4.f, Y, Width + 8.f, RowHeight - 2.f,
			FLinearColor(0.f, 0.2f, 0.45f, 0.25f));
		// Left cyan accent stripe
		SBDrawRect(Canvas, X - 4.f, Y + 1.f, 3.f, RowHeight - 4.f,
			FLinearColor(0.f, 0.7f, 1.f, 0.8f));
		// Top/bottom subtle lines
		SBDrawRect(Canvas, X, Y, Width, 1.f,
			FLinearColor(0.f, 0.5f, 0.8f, 0.15f));
		SBDrawRect(Canvas, X, Y + RowHeight - 3.f, Width, 1.f,
			FLinearColor(0.f, 0.5f, 0.8f, 0.15f));
	}

	// Rank number — gold/silver/bronze for top 3
	if (Rank == 1)      Canvas->SetDrawColor(255, 215, 50, 255);
	else if (Rank == 2) Canvas->SetDrawColor(200, 200, 210, 230);
	else if (Rank == 3) Canvas->SetDrawColor(200, 140, 60, 220);
	else if (PS->bIsAlive) Canvas->SetDrawColor(170, 175, 185, 200);
	else Canvas->SetDrawColor(90, 45, 45, 150);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), Rank), X, Y);

	// Player name
	if (bIsLocalPlayer) Canvas->SetDrawColor(0, 220, 255, 255);
	else if (PS->bIsAlive) Canvas->SetDrawColor(200, 210, 220, 240);
	else Canvas->SetDrawColor(100, 50, 50, 160);
	Canvas->DrawText(Font, PS->DisplayName, X + Width * 0.06f, Y);

	// Stats
	if (PS->bIsAlive) Canvas->SetDrawColor(200, 210, 220, 220);
	else Canvas->SetDrawColor(100, 50, 50, 160);

	// Kills — highlight high kill counts
	if (PS->Kills >= 5 && PS->bIsAlive) Canvas->SetDrawColor(255, 200, 50, 255);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->Kills), X + Width * 0.42f, Y);

	if (PS->bIsAlive) Canvas->SetDrawColor(200, 210, 220, 220);
	else Canvas->SetDrawColor(100, 50, 50, 160);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->DamageDealt), X + Width * 0.54f, Y);
	Canvas->DrawText(Font, FString::Printf(TEXT("%d"), PS->Assists), X + Width * 0.68f, Y);

	// Status badge
	if (PS->bIsAlive)
	{
		float StatusX = X + Width * 0.82f;
		SBDrawRect(Canvas, StatusX - 3.f, Y + 2.f, 55.f, RowHeight - 6.f,
			FLinearColor(0.f, 0.35f, 0.1f, 0.35f));
		SBDrawRect(Canvas, StatusX - 3.f, Y + 2.f, 2.f, RowHeight - 6.f,
			FLinearColor(0.f, 0.8f, 0.3f, 0.5f));
		Canvas->SetDrawColor(50, 230, 80, 255);
		Canvas->DrawText(Font, TEXT("ALIVE"), StatusX + 2.f, Y);
	}
	else
	{
		float StatusX = X + Width * 0.82f;
		FString PlaceStr = FString::Printf(TEXT("#%d"), PS->Placement);
		SBDrawRect(Canvas, StatusX - 3.f, Y + 2.f, 45.f, RowHeight - 6.f,
			FLinearColor(0.35f, 0.05f, 0.05f, 0.3f));
		SBDrawRect(Canvas, StatusX - 3.f, Y + 2.f, 2.f, RowHeight - 6.f,
			FLinearColor(0.7f, 0.1f, 0.1f, 0.4f));
		Canvas->SetDrawColor(180, 60, 60, 200);
		Canvas->DrawText(Font, PlaceStr, StatusX + 2.f, Y);
	}
}
