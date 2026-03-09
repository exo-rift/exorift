#include "UI/ExoPingSystem.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

TArray<FPingEntry> FExoPingSystem::ActivePings;

void FExoPingSystem::AddPing(const FVector& Location, const FString& Label, FLinearColor Color)
{
	FPingEntry Entry;
	Entry.WorldLocation = Location;
	Entry.SpawnTime = 0.f;
	Entry.Lifetime = 5.f;
	Entry.Color = Color;
	Entry.Label = Label;
	ActivePings.Add(Entry);
}

void FExoPingSystem::Tick(float DeltaTime)
{
	for (int32 i = ActivePings.Num() - 1; i >= 0; i--)
	{
		ActivePings[i].Lifetime -= DeltaTime;
		if (ActivePings[i].Lifetime <= 0.f)
		{
			ActivePings.RemoveAt(i);
		}
	}
}

void FExoPingSystem::DrawPings(AHUD* HUD, UCanvas* Canvas, UFont* Font,
	const FVector& ViewerLocation)
{
	if (!HUD || !Canvas || !Font) return;

	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;

	for (const FPingEntry& Ping : ActivePings)
	{
		// Compute fade alpha for last 1.5s
		float Alpha = 1.f;
		if (Ping.Lifetime < FadeOutDuration)
		{
			Alpha = FMath::Clamp(Ping.Lifetime / FadeOutDuration, 0.f, 1.f);
		}

		FLinearColor PingColor = Ping.Color;
		PingColor.A *= Alpha;

		// Distance from viewer
		float Distance = FVector::Dist(ViewerLocation, Ping.WorldLocation);
		float DistMeters = Distance / 100.f; // UU to meters

		// Try to project world location to screen
		FVector2D ScreenPos;
		bool bOnScreen = ProjectToScreen(HUD, Ping.WorldLocation, ScreenPos);

		// Check if within screen bounds (with margin)
		bool bInBounds = bOnScreen
			&& ScreenPos.X >= EdgeMargin && ScreenPos.X <= ScreenW - EdgeMargin
			&& ScreenPos.Y >= EdgeMargin && ScreenPos.Y <= ScreenH - EdgeMargin;

		if (bInBounds)
		{
			// On screen: draw diamond + label + distance
			DrawDiamond(HUD, ScreenPos, 8.f, PingColor);

			// Label above diamond
			FString PingText = FString::Printf(TEXT("%s [%dm]"), *Ping.Label, FMath::RoundToInt(DistMeters));
			float TextW, TextH;
			HUD->GetTextSize(PingText, TextW, TextH, Font, 0.7f);

			float TextX = ScreenPos.X - TextW * 0.5f;
			float TextY = ScreenPos.Y - 22.f;

			// Background
			HUD->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f * Alpha),
				TextX - 4.f, TextY - 2.f, TextW + 8.f, TextH + 4.f);
			HUD->DrawText(PingText, PingColor, TextX, TextY, Font, 0.7f);
		}
		else
		{
			// Off screen: clamp to edge and draw arrow
			FVector2D Center(ScreenW * 0.5f, ScreenH * 0.5f);

			// If behind camera, flip the direction
			FVector2D Dir;
			if (!bOnScreen)
			{
				Dir = Center - ScreenPos;
			}
			else
			{
				Dir = ScreenPos - Center;
			}

			if (Dir.IsNearlyZero())
			{
				Dir = FVector2D(0.f, -1.f);
			}
			Dir.Normalize();

			FVector2D EdgePos = ClampToScreenEdge(Center + Dir * 1000.f, ScreenW, ScreenH, EdgeMargin);

			DrawDiamond(HUD, EdgePos, 6.f, PingColor);
			DrawArrow(HUD, EdgePos, Dir, 12.f, PingColor);

			// Distance label near edge indicator
			FString DistText = FString::Printf(TEXT("%dm"), FMath::RoundToInt(DistMeters));
			float TW, TH;
			HUD->GetTextSize(DistText, TW, TH, Font, 0.6f);
			HUD->DrawText(DistText, PingColor,
				EdgePos.X - TW * 0.5f, EdgePos.Y + 10.f, Font, 0.6f);
		}
	}
}

bool FExoPingSystem::ProjectToScreen(AHUD* HUD, const FVector& WorldPos, FVector2D& OutScreen)
{
	APlayerController* PC = HUD->GetOwningPlayerController();
	if (!PC) return false;

	return PC->ProjectWorldLocationToScreen(WorldPos, OutScreen, true);
}

FVector2D FExoPingSystem::ClampToScreenEdge(const FVector2D& ScreenPos,
	float ScreenW, float ScreenH, float Margin)
{
	FVector2D Center(ScreenW * 0.5f, ScreenH * 0.5f);
	FVector2D Dir = ScreenPos - Center;

	if (Dir.IsNearlyZero()) return Center;

	// Scale direction to hit screen edge (with margin)
	float HalfW = ScreenW * 0.5f - Margin;
	float HalfH = ScreenH * 0.5f - Margin;

	float ScaleX = (FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER) ? HalfW / FMath::Abs(Dir.X) : BIG_NUMBER;
	float ScaleY = (FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER) ? HalfH / FMath::Abs(Dir.Y) : BIG_NUMBER;
	float Scale = FMath::Min(ScaleX, ScaleY);

	return Center + Dir * FMath::Min(Scale, 1.f);
}

void FExoPingSystem::DrawDiamond(AHUD* HUD, const FVector2D& Pos, float Size, const FLinearColor& Color)
{
	// Diamond: four lines forming a rhombus
	HUD->DrawLine(Pos.X, Pos.Y - Size, Pos.X + Size, Pos.Y, Color, 2.f);
	HUD->DrawLine(Pos.X + Size, Pos.Y, Pos.X, Pos.Y + Size, Color, 2.f);
	HUD->DrawLine(Pos.X, Pos.Y + Size, Pos.X - Size, Pos.Y, Color, 2.f);
	HUD->DrawLine(Pos.X - Size, Pos.Y, Pos.X, Pos.Y - Size, Color, 2.f);
}

void FExoPingSystem::DrawArrow(AHUD* HUD, const FVector2D& Pos,
	const FVector2D& Direction, float Size, const FLinearColor& Color)
{
	FVector2D Tip = Pos + Direction * Size;
	FVector2D Perp(-Direction.Y, Direction.X);

	FVector2D Left = Pos - Direction * Size * 0.3f + Perp * Size * 0.4f;
	FVector2D Right = Pos - Direction * Size * 0.3f - Perp * Size * 0.4f;

	HUD->DrawLine(Tip.X, Tip.Y, Left.X, Left.Y, Color, 2.f);
	HUD->DrawLine(Tip.X, Tip.Y, Right.X, Right.Y, Color, 2.f);
}
