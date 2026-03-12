// ExoMatchSummary.cpp — End-of-match results screen with sci-fi styling
#include "UI/ExoMatchSummary.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Core/ExoGameState.h"
#include "Core/ExoPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void FExoMatchSummary::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!HUD || !Canvas || !Font) return;

	TArray<FMatchSummaryEntry> Entries;
	FMatchSummaryEntry LocalEntry;
	int32 LocalPlacement = 0;
	float MatchDuration = 0.f;
	float TimeRemaining = 0.f;

	AExoGameState* ExoGS = HUD->GetWorld()->GetGameState<AExoGameState>();
	if (!ExoGS) return;

	MatchDuration = ExoGS->MatchElapsedTime;
	TimeRemaining = ExoGS->EndGameTimeRemaining;

	for (APlayerState* PS : ExoGS->PlayerArray)
	{
		AExoPlayerState* ExoPS = Cast<AExoPlayerState>(PS);
		if (!ExoPS) continue;

		FMatchSummaryEntry Entry;
		Entry.PlayerName = ExoPS->DisplayName;
		Entry.Kills = ExoPS->Kills;
		Entry.Deaths = ExoPS->Deaths;
		Entry.DamageDealt = ExoPS->DamageDealt;
		Entry.Placement = ExoPS->Placement;
		Entry.Accuracy = ExoPS->GetAccuracy();
		APlayerController* LocalPC = HUD->GetOwningPlayerController();
		Entry.bIsLocalPlayer = LocalPC && (PS == LocalPC->PlayerState);

		if (Entry.bIsLocalPlayer)
		{
			LocalEntry = Entry;
			LocalPlacement = Entry.Placement;
		}
		Entries.Add(Entry);
	}

	Entries.Sort([](const FMatchSummaryEntry& A, const FMatchSummaryEntry& B)
	{
		return A.Placement < B.Placement;
	});

	DrawBackground(HUD, Canvas);
	DrawTitle(HUD, Canvas, Font, LocalPlacement);
	DrawLeaderboard(HUD, Canvas, Font, Entries);
	DrawPersonalStats(HUD, Canvas, Font, LocalEntry, MatchDuration);
	DrawCountdownAndOptions(HUD, Canvas, Font, TimeRemaining);
}

void FExoMatchSummary::DrawBackground(AHUD* HUD, UCanvas* Canvas)
{
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	// Full screen dark overlay
	HUD->DrawRect(FLinearColor(0.01f, 0.01f, 0.03f, 0.88f), 0.f, 0.f, W, H);

	// Central panel
	float PanelW = W * 0.52f;
	float PanelH = H * 0.72f;
	float PanelX = (W - PanelW) * 0.5f;
	float PanelY = (H - PanelH) * 0.5f;

	HUD->DrawRect(FLinearColor(0.04f, 0.05f, 0.08f, 0.95f), PanelX, PanelY, PanelW, PanelH);

	// Top accent bar (cyan glow)
	HUD->DrawRect(FLinearColor(0.f, 0.6f, 1.f, 0.8f), PanelX, PanelY, PanelW, 3.f);
	HUD->DrawRect(FLinearColor(0.f, 0.3f, 0.6f, 0.12f), PanelX, PanelY + 3.f, PanelW, 8.f);

	// Bottom accent
	HUD->DrawRect(FLinearColor(0.f, 0.4f, 0.7f, 0.4f), PanelX, PanelY + PanelH - 2.f, PanelW, 2.f);

	// Side borders
	FLinearColor SideCol(0.f, 0.35f, 0.6f, 0.25f);
	HUD->DrawLine(PanelX, PanelY, PanelX, PanelY + PanelH, SideCol, 1.f);
	HUD->DrawLine(PanelX + PanelW, PanelY, PanelX + PanelW, PanelY + PanelH, SideCol, 1.f);

	// Corner brackets
	float BLen = 25.f;
	FLinearColor BCol(0.f, 0.7f, 1.f, 0.5f);
	// Top-left
	HUD->DrawLine(PanelX, PanelY, PanelX + BLen, PanelY, BCol, 2.f);
	HUD->DrawLine(PanelX, PanelY, PanelX, PanelY + BLen, BCol, 2.f);
	// Top-right
	HUD->DrawLine(PanelX + PanelW, PanelY, PanelX + PanelW - BLen, PanelY, BCol, 2.f);
	HUD->DrawLine(PanelX + PanelW, PanelY, PanelX + PanelW, PanelY + BLen, BCol, 2.f);
	// Bottom-left
	HUD->DrawLine(PanelX, PanelY + PanelH, PanelX + BLen, PanelY + PanelH, BCol, 2.f);
	HUD->DrawLine(PanelX, PanelY + PanelH, PanelX, PanelY + PanelH - BLen, BCol, 2.f);
	// Bottom-right
	HUD->DrawLine(PanelX + PanelW, PanelY + PanelH, PanelX + PanelW - BLen, PanelY + PanelH, BCol, 2.f);
	HUD->DrawLine(PanelX + PanelW, PanelY + PanelH, PanelX + PanelW, PanelY + PanelH - BLen, BCol, 2.f);
}

void FExoMatchSummary::DrawTitle(AHUD* HUD, UCanvas* Canvas, UFont* Font, int32 LocalPlacement)
{
	FString Title;
	FLinearColor TitleColor;

	if (LocalPlacement == 1)
	{
		Title = TEXT("VICTORY ROYALE");
		TitleColor = FLinearColor(1.f, 0.85f, 0.2f, 1.f);
	}
	else if (LocalPlacement <= 3)
	{
		Title = FString::Printf(TEXT("#%d — SO CLOSE"), LocalPlacement);
		TitleColor = FLinearColor(0.75f, 0.82f, 0.92f, 1.f);
	}
	else
	{
		Title = FString::Printf(TEXT("ELIMINATED — #%d"), LocalPlacement);
		TitleColor = FLinearColor(0.8f, 0.25f, 0.25f, 1.f);
	}

	float TW, TH;
	HUD->GetTextSize(Title, TW, TH, Font, 2.2f);
	float CX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.17f;

	// Title glow
	if (LocalPlacement == 1)
	{
		HUD->DrawRect(FLinearColor(0.6f, 0.4f, 0.f, 0.1f),
			CX - TW * 0.5f - 20.f, Y - 8.f, TW + 40.f, TH + 16.f);
	}

	// Shadow
	HUD->DrawText(Title, FLinearColor(0.f, 0.f, 0.f, 0.5f),
		CX - TW * 0.5f + 2.f, Y + 2.f, Font, 2.2f);
	HUD->DrawText(Title, TitleColor, CX - TW * 0.5f, Y, Font, 2.2f);

	// Accent lines under title
	float LineY = Y + TH + 10.f;
	FLinearColor LineCol(0.f, 0.5f, 0.8f, 0.4f);
	HUD->DrawLine(CX - 150.f, LineY, CX - 20.f, LineY, LineCol, 1.f);
	HUD->DrawLine(CX + 20.f, LineY, CX + 150.f, LineY, LineCol, 1.f);
}

void FExoMatchSummary::DrawLeaderboard(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	const TArray<FMatchSummaryEntry>& Entries)
{
	float CX = Canvas->SizeX * 0.5f;
	float TableW = Canvas->SizeX * 0.4f;
	float StartX = CX - TableW * 0.5f;
	float StartY = Canvas->SizeY * 0.32f;
	float RowH = 26.f;

	// Header
	FLinearColor HeaderCol(0.45f, 0.55f, 0.7f, 0.8f);
	HUD->DrawText(TEXT("#"), HeaderCol, StartX, StartY, Font, 0.85f);
	HUD->DrawText(TEXT("PLAYER"), HeaderCol, StartX + TableW * 0.08f, StartY, Font, 0.85f);
	HUD->DrawText(TEXT("KILLS"), HeaderCol, StartX + TableW * 0.55f, StartY, Font, 0.85f);
	HUD->DrawText(TEXT("DMG"), HeaderCol, StartX + TableW * 0.72f, StartY, Font, 0.85f);
	HUD->DrawText(TEXT("ACC"), HeaderCol, StartX + TableW * 0.88f, StartY, Font, 0.85f);
	StartY += RowH + 4.f;

	// Separator
	HUD->DrawLine(StartX, StartY, StartX + TableW, StartY,
		FLinearColor(0.2f, 0.3f, 0.45f, 0.5f), 1.f);
	StartY += 4.f;

	int32 Count = FMath::Min(Entries.Num(), 10);
	for (int32 i = 0; i < Count; i++)
	{
		const FMatchSummaryEntry& Entry = Entries[i];
		float RY = StartY + i * RowH;

		// Local player highlight
		if (Entry.bIsLocalPlayer)
		{
			HUD->DrawRect(FLinearColor(0.f, 0.25f, 0.5f, 0.2f),
				StartX - 5.f, RY - 2.f, TableW + 10.f, RowH - 2.f);
			// Left accent
			HUD->DrawRect(FLinearColor(0.f, 0.7f, 1.f, 0.6f),
				StartX - 5.f, RY - 2.f, 3.f, RowH - 2.f);
		}

		// Alternating rows
		if (i % 2 == 1)
		{
			HUD->DrawRect(FLinearColor(0.05f, 0.06f, 0.1f, 0.2f),
				StartX - 5.f, RY - 2.f, TableW + 10.f, RowH - 2.f);
		}

		// Rank color (by placement, not array index)
		FLinearColor RankCol;
		if (Entry.Placement == 1) RankCol = FLinearColor(1.f, 0.85f, 0.2f, 1.f);
		else if (Entry.Placement == 2) RankCol = FLinearColor(0.8f, 0.82f, 0.86f, 0.95f);
		else if (Entry.Placement == 3) RankCol = FLinearColor(0.8f, 0.55f, 0.25f, 0.9f);
		else RankCol = FLinearColor(0.7f, 0.72f, 0.78f, 0.8f);

		FLinearColor RowCol = Entry.bIsLocalPlayer
			? FLinearColor(0.3f, 0.85f, 1.f, 1.f) : RankCol;

		HUD->DrawText(FString::Printf(TEXT("%d"), Entry.Placement), RankCol,
			StartX, RY, Font, 0.85f);
		HUD->DrawText(Entry.PlayerName, RowCol,
			StartX + TableW * 0.08f, RY, Font, 0.85f);
		HUD->DrawText(FString::Printf(TEXT("%d"), Entry.Kills), RowCol,
			StartX + TableW * 0.55f, RY, Font, 0.85f);
		HUD->DrawText(FString::Printf(TEXT("%d"), Entry.DamageDealt), RowCol,
			StartX + TableW * 0.72f, RY, Font, 0.85f);
		HUD->DrawText(FString::Printf(TEXT("%.0f%%"), Entry.Accuracy), RowCol,
			StartX + TableW * 0.88f, RY, Font, 0.85f);
	}
}

void FExoMatchSummary::DrawPersonalStats(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	const FMatchSummaryEntry& LocalEntry, float MatchDuration)
{
	float CX = Canvas->SizeX * 0.5f;
	float BaseY = Canvas->SizeY * 0.68f;
	float ColW = 130.f;

	struct FStat { FString Label; FString Value; FLinearColor ValueCol; };
	TArray<FStat> Stats;
	Stats.Add({TEXT("KILLS"), FString::Printf(TEXT("%d"), LocalEntry.Kills),
		FLinearColor(0.9f, 0.92f, 0.95f, 1.f)});
	Stats.Add({TEXT("DEATHS"), FString::Printf(TEXT("%d"), LocalEntry.Deaths),
		FLinearColor(0.9f, 0.92f, 0.95f, 1.f)});
	Stats.Add({TEXT("DAMAGE"), FString::Printf(TEXT("%d"), LocalEntry.DamageDealt),
		FLinearColor(0.9f, 0.92f, 0.95f, 1.f)});

	int32 Mins = FMath::FloorToInt(MatchDuration / 60.f);
	int32 Secs = FMath::FloorToInt(FMath::Fmod(MatchDuration, 60.f));
	Stats.Add({TEXT("ACCURACY"), FString::Printf(TEXT("%.1f%%"), LocalEntry.Accuracy),
		FLinearColor(0.9f, 0.92f, 0.95f, 1.f)});
	Stats.Add({TEXT("SURVIVED"), FString::Printf(TEXT("%d:%02d"), Mins, Secs),
		FLinearColor(0.9f, 0.92f, 0.95f, 1.f)});

	float TotalW = (Stats.Num() - 1) * ColW;
	float StartX = CX - TotalW * 0.5f;

	// "YOUR STATS" header
	FString Header = TEXT("YOUR PERFORMANCE");
	float HW, HH;
	HUD->GetTextSize(Header, HW, HH, Font, 0.9f);
	HUD->DrawText(Header, FLinearColor(0.f, 0.7f, 1.f, 0.8f),
		CX - HW * 0.5f, BaseY - HH - 14.f, Font, 0.9f);

	// Separator
	HUD->DrawLine(StartX - 10.f, BaseY - 6.f, StartX + TotalW + 10.f, BaseY - 6.f,
		FLinearColor(0.f, 0.4f, 0.7f, 0.35f), 1.f);

	// Background panel
	HUD->DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.5f),
		StartX - 20.f, BaseY - 8.f, TotalW + 40.f, 56.f);

	FLinearColor LabelCol(0.5f, 0.55f, 0.62f, 0.9f);

	for (int32 i = 0; i < Stats.Num(); i++)
	{
		float X = StartX + i * ColW;

		float LW, LH;
		HUD->GetTextSize(Stats[i].Label, LW, LH, Font, 0.7f);
		HUD->DrawText(Stats[i].Label, LabelCol, X - LW * 0.5f, BaseY, Font, 0.7f);

		float VW, VH;
		HUD->GetTextSize(Stats[i].Value, VW, VH, Font, 1.1f);
		HUD->DrawText(Stats[i].Value, Stats[i].ValueCol,
			X - VW * 0.5f, BaseY + LH + 4.f, Font, 1.1f);
	}
}

void FExoMatchSummary::DrawCountdownAndOptions(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float TimeRemaining)
{
	float CX = Canvas->SizeX * 0.5f;

	int32 SecsLeft = FMath::CeilToInt(FMath::Max(TimeRemaining, 0.f));
	FString CountdownStr = FString::Printf(TEXT("Returning to lobby in %ds"), SecsLeft);
	float CDW, CDH;
	HUD->GetTextSize(CountdownStr, CDW, CDH, Font, 0.85f);
	HUD->DrawText(CountdownStr, FLinearColor(0.55f, 0.6f, 0.7f, 0.85f),
		CX - CDW * 0.5f, Canvas->SizeY * 0.80f, Font, 0.85f);

	// Action buttons
	float BtnY = Canvas->SizeY * 0.85f;
	float BtnW = 180.f;
	float BtnH = 32.f;
	float BtnGap = 30.f;

	// Play Again
	float PAX = CX - BtnW - BtnGap * 0.5f;
	HUD->DrawRect(FLinearColor(0.04f, 0.08f, 0.14f, 0.85f), PAX, BtnY, BtnW, BtnH);
	HUD->DrawLine(PAX, BtnY, PAX + BtnW, BtnY, FLinearColor(0.f, 0.5f, 0.8f, 0.5f), 1.f);
	HUD->DrawLine(PAX, BtnY + BtnH, PAX + BtnW, BtnY + BtnH, FLinearColor(0.f, 0.5f, 0.8f, 0.5f), 1.f);
	FString PlayStr = TEXT("[ENTER]  PLAY AGAIN");
	float PW, PH;
	HUD->GetTextSize(PlayStr, PW, PH, Font, 0.9f);
	HUD->DrawText(PlayStr, FLinearColor(0.3f, 0.8f, 1.f, 0.9f),
		PAX + (BtnW - PW) * 0.5f, BtnY + (BtnH - PH) * 0.5f, Font, 0.9f);

	// Main Menu
	float MMX = CX + BtnGap * 0.5f;
	HUD->DrawRect(FLinearColor(0.04f, 0.08f, 0.14f, 0.85f), MMX, BtnY, BtnW, BtnH);
	HUD->DrawLine(MMX, BtnY, MMX + BtnW, BtnY, FLinearColor(0.3f, 0.3f, 0.4f, 0.4f), 1.f);
	HUD->DrawLine(MMX, BtnY + BtnH, MMX + BtnW, BtnY + BtnH, FLinearColor(0.3f, 0.3f, 0.4f, 0.4f), 1.f);
	FString MenuStr = TEXT("[BKSP]  MAIN MENU");
	float MW, MH;
	HUD->GetTextSize(MenuStr, MW, MH, Font, 0.9f);
	HUD->DrawText(MenuStr, FLinearColor(0.6f, 0.65f, 0.7f, 0.85f),
		MMX + (BtnW - MW) * 0.5f, BtnY + (BtnH - MH) * 0.5f, Font, 0.9f);
}
