#include "UI/ExoHitDirectionIndicator.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

TArray<FExoHitDirectionIndicator::FHitIndicator> FExoHitDirectionIndicator::ActiveIndicators;

void FExoHitDirectionIndicator::AddHit(const FVector& SourceLocation)
{
	FHitIndicator Ind;
	Ind.SourceWorldLocation = SourceLocation;
	Ind.Age = 0.f;

	if (ActiveIndicators.Num() >= MaxIndicators)
	{
		ActiveIndicators.RemoveAt(0);
	}
	ActiveIndicators.Add(Ind);
}

void FExoHitDirectionIndicator::Tick(float DeltaTime)
{
	for (int32 i = ActiveIndicators.Num() - 1; i >= 0; i--)
	{
		ActiveIndicators[i].Age += DeltaTime;
		if (ActiveIndicators[i].Age >= ActiveIndicators[i].Lifetime)
		{
			ActiveIndicators.RemoveAt(i);
		}
	}
}

void FExoHitDirectionIndicator::Draw(AHUD* HUD, UCanvas* Canvas)
{
	if (!HUD || !Canvas || ActiveIndicators.Num() == 0) return;

	APlayerController* PC = HUD->GetOwningPlayerController();
	if (!PC || !PC->GetPawn()) return;

	FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
	FRotator PlayerRot = PC->GetControlRotation();

	float CX = Canvas->SizeX * 0.5f;
	float CY = Canvas->SizeY * 0.5f;
	float IndicatorDist = FMath::Min(Canvas->SizeX, Canvas->SizeY) * 0.2f;
	float ArrowLength = 30.f;
	float ArrowWidth = 12.f;

	for (const FHitIndicator& Ind : ActiveIndicators)
	{
		float Alpha = 1.f - (Ind.Age / Ind.Lifetime);
		Alpha = Alpha * Alpha; // Fast fade

		// Get direction from player to damage source
		FVector ToSource = Ind.SourceWorldLocation - PlayerLoc;
		ToSource.Z = 0.f; // Flatten to 2D
		ToSource.Normalize();

		// Convert to screen-relative angle
		FVector Forward = PlayerRot.Vector();
		Forward.Z = 0.f;
		Forward.Normalize();

		FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);

		float DotForward = FVector::DotProduct(ToSource, Forward);
		float DotRight = FVector::DotProduct(ToSource, Right);
		float Angle = FMath::Atan2(DotRight, DotForward);

		// Position on circle around crosshair
		float DirX = FMath::Sin(Angle);
		float DirY = -FMath::Cos(Angle);

		float IndicatorX = CX + DirX * IndicatorDist;
		float IndicatorY = CY + DirY * IndicatorDist;

		// Draw arrow pointing inward (toward the center, indicating source direction)
		FLinearColor ArrowColor(1.f, 0.15f, 0.05f, Alpha * 0.9f);

		// Arrow tip points inward
		FVector2D Tip(IndicatorX - DirX * ArrowLength, IndicatorY - DirY * ArrowLength);
		FVector2D Base(IndicatorX + DirX * ArrowLength * 0.3f, IndicatorY + DirY * ArrowLength * 0.3f);

		// Perpendicular for arrow width
		FVector2D Perp(-DirY, DirX);

		FVector2D Left = Base + Perp * ArrowWidth;
		FVector2D Right2D = Base - Perp * ArrowWidth;

		// Draw three lines to form arrow
		HUD->DrawLine(Tip.X, Tip.Y, Left.X, Left.Y, ArrowColor, 2.5f);
		HUD->DrawLine(Tip.X, Tip.Y, Right2D.X, Right2D.Y, ArrowColor, 2.5f);
		HUD->DrawLine(Left.X, Left.Y, Right2D.X, Right2D.Y, ArrowColor, 1.5f);

		// Fill center with a red rect for visibility
		float FillX = (Tip.X + Base.X) * 0.5f - 3.f;
		float FillY = (Tip.Y + Base.Y) * 0.5f - 3.f;
		HUD->DrawRect(FLinearColor(1.f, 0.1f, 0.05f, Alpha * 0.4f),
			FillX, FillY, 6.f, 6.f);
	}
}
