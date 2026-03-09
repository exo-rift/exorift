#include "UI/ExoCommsWheel.h"
#include "UI/ExoPingSystem.h"
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

static const FLinearColor CommColors[FExoCommsWheel::NumOptions] =
{
	FLinearColor(1.f, 0.3f, 0.3f, 1.f),   // Enemy Spotted — red
	FLinearColor(0.3f, 0.9f, 0.4f, 1.f),   // Need Energy — green
	FLinearColor(0.3f, 0.6f, 1.f, 1.f),    // Going Here — blue
	FLinearColor(1.f, 0.8f, 0.1f, 1.f),    // Help — yellow
	FLinearColor(0.5f, 0.9f, 0.5f, 1.f),   // Thanks — light green
	FLinearColor(0.8f, 0.8f, 0.9f, 1.f),   // Good Game — light grey
	FLinearColor(1.f, 0.6f, 0.2f, 1.f),    // Fall Back — orange
	FLinearColor(0.2f, 0.8f, 1.f, 1.f)     // Push Forward — cyan
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

	// Compute angle from mouse delta (0 = up, clockwise)
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

	// Dark circular background — draw as filled sectors
	for (int32 i = 0; i < NumOptions; ++i)
	{
		float StartAngle = i * SectorAngle - PI * 0.5f; // Offset so sector 0 is top
		bool bHighlighted = (i == HighlightedOption);

		FLinearColor SectorColor = bHighlighted
			? FLinearColor(0.15f, 0.15f, 0.2f, 0.9f)
			: FLinearColor(0.05f, 0.05f, 0.08f, BackgroundAlpha);

		// Draw sector as triangle fan segments
		constexpr int32 Segments = 8;
		float SegAngle = SectorAngle / static_cast<float>(Segments);

		for (int32 s = 0; s < Segments; ++s)
		{
			float A0 = StartAngle + s * SegAngle;
			float A1 = StartAngle + (s + 1) * SegAngle;

			FVector2D P0(CX + FMath::Cos(A0) * InnerDeadzone, CY + FMath::Sin(A0) * InnerDeadzone);
			FVector2D P1(CX + FMath::Cos(A0) * WheelRadius, CY + FMath::Sin(A0) * WheelRadius);
			FVector2D P2(CX + FMath::Cos(A1) * WheelRadius, CY + FMath::Sin(A1) * WheelRadius);
			FVector2D P3(CX + FMath::Cos(A1) * InnerDeadzone, CY + FMath::Sin(A1) * InnerDeadzone);

			// Fill each quad with two triangles drawn as thick lines
			HUD->DrawLine(P0.X, P0.Y, P1.X, P1.Y, SectorColor, 1.f);
			HUD->DrawLine(P1.X, P1.Y, P2.X, P2.Y, SectorColor, 1.f);
			HUD->DrawLine(P2.X, P2.Y, P3.X, P3.Y, SectorColor, 1.f);
		}

		// Filled background rectangle per sector using DrawRect
		float MidAngle = StartAngle + SectorAngle * 0.5f;
		float LabelDist = WheelRadius * 0.65f;
		float LX = CX + FMath::Cos(MidAngle) * LabelDist;
		float LY = CY + FMath::Sin(MidAngle) * LabelDist;

		// Background rect behind label
		FString Label = CommTexts[i];
		float TW, TH;
		HUD->GetTextSize(Label, TW, TH, Font, 0.75f);
		float RX = LX - TW * 0.5f - 6.f;
		float RY = LY - TH * 0.5f - 3.f;
		HUD->DrawRect(SectorColor, RX, RY, TW + 12.f, TH + 6.f);
	}

	// Draw divider lines between sectors
	for (int32 i = 0; i < NumOptions; ++i)
	{
		float Angle = i * SectorAngle - PI * 0.5f;
		float X0 = CX + FMath::Cos(Angle) * InnerDeadzone;
		float Y0 = CY + FMath::Sin(Angle) * InnerDeadzone;
		float X1 = CX + FMath::Cos(Angle) * WheelRadius;
		float Y1 = CY + FMath::Sin(Angle) * WheelRadius;
		HUD->DrawLine(X0, Y0, X1, Y1, FLinearColor(0.3f, 0.35f, 0.4f, 0.6f), 1.f);
	}

	// Draw outer ring
	constexpr int32 RingSegs = 64;
	float RingStep = (2.f * PI) / static_cast<float>(RingSegs);
	for (int32 i = 0; i < RingSegs; ++i)
	{
		float A0 = i * RingStep;
		float A1 = (i + 1) * RingStep;
		HUD->DrawLine(
			CX + FMath::Cos(A0) * WheelRadius, CY + FMath::Sin(A0) * WheelRadius,
			CX + FMath::Cos(A1) * WheelRadius, CY + FMath::Sin(A1) * WheelRadius,
			FLinearColor(0.4f, 0.5f, 0.6f, 0.7f), 1.5f);
	}

	// Draw labels in each sector
	for (int32 i = 0; i < NumOptions; ++i)
	{
		float MidAngle = i * SectorAngle - PI * 0.5f + SectorAngle * 0.5f;
		float LabelDist = WheelRadius * 0.65f;
		float LX = CX + FMath::Cos(MidAngle) * LabelDist;
		float LY = CY + FMath::Sin(MidAngle) * LabelDist;

		bool bHighlighted = (i == HighlightedOption);
		FLinearColor TextColor = bHighlighted ? CommColors[i] : FLinearColor(0.7f, 0.7f, 0.75f, 0.9f);
		float Scale = bHighlighted ? 0.85f : 0.75f;

		FString Label = CommTexts[i];
		float TW, TH;
		HUD->GetTextSize(Label, TW, TH, Font, Scale);
		HUD->DrawText(Label, TextColor, LX - TW * 0.5f, LY - TH * 0.5f, Font, Scale);
	}

	// Center dot
	HUD->DrawRect(FLinearColor(0.6f, 0.7f, 0.8f, 0.8f), CX - 3.f, CY - 3.f, 6.f, 6.f);
}

void FExoCommsWheel::SendComm(int32 Index, UWorld* World)
{
	if (Index < 0 || Index >= NumOptions || !World) return;

	// Get the local player's pawn location for the ping
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	FVector PawnLoc = PC->GetPawn()->GetActorLocation();
	FExoPingSystem::AddPing(PawnLoc, GetCommText(Index), CommColors[Index]);
}

FString FExoCommsWheel::GetCommText(int32 Index)
{
	if (Index >= 0 && Index < NumOptions)
	{
		return CommTexts[Index];
	}
	return FString();
}
