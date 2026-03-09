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

		// Scale-up on fresh hit then settle
		float HitScale = 1.f;
		if (Ind.Age < 0.15f)
		{
			HitScale = 1.f + (1.f - Ind.Age / 0.15f) * 0.3f;
		}

		// Get direction from player to damage source
		FVector ToSource = Ind.SourceWorldLocation - PlayerLoc;
		ToSource.Z = 0.f;
		ToSource.Normalize();

		FVector Forward = PlayerRot.Vector();
		Forward.Z = 0.f;
		Forward.Normalize();

		FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);

		float DotForward = FVector::DotProduct(ToSource, Forward);
		float DotRight = FVector::DotProduct(ToSource, Right);
		float Angle = FMath::Atan2(DotRight, DotForward);

		float DirX = FMath::Sin(Angle);
		float DirY = -FMath::Cos(Angle);

		float ScaledDist = IndicatorDist * HitScale;
		float ScaledLen = ArrowLength * HitScale;
		float ScaledWidth = ArrowWidth * HitScale;

		float IX = CX + DirX * ScaledDist;
		float IY = CY + DirY * ScaledDist;

		FVector2D Tip(IX - DirX * ScaledLen, IY - DirY * ScaledLen);
		FVector2D Base(IX + DirX * ScaledLen * 0.3f, IY + DirY * ScaledLen * 0.3f);
		FVector2D Perp(-DirY, DirX);
		FVector2D Left = Base + Perp * ScaledWidth;
		FVector2D Right2D = Base - Perp * ScaledWidth;

		// Outer glow (wide, dim)
		FLinearColor GlowCol(1.f, 0.15f, 0.08f, Alpha * 0.2f);
		HUD->DrawLine(Tip.X, Tip.Y, Left.X, Left.Y, GlowCol, 6.f);
		HUD->DrawLine(Tip.X, Tip.Y, Right2D.X, Right2D.Y, GlowCol, 6.f);
		HUD->DrawLine(Left.X, Left.Y, Right2D.X, Right2D.Y, GlowCol, 4.f);

		// Main arrow outline
		FLinearColor ArrowColor(1.f, 0.15f, 0.05f, Alpha * 0.9f);
		HUD->DrawLine(Tip.X, Tip.Y, Left.X, Left.Y, ArrowColor, 2.5f);
		HUD->DrawLine(Tip.X, Tip.Y, Right2D.X, Right2D.Y, ArrowColor, 2.5f);
		HUD->DrawLine(Left.X, Left.Y, Right2D.X, Right2D.Y, ArrowColor, 1.5f);

		// Inner bright core line (tip to base center)
		FVector2D MidBase = (Left + Right2D) * 0.5f;
		FLinearColor CoreCol(1.f, 0.35f, 0.15f, Alpha * 0.55f);
		HUD->DrawLine(Tip.X, Tip.Y, MidBase.X, MidBase.Y, CoreCol, 1.5f);

		// Center glow rect
		float FillX = (Tip.X + MidBase.X) * 0.5f - 4.f;
		float FillY = (Tip.Y + MidBase.Y) * 0.5f - 4.f;
		HUD->DrawRect(FLinearColor(1.f, 0.1f, 0.05f, Alpha * 0.3f),
			FillX, FillY, 8.f, 8.f);
	}
}
