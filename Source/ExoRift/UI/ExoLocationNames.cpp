// ExoLocationNames.cpp — Named map regions with enter/exit banner
#include "UI/ExoLocationNames.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

FString FExoLocationNames::CurrentLocation;
FString FExoLocationNames::DisplayLocation;
float FExoLocationNames::TransitionAlpha = 0.f;
float FExoLocationNames::DisplayTimer = 0.f;
bool FExoLocationNames::bShowingBanner = false;

const TArray<FExoLocationNames::FNamedRegion>& FExoLocationNames::GetRegions()
{
	static TArray<FNamedRegion> Regions = {
		// Central hub
		{TEXT("COMMAND CENTER"), FVector(0.f, 0.f, 0.f), 15000.f},
		// Cardinal compounds
		{TEXT("INDUSTRIAL DISTRICT"), FVector(0.f, 16000.f, 0.f), 5000.f},
		{TEXT("RESEARCH LABS"), FVector(0.f, -16000.f, 0.f), 5000.f},
		{TEXT("POWER STATION"), FVector(16000.f, 0.f, 0.f), 5000.f},
		{TEXT("BARRACKS"), FVector(-16000.f, 0.f, 0.f), 5000.f},
		// Corner outposts
		{TEXT("OUTPOST ALPHA"), FVector(24000.f, 24000.f, 0.f), 4000.f},
		{TEXT("OUTPOST BRAVO"), FVector(-24000.f, 24000.f, 0.f), 4000.f},
		{TEXT("OUTPOST CHARLIE"), FVector(24000.f, -24000.f, 0.f), 4000.f},
		{TEXT("OUTPOST DELTA"), FVector(-24000.f, -24000.f, 0.f), 4000.f},
		// Scattered landmarks
		{TEXT("CROSSROADS"), FVector(8000.f, 8000.f, 0.f), 3000.f},
		{TEXT("THE RUINS"), FVector(-10000.f, 10000.f, 0.f), 3000.f},
		{TEXT("SCRAPYARD"), FVector(12000.f, -8000.f, 0.f), 3000.f},
		{TEXT("DARK HOLLOW"), FVector(-6000.f, -10000.f, 0.f), 3000.f},
		{TEXT("WATCHTOWER"), FVector(4000.f, -14000.f, 0.f), 3000.f},
		{TEXT("RELAY STATION"), FVector(-14000.f, -4000.f, 0.f), 3000.f},
		{TEXT("OVERLOOK"), FVector(10000.f, 14000.f, 0.f), 3000.f},
		{TEXT("SIGNAL POINT"), FVector(-20000.f, -16000.f, 0.f), 3000.f},
		{TEXT("CITADEL"), FVector(18000.f, 10000.f, 0.f), 3000.f},
		{TEXT("NORTHERN RIDGE"), FVector(-4000.f, 24000.f, 0.f), 4000.f},
	};
	return Regions;
}

FString FExoLocationNames::GetLocationAt(const FVector& WorldPos)
{
	const auto& Regions = GetRegions();
	for (const FNamedRegion& R : Regions)
	{
		float Dist2D = FVector::Dist2D(WorldPos, R.Center);
		if (Dist2D < R.Radius)
			return R.Name;
	}
	return FString();
}

void FExoLocationNames::Tick(float DeltaTime, const FVector& PlayerLocation)
{
	FString NewLocation = GetLocationAt(PlayerLocation);

	if (NewLocation != CurrentLocation)
	{
		CurrentLocation = NewLocation;
		if (!CurrentLocation.IsEmpty())
		{
			DisplayLocation = CurrentLocation;
			bShowingBanner = true;
			TransitionAlpha = 0.f;
			DisplayTimer = 0.f;
		}
	}

	if (bShowingBanner)
	{
		DisplayTimer += DeltaTime;

		// Phase 1: fade in (0-0.5s)
		if (DisplayTimer < 0.5f)
		{
			TransitionAlpha = FMath::Clamp(DisplayTimer / 0.5f, 0.f, 1.f);
		}
		// Phase 2: hold (0.5-3s)
		else if (DisplayTimer < 3.f)
		{
			TransitionAlpha = 1.f;
		}
		// Phase 3: fade out (3-4s)
		else if (DisplayTimer < 4.f)
		{
			TransitionAlpha = 1.f - FMath::Clamp((DisplayTimer - 3.f) / 1.f, 0.f, 1.f);
		}
		else
		{
			bShowingBanner = false;
			TransitionAlpha = 0.f;
		}
	}
}

void FExoLocationNames::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!bShowingBanner || TransitionAlpha <= 0.01f || !Canvas || !Font) return;

	float CX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.2f;

	// Slide in from above during fade-in
	float SlideOffset = (1.f - TransitionAlpha) * 20.f;
	Y -= SlideOffset;

	float Alpha = TransitionAlpha;

	// Location name — large centered text
	float TextW = 0.f, TextH = 0.f;
	HUD->GetTextSize(DisplayLocation, TextW, TextH, Font, 1.5f);
	float TextX = CX - TextW * 0.5f;

	// Background panel
	float PadX = 30.f, PadY = 10.f;
	float PanelW = TextW + PadX * 2.f;
	float PanelH = TextH + PadY * 2.f + 18.f; // Extra for sub-text
	float PanelX = CX - PanelW * 0.5f;
	float PanelY = Y - PadY;

	HUD->DrawRect(FLinearColor(0.03f, 0.03f, 0.05f, 0.65f * Alpha),
		PanelX, PanelY, PanelW, PanelH);

	// Top accent line
	HUD->DrawRect(FLinearColor(0.2f, 0.6f, 1.f, 0.8f * Alpha),
		PanelX, PanelY, PanelW, 2.f);

	// Location name
	FLinearColor TextCol(0.85f, 0.9f, 1.f, Alpha);
	Canvas->SetDrawColor(TextCol.R * 255, TextCol.G * 255, TextCol.B * 255,
		FMath::RoundToInt32(Alpha * 255));
	HUD->DrawText(DisplayLocation, TextCol, TextX, Y, Font, 1.5f);

	// Sub-text "ENTERING AREA"
	FString SubText = TEXT("ENTERING AREA");
	float SubW = 0.f, SubH = 0.f;
	HUD->GetTextSize(SubText, SubW, SubH, Font, 0.8f);
	FLinearColor SubCol(0.5f, 0.65f, 0.8f, Alpha * 0.7f);
	HUD->DrawText(SubText, SubCol, CX - SubW * 0.5f, Y + TextH + 2.f, Font, 0.8f);

	// Corner brackets
	float BL = 8.f;
	FLinearColor BrCol(0.3f, 0.6f, 1.f, Alpha * 0.5f);
	HUD->DrawLine(PanelX, PanelY, PanelX + BL, PanelY, BrCol);
	HUD->DrawLine(PanelX, PanelY, PanelX, PanelY + BL, BrCol);
	HUD->DrawLine(PanelX + PanelW, PanelY + PanelH,
		PanelX + PanelW - BL, PanelY + PanelH, BrCol);
	HUD->DrawLine(PanelX + PanelW, PanelY + PanelH,
		PanelX + PanelW, PanelY + PanelH - BL, BrCol);
}
