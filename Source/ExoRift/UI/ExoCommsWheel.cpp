#include "UI/ExoCommsWheel.h"
#include "UI/ExoPingSystem.h"
#include "Core/ExoAudioManager.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

bool FExoCommsWheel::bIsOpen = false;
int32 FExoCommsWheel::HighlightedOption = -1;
FVector2D FExoCommsWheel::MouseDelta = FVector2D::ZeroVector;

static const FString CommTexts[FExoCommsWheel::NumOptions] =
{
	TEXT("Enemy Spotted"),
	TEXT("Need Energy"),
	TEXT("Going Here"),
	TEXT("Help"),
	TEXT("Thanks"),
	TEXT("Good Game"),
	TEXT("Fall Back"),
	TEXT("Push Forward")
};

static const TCHAR* CommIcons[FExoCommsWheel::NumOptions] =
{
	TEXT("!"), TEXT("E"), TEXT(">"), TEXT("?"),
	TEXT("+"), TEXT("G"), TEXT("<"), TEXT("^")
};

static const FLinearColor CommColors[FExoCommsWheel::NumOptions] =
{
	FLinearColor(1.f, 0.3f, 0.3f, 1.f),   // Enemy Spotted
	FLinearColor(0.3f, 0.9f, 0.4f, 1.f),   // Need Energy
	FLinearColor(0.3f, 0.6f, 1.f, 1.f),    // Going Here
	FLinearColor(1.f, 0.8f, 0.1f, 1.f),    // Help
	FLinearColor(0.5f, 0.9f, 0.5f, 1.f),   // Thanks
	FLinearColor(0.8f, 0.8f, 0.9f, 1.f),   // Good Game
	FLinearColor(1.f, 0.6f, 0.2f, 1.f),    // Fall Back
	FLinearColor(0.2f, 0.8f, 1.f, 1.f)     // Push Forward
};

void FExoCommsWheel::Open()
{
	bIsOpen = true;
	HighlightedOption = -1;
	MouseDelta = FVector2D::ZeroVector;
}

void FExoCommsWheel::Close(UWorld* World)
{
	if (bIsOpen && HighlightedOption >= 0 && HighlightedOption < NumOptions)
	{
		SendComm(HighlightedOption, World);
	}
	bIsOpen = false;
	HighlightedOption = -1;
	MouseDelta = FVector2D::ZeroVector;
}

void FExoCommsWheel::UpdateMouse(FVector2D Delta)
{
	if (!bIsOpen) return;

	MouseDelta += Delta;

	float Dist = MouseDelta.Size();
	if (Dist < InnerDeadzone)
	{
		HighlightedOption = -1;
		return;
	}

	float AngleRad = FMath::Atan2(MouseDelta.X, -MouseDelta.Y);
	if (AngleRad < 0.f) AngleRad += 2.f * PI;

	float SectorSize = (2.f * PI) / static_cast<float>(NumOptions);
	HighlightedOption = FMath::FloorToInt(AngleRad / SectorSize) % NumOptions;
}

void FExoCommsWheel::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!bIsOpen || !HUD || !Canvas || !Font) return;

	const float CX = Canvas->SizeX * 0.5f;
	const float CY = Canvas->SizeY * 0.5f;
	const float SectorAngle = (2.f * PI) / static_cast<float>(NumOptions);
	const float Time = HUD->GetWorld()->GetTimeSeconds();

	// Full-screen dim overlay
	HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

	// --- Draw sector background rects behind labels ---
	for (int32 i = 0; i < NumOptions; ++i)
	{
		float MidAngle = i * SectorAngle - PI * 0.5f + SectorAngle * 0.5f;
		float LabelDist = WheelRadius * 0.65f;
		float LX = CX + FMath::Cos(MidAngle) * LabelDist;
		float LY = CY + FMath::Sin(MidAngle) * LabelDist;

		bool bHighlighted = (i == HighlightedOption);

		FString Label = CommTexts[i];
		float TW, TH;
		HUD->GetTextSize(Label, TW, TH, Font, 0.75f);
		float PadX = 18.f, PadY = 8.f;
		float RX = LX - TW * 0.5f - PadX;
		float RY = LY - TH * 0.5f - PadY;
		float RW = TW + PadX * 2.f;
		float RH = TH + PadY * 2.f;

		FLinearColor SectorBg = bHighlighted
			? FLinearColor(0.1f, 0.12f, 0.18f, 0.92f)
			: FLinearColor(0.04f, 0.04f, 0.07f, 0.8f);
		HUD->DrawRect(SectorBg, RX, RY, RW, RH);

		if (bHighlighted)
		{
			// Accent border using sector color
			FLinearColor Acc = CommColors[i];
			Acc.A = 0.7f + 0.2f * FMath::Abs(FMath::Sin(Time * 4.f));

			// Left accent stripe
			HUD->DrawRect(Acc, RX, RY, 3.f, RH);
			// Top accent line
			HUD->DrawRect(FLinearColor(Acc.R, Acc.G, Acc.B, 0.4f), RX, RY, RW, 1.f);
			// Bottom accent line
			HUD->DrawRect(FLinearColor(Acc.R, Acc.G, Acc.B, 0.25f), RX, RY + RH - 1.f, RW, 1.f);

			// Corner brackets
			float BL = 8.f;
			HUD->DrawLine(RX, RY, RX + BL, RY, Acc, 1.5f);
			HUD->DrawLine(RX, RY, RX, RY + BL, Acc, 1.5f);
			HUD->DrawLine(RX + RW, RY + RH, RX + RW - BL, RY + RH, Acc, 1.5f);
			HUD->DrawLine(RX + RW, RY + RH, RX + RW, RY + RH - BL, Acc, 1.5f);
		}
		else
		{
			// Subtle border
			FLinearColor Bord(0.2f, 0.25f, 0.3f, 0.3f);
			HUD->DrawLine(RX, RY, RX + RW, RY, Bord, 0.5f);
			HUD->DrawLine(RX, RY + RH, RX + RW, RY + RH, Bord, 0.5f);
			HUD->DrawLine(RX, RY, RX, RY + RH, Bord, 0.5f);
			HUD->DrawLine(RX + RW, RY, RX + RW, RY + RH, Bord, 0.5f);
		}
	}

	// --- Divider lines between sectors (from inner to outer ring) ---
	for (int32 i = 0; i < NumOptions; ++i)
	{
		float Angle = i * SectorAngle - PI * 0.5f;
		float X0 = CX + FMath::Cos(Angle) * (InnerDeadzone + 5.f);
		float Y0 = CY + FMath::Sin(Angle) * (InnerDeadzone + 5.f);
		float X1 = CX + FMath::Cos(Angle) * (InnerDeadzone + 20.f);
		float Y1 = CY + FMath::Sin(Angle) * (InnerDeadzone + 20.f);
		HUD->DrawLine(X0, Y0, X1, Y1, FLinearColor(0.35f, 0.4f, 0.5f, 0.35f), 1.f);
	}

	// --- Inner ring ---
	constexpr int32 RingSegs = 48;
	float RingStep = (2.f * PI) / static_cast<float>(RingSegs);
	FLinearColor InnerRingCol(0.3f, 0.5f, 0.7f, 0.4f);
	for (int32 i = 0; i < RingSegs; ++i)
	{
		float A0 = i * RingStep;
		float A1 = (i + 1) * RingStep;
		float R = InnerDeadzone + 3.f;
		HUD->DrawLine(
			CX + FMath::Cos(A0) * R, CY + FMath::Sin(A0) * R,
			CX + FMath::Cos(A1) * R, CY + FMath::Sin(A1) * R,
			InnerRingCol, 1.f);
	}

	// --- Labels + icon letters ---
	for (int32 i = 0; i < NumOptions; ++i)
	{
		float MidAngle = i * SectorAngle - PI * 0.5f + SectorAngle * 0.5f;
		float LabelDist = WheelRadius * 0.65f;
		float LX = CX + FMath::Cos(MidAngle) * LabelDist;
		float LY = CY + FMath::Sin(MidAngle) * LabelDist;

		bool bHighlighted = (i == HighlightedOption);
		FLinearColor TextColor = bHighlighted ? CommColors[i] : FLinearColor(0.6f, 0.65f, 0.7f, 0.85f);
		float Scale = bHighlighted ? 0.85f : 0.75f;

		// Icon letter above label
		FString Icon = CommIcons[i];
		float IW, IH;
		HUD->GetTextSize(Icon, IW, IH, Font, 0.7f);
		FLinearColor IconCol = bHighlighted
			? FLinearColor(CommColors[i].R, CommColors[i].G, CommColors[i].B, 0.9f)
			: FLinearColor(0.5f, 0.55f, 0.6f, 0.6f);
		HUD->DrawText(Icon, IconCol, LX - IW * 0.5f, LY - 18.f, Font, 0.7f);

		// Label text
		FString Label = CommTexts[i];
		float TW, TH;
		HUD->GetTextSize(Label, TW, TH, Font, Scale);
		HUD->DrawText(Label, TextColor, LX - TW * 0.5f, LY - TH * 0.5f + 3.f, Font, Scale);
	}

	// --- Center diamond indicator ---
	float DotR = 4.f;
	FLinearColor CenterCol = (HighlightedOption >= 0)
		? FLinearColor(CommColors[HighlightedOption].R, CommColors[HighlightedOption].G,
			CommColors[HighlightedOption].B, 0.9f)
		: FLinearColor(0.5f, 0.65f, 0.8f, 0.8f);
	HUD->DrawLine(CX, CY - DotR, CX + DotR, CY, CenterCol, 1.5f);
	HUD->DrawLine(CX + DotR, CY, CX, CY + DotR, CenterCol, 1.5f);
	HUD->DrawLine(CX, CY + DotR, CX - DotR, CY, CenterCol, 1.5f);
	HUD->DrawLine(CX - DotR, CY, CX, CY - DotR, CenterCol, 1.5f);

	// --- Selection direction line from center to highlighted sector ---
	if (HighlightedOption >= 0)
	{
		float MidAngle = HighlightedOption * SectorAngle - PI * 0.5f + SectorAngle * 0.5f;
		float LineLen = InnerDeadzone + 8.f;
		FLinearColor LineCol = CommColors[HighlightedOption];
		LineCol.A = 0.35f;
		HUD->DrawLine(CX, CY,
			CX + FMath::Cos(MidAngle) * LineLen,
			CY + FMath::Sin(MidAngle) * LineLen,
			LineCol, 1.5f);
	}

	// --- "COMMS" label at top ---
	FString Title = TEXT("COMMS");
	float TitleW, TitleH;
	HUD->GetTextSize(Title, TitleW, TitleH, Font, 0.7f);
	float TitleX = CX - TitleW * 0.5f;
	float TitleY = CY - WheelRadius - 30.f;
	HUD->DrawRect(FLinearColor(0.02f, 0.02f, 0.05f, 0.7f),
		TitleX - 10.f, TitleY - 3.f, TitleW + 20.f, TitleH + 6.f);
	HUD->DrawRect(FLinearColor(0.f, 0.5f, 0.8f, 0.4f),
		TitleX - 10.f, TitleY + TitleH + 3.f, TitleW + 20.f, 1.f);
	HUD->DrawText(Title, FLinearColor(0.6f, 0.75f, 0.9f, 0.9f), TitleX, TitleY, Font, 0.7f);
}

void FExoCommsWheel::SendComm(int32 Index, UWorld* World)
{
	if (Index < 0 || Index >= NumOptions || !World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	FVector PawnLoc = PC->GetPawn()->GetActorLocation();
	FExoPingSystem::AddPing(PawnLoc, GetCommText(Index), CommColors[Index]);

	// Audio feedback: softer confirmation chirp for comms pings
	if (UExoAudioManager* Audio = UExoAudioManager::Get(World))
	{
		Audio->PlayAbilityActivateSound(PawnLoc, 1.6f);
	}
}

FString FExoCommsWheel::GetCommText(int32 Index)
{
	if (Index >= 0 && Index < NumOptions)
	{
		return CommTexts[Index];
	}
	return FString();
}
