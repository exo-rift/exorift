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

	AGameStateBase* GS = HUD->GetWorld()->GetGameState<AGameStateBase>();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
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
	DrawPersonalStats(HUD, Canvas, Font, LocalEntry);
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
	const FMatchSummaryEntry& LocalEntry)
{
	float StatsX = Canvas->SizeX * 0.3f;
	float StatsY = Canvas->SizeY * 0.75f;

	FLinearColor LabelColor(0.5f, 0.6f, 0.7f, 0.8f);
	FLinearColor ValueColor(0.9f, 0.92f, 0.95f, 1.f);

	FString KillStr = FString::Printf(TEXT("Kills: %d"), LocalEntry.Kills);
	FString PlaceStr = FString::Printf(TEXT("Placement: #%d"), LocalEntry.Placement);

	HUD->DrawText(KillStr, ValueColor, StatsX, StatsY, Font, 1.1f);
	HUD->DrawText(PlaceStr, ValueColor, StatsX + 250.f, StatsY, Font, 1.1f);
}
