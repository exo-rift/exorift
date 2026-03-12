// ExoHUDWorldPrompts.cpp — Supply drop announcement, location banner, zipline prompt
#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoGameState.h"
#include "Map/ExoZipline.h"
#include "UI/ExoLocationNames.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"

void AExoHUD::DrawSupplyDropAnnouncement()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS || GS->AnnouncementTimer <= 0.f) return;

	GS->AnnouncementTimer -= GetWorld()->GetDeltaSeconds();
	float T = GS->AnnouncementTimer;

	// Fade: full alpha for first 2s, then fade out
	float Alpha = FMath::Clamp(T / 1.5f, 0.f, 1.f);
	// Scale-in effect: starts slightly larger
	float ScaleIn = FMath::Clamp(1.f - (T - 3.f) * 2.f, 0.f, 1.f);
	ScaleIn = FMath::Max(ScaleIn, 0.f);

	FString AnnText = GS->SupplyDropAnnouncement;
	float TW, TH;
	GetTextSize(AnnText, TW, TH, HUDFont, 1.3f);
	float PanelW = TW + 50.f;
	float PanelH = TH + 18.f;
	float X = (Canvas->SizeX - PanelW) * 0.5f;
	float Y = Canvas->SizeY * 0.18f;

	// Panel background
	DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, 0.7f * Alpha), X, Y, PanelW, PanelH);

	// Top and bottom accent bars (yellow-gold)
	FLinearColor AccCol(1.f, 0.8f, 0.1f, 0.6f * Alpha);
	DrawRect(AccCol, X, Y, PanelW, 2.f);
	DrawRect(FLinearColor(AccCol.R, AccCol.G, AccCol.B, 0.3f * Alpha),
		X, Y + PanelH - 1.f, PanelW, 1.f);

	// Diamond icon before text
	float DX = X + 15.f;
	float DY = Y + PanelH * 0.5f;
	float DS = 5.f;
	FLinearColor DiamCol(1.f, 0.85f, 0.15f, Alpha);
	DrawLine(DX, DY - DS, DX + DS, DY, DiamCol, 1.5f);
	DrawLine(DX + DS, DY, DX, DY + DS, DiamCol, 1.5f);
	DrawLine(DX, DY + DS, DX - DS, DY, DiamCol, 1.5f);
	DrawLine(DX - DS, DY, DX, DY - DS, DiamCol, 1.5f);

	// Main text
	DrawText(AnnText, FLinearColor(1.f, 0.88f, 0.2f, Alpha),
		X + (PanelW - TW) * 0.5f, Y + (PanelH - TH) * 0.5f, HUDFont, 1.3f);

	if (T <= 0.f) GS->SupplyDropAnnouncement.Empty();
}

void AExoHUD::DrawLocationBanner()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	float DT = GetWorld()->GetDeltaSeconds();
	FExoLocationNames::Tick(DT, Char->GetActorLocation());
	FExoLocationNames::Draw(this, Canvas, HUDFont);
}

void AExoHUD::DrawZiplinePrompt()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char) return;

	const FVector PlayerLoc = Char->GetActorLocation();
	constexpr float ProximityRadius = 400.f;
	float ClosestDist = ProximityRadius;
	bool bFound = false;

	for (TActorIterator<AExoZipline> It(GetWorld()); It; ++It)
	{
		AExoZipline* Zip = *It;
		float DistStart = FVector::Distance(PlayerLoc, Zip->GetStartPoint());
		float DistEnd = FVector::Distance(PlayerLoc, Zip->GetEndPoint());
		float MinDist = FMath::Min(DistStart, DistEnd);
		if (MinDist < ClosestDist)
		{
			ClosestDist = MinDist;
			bFound = true;
		}
	}

	if (!bFound) return;

	// Prompt text and sizing
	const FString Prompt = TEXT("PRESS [E] TO RIDE ZIPLINE");
	float TextW, TextH;
	GetTextSize(Prompt, TextW, TextH, HUDFont, 0.9f);
	float PanelW = TextW + 50.f;
	float PanelH = TextH + 20.f;
	float X = (Canvas->SizeX - PanelW) * 0.5f;
	float Y = Canvas->SizeY * 0.58f;
	float Time = GetWorld()->GetTimeSeconds();

	// Fade based on proximity (stronger when closer)
	float ProxAlpha = 1.f - (ClosestDist / ProximityRadius);
	ProxAlpha = FMath::Clamp(ProxAlpha, 0.3f, 1.f);

	// Panel background — dark sci-fi
	DrawRect(FLinearColor(0.01f, 0.02f, 0.06f, 0.8f * ProxAlpha),
		X, Y, PanelW, PanelH);

	// Top and bottom accent bars (pulsing cyan-teal)
	float Pulse = 0.5f + 0.35f * FMath::Sin(Time * 3.5f);
	FLinearColor AccentCol(0.f, 0.75f, 1.f, 0.7f * Pulse * ProxAlpha);
	DrawRect(AccentCol, X, Y, PanelW, 2.f);
	DrawRect(FLinearColor(AccentCol.R, AccentCol.G, AccentCol.B,
		0.3f * Pulse * ProxAlpha), X, Y + PanelH - 1.f, PanelW, 1.f);

	// Corner bracket accents
	float BLen = 12.f;
	FLinearColor BCol(0.f, 0.8f, 1.f, 0.5f * ProxAlpha);
	DrawLine(X, Y, X + BLen, Y, BCol);
	DrawLine(X, Y, X, Y + BLen, BCol);
	DrawLine(X + PanelW, Y, X + PanelW - BLen, Y, BCol);
	DrawLine(X + PanelW, Y, X + PanelW, Y + BLen, BCol);
	DrawLine(X, Y + PanelH, X + BLen, Y + PanelH, BCol);
	DrawLine(X, Y + PanelH, X, Y + PanelH - BLen, BCol);
	DrawLine(X + PanelW, Y + PanelH, X + PanelW - BLen, Y + PanelH, BCol);
	DrawLine(X + PanelW, Y + PanelH, X + PanelW, Y + PanelH - BLen, BCol);

	// Prompt text — bright cyan-white
	FLinearColor TextCol(0.8f, 0.95f, 1.f, 0.95f * ProxAlpha);
	DrawText(Prompt, TextCol,
		X + (PanelW - TextW) * 0.5f,
		Y + (PanelH - TextH) * 0.5f, HUDFont, 0.9f);
}
