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

	// Gather all player stats
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
		Entry.Placement = ExoPS->Placement;
		Entry.bIsLocalPlayer = (PS == HUD->GetOwningPlayerController()->PlayerState);

		if (Entry.bIsLocalPlayer)
		{
			LocalEntry = Entry;
			LocalPlacement = Entry.Placement;
		}

		Entries.Add(Entry);
	}

	// Sort by placement
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
	// Dark overlay
	HUD->DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.85f),
		0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

	// Central panel
	float PanelW = Canvas->SizeX * 0.5f;
	float PanelH = Canvas->SizeY * 0.7f;
	float PanelX = (Canvas->SizeX - PanelW) * 0.5f;
	float PanelY = (Canvas->SizeY - PanelH) * 0.5f;

	HUD->DrawRect(FLinearColor(0.06f, 0.07f, 0.1f, 0.95f), PanelX, PanelY, PanelW, PanelH);

	// Panel border
	FLinearColor Border(0.2f, 0.3f, 0.5f, 0.8f);
	HUD->DrawLine(PanelX, PanelY, PanelX + PanelW, PanelY, Border, 2.f);
	HUD->DrawLine(PanelX + PanelW, PanelY, PanelX + PanelW, PanelY + PanelH, Border, 2.f);
	HUD->DrawLine(PanelX + PanelW, PanelY + PanelH, PanelX, PanelY + PanelH, Border, 2.f);
	HUD->DrawLine(PanelX, PanelY + PanelH, PanelX, PanelY, Border, 2.f);
}

void FExoMatchSummary::DrawTitle(AHUD* HUD, UCanvas* Canvas, UFont* Font, int32 LocalPlacement)
{
	FString Title;
	FLinearColor TitleColor;

	if (LocalPlacement == 1)
	{
		Title = TEXT("VICTORY");
		TitleColor = FLinearColor(1.f, 0.85f, 0.2f, 1.f); // Gold
	}
	else if (LocalPlacement <= 3)
	{
		Title = FString::Printf(TEXT("#%d — ALMOST"), LocalPlacement);
		TitleColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.f); // Silver
	}
	else
	{
		Title = FString::Printf(TEXT("ELIMINATED — #%d"), LocalPlacement);
		TitleColor = FLinearColor(0.8f, 0.3f, 0.3f, 1.f); // Red
	}

	float TitleW, TitleH;
	HUD->GetTextSize(Title, TitleW, TitleH, Font, 2.f);
	float X = (Canvas->SizeX - TitleW) * 0.5f;
	float Y = Canvas->SizeY * 0.18f;

	HUD->DrawText(Title, TitleColor, X, Y, Font, 2.f);
}

void FExoMatchSummary::DrawLeaderboard(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	const TArray<FMatchSummaryEntry>& Entries)
{
	float StartX = Canvas->SizeX * 0.3f;
	float StartY = Canvas->SizeY * 0.32f;
	float RowH = 28.f;

	// Header
	FLinearColor HeaderColor(0.5f, 0.6f, 0.7f, 0.8f);
	HUD->DrawText(TEXT("#"), HeaderColor, StartX, StartY, Font, 0.9f);
	HUD->DrawText(TEXT("PLAYER"), HeaderColor, StartX + 50.f, StartY, Font, 0.9f);
	HUD->DrawText(TEXT("KILLS"), HeaderColor, StartX + 350.f, StartY, Font, 0.9f);
	StartY += RowH + 5.f;

	// Separator
	HUD->DrawLine(StartX, StartY, StartX + 420.f, StartY,
		FLinearColor(0.3f, 0.3f, 0.4f, 0.5f), 1.f);
	StartY += 5.f;

	// Show top 10
	int32 Count = FMath::Min(Entries.Num(), 10);
	for (int32 i = 0; i < Count; i++)
	{
		const FMatchSummaryEntry& Entry = Entries[i];
		FLinearColor RowColor = Entry.bIsLocalPlayer ?
			FLinearColor(0.3f, 0.8f, 1.f, 1.f) :
			FLinearColor(0.8f, 0.82f, 0.85f, 0.9f);

		// Highlight local player row
		if (Entry.bIsLocalPlayer)
		{
			HUD->DrawRect(FLinearColor(0.1f, 0.2f, 0.3f, 0.4f),
				StartX - 5.f, StartY - 2.f, 430.f, RowH);
		}

		FString PlaceStr = FString::Printf(TEXT("%d"), Entry.Placement);
		FString KillStr = FString::Printf(TEXT("%d"), Entry.Kills);

		HUD->DrawText(PlaceStr, RowColor, StartX, StartY, Font, 0.85f);
		HUD->DrawText(Entry.PlayerName, RowColor, StartX + 50.f, StartY, Font, 0.85f);
		HUD->DrawText(KillStr, RowColor, StartX + 360.f, StartY, Font, 0.85f);

		StartY += RowH;
	}
}

void FExoMatchSummary::DrawPersonalStats(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	const FMatchSummaryEntry& LocalEntry, float MatchDuration)
{
	float StatsX = Canvas->SizeX * 0.3f;
	float StatsY = Canvas->SizeY * 0.72f;

	FLinearColor ValueColor(0.9f, 0.92f, 0.95f, 1.f);

	FString KillStr = FString::Printf(TEXT("Kills: %d"), LocalEntry.Kills);
	FString PlaceStr = FString::Printf(TEXT("Placement: #%d"), LocalEntry.Placement);

	int32 Mins = FMath::FloorToInt(MatchDuration / 60.f);
	int32 Secs = FMath::FloorToInt(FMath::Fmod(MatchDuration, 60.f));
	FString TimeStr = FString::Printf(TEXT("Duration: %d:%02d"), Mins, Secs);

	HUD->DrawText(KillStr, ValueColor, StatsX, StatsY, Font, 1.1f);
	HUD->DrawText(PlaceStr, ValueColor, StatsX + 200.f, StatsY, Font, 1.1f);
	HUD->DrawText(TimeStr, ValueColor, StatsX + 420.f, StatsY, Font, 1.1f);
}

void FExoMatchSummary::DrawCountdownAndOptions(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	float TimeRemaining)
{
	// Countdown timer
	int32 SecsLeft = FMath::CeilToInt(FMath::Max(TimeRemaining, 0.f));
	FString CountdownStr = FString::Printf(TEXT("Returning to lobby in %ds"), SecsLeft);
	float CDW, CDH;
	HUD->GetTextSize(CountdownStr, CDW, CDH, Font, 0.9f);
	float CDX = (Canvas->SizeX - CDW) * 0.5f;
	float CDY = Canvas->SizeY * 0.80f;

	FLinearColor CountdownColor(0.6f, 0.7f, 0.8f, 0.9f);
	HUD->DrawText(CountdownStr, CountdownColor, CDX, CDY, Font, 0.9f);

	// Action options
	float OptY = Canvas->SizeY * 0.86f;
	FLinearColor OptColor(0.3f, 0.8f, 1.f, 0.9f);
	FLinearColor KeyColor(1.f, 0.85f, 0.2f, 1.f);

	// "PLAY AGAIN (Enter)"
	FString PlayAgain = TEXT("[ENTER]  PLAY AGAIN");
	float PAW, PAH;
	HUD->GetTextSize(PlayAgain, PAW, PAH, Font, 1.f);
	float PAX = Canvas->SizeX * 0.35f;

	HUD->DrawRect(FLinearColor(0.08f, 0.1f, 0.15f, 0.8f),
		PAX - 10.f, OptY - 5.f, PAW + 20.f, PAH + 10.f);
	HUD->DrawText(PlayAgain, OptColor, PAX, OptY, Font, 1.f);

	// "MAIN MENU (Backspace)"
	FString MainMenu = TEXT("[BKSP]  MAIN MENU");
	float MMW, MMH;
	HUD->GetTextSize(MainMenu, MMW, MMH, Font, 1.f);
	float MMX = Canvas->SizeX * 0.55f;

	HUD->DrawRect(FLinearColor(0.08f, 0.1f, 0.15f, 0.8f),
		MMX - 10.f, OptY - 5.f, MMW + 20.f, MMH + 10.f);
	HUD->DrawText(MainMenu, OptColor, MMX, OptY, Font, 1.f);
}
