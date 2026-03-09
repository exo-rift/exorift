// ExoHUDWorld.cpp — World markers: weather, abilities, supply drops
#include "UI/ExoHUD.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoAbilityComponent.h"
#include "Core/ExoGameState.h"
#include "Map/ExoSupplyDropManager.h"
#include "Map/ExoSupplyDrop.h"
#include "Visual/ExoWeatherSystem.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"

void AExoHUD::DrawWeatherIndicator()
{
	AExoWeatherSystem* Weather = AExoWeatherSystem::Get(GetWorld());
	if (!Weather) return;

	// Position: top-left, below kill streak area (Y ~292)
	float X = 30.f;
	float Y = 294.f;

	FString Label;
	FLinearColor Col = ColorWhite;
	switch (Weather->GetCurrentWeather())
	{
	case EExoWeatherState::Clear:    Label = TEXT("CLEAR");    Col = FLinearColor(0.7f, 0.9f, 1.f, 0.8f); break;
	case EExoWeatherState::Overcast: Label = TEXT("OVERCAST"); Col = FLinearColor(0.6f, 0.6f, 0.65f, 0.8f); break;
	case EExoWeatherState::Rain:     Label = TEXT("RAIN");     Col = FLinearColor(0.3f, 0.5f, 0.9f, 0.9f); break;
	case EExoWeatherState::Storm:    Label = TEXT("STORM");    Col = FLinearColor(1.f, 0.4f, 0.2f, 0.9f); break;
	case EExoWeatherState::Fog:      Label = TEXT("FOG");      Col = FLinearColor(0.7f, 0.7f, 0.75f, 0.9f); break;
	}

	float TW, TH;
	GetTextSize(Label, TW, TH, HUDFont, 0.85f);
	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, TW + 10.f, TH + 6.f);
	DrawText(Label, Col, X, Y, HUDFont, 0.85f);
}

void AExoHUD::DrawAbilities()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetAbilityComponent()) return;
	const TArray<FExoAbility>& Abilities = Char->GetAbilityComponent()->GetAbilities();
	if (Abilities.Num() == 0) return;

	const float SlotW = 60.f, SlotH = 50.f, SlotGap = 8.f;
	float TotalW = Abilities.Num() * SlotW + (Abilities.Num() - 1) * SlotGap;
	float StartX = (Canvas->SizeX - TotalW) * 0.5f;
	float Y = Canvas->SizeY - 95.f;

	const TCHAR* Icons[] = { TEXT("D"), TEXT("S"), TEXT("B") };
	const FLinearColor ReadyColors[] = {
		FLinearColor(0.f, 0.9f, 1.f, 0.9f),  // Cyan - Dash
		FLinearColor(1.f, 0.9f, 0.1f, 0.9f),  // Yellow - Scan
		FLinearColor(0.2f, 0.5f, 1.f, 0.9f)   // Blue - Shield
	};
	const FLinearColor Grey(0.3f, 0.3f, 0.35f, 0.7f);

	for (int32 i = 0; i < FMath::Min(Abilities.Num(), 3); ++i)
	{
		const FExoAbility& Ab = Abilities[i];
		float X = StartX + i * (SlotW + SlotGap);
		bool bReady = Ab.bIsReady();
		FLinearColor SlotCol = bReady ? ReadyColors[i] : Grey;
		DrawRect(ColorBgDark, X, Y, SlotW, SlotH);
		DrawLine(X, Y, X + SlotW, Y, SlotCol); DrawLine(X, Y + SlotH, X + SlotW, Y + SlotH, SlotCol);
		DrawLine(X, Y, X, Y + SlotH, SlotCol); DrawLine(X + SlotW, Y, X + SlotW, Y + SlotH, SlotCol);
		DrawText(Icons[i], SlotCol, X + 22.f, Y + 4.f, HUDFont, 1.2f);
		if (!bReady)
		{
			float Pct = Ab.CooldownRemaining / Ab.Cooldown;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), X + 1.f, Y + 1.f, (SlotW - 2.f) * Pct, SlotH - 2.f);
			FString CdText = FString::Printf(TEXT("%.0f"), Ab.CooldownRemaining);
			DrawText(CdText, ColorWhite, X + 18.f, Y + 30.f, HUDFont, 0.7f);
		}
		else
		{
			float Glow = 0.3f + 0.15f * FMath::Abs(FMath::Sin(GetWorld()->GetTimeSeconds() * 2.f));
			FLinearColor GlowCol = SlotCol; GlowCol.A = Glow;
			DrawRect(GlowCol, X + 1.f, Y + 1.f, SlotW - 2.f, SlotH - 2.f);
		}
	}
}

void AExoHUD::DrawSupplyDropMarkers()
{
	AExoSupplyDropManager* Manager = nullptr;
	for (TActorIterator<AExoSupplyDropManager> It(GetWorld()); It; ++It)
	{
		Manager = *It;
		break;
	}
	if (!Manager) return;

	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	FVector ViewLoc = Char ? Char->GetActorLocation() : FVector::ZeroVector;
	const float ScreenW = Canvas->SizeX;
	const float ScreenH = Canvas->SizeY;
	const FLinearColor DropColor(1.f, 0.85f, 0.15f, 0.9f); // Yellow
	constexpr float EdgeMargin = 40.f;

	for (AExoSupplyDrop* Drop : Manager->GetActiveDrops())
	{
		if (!Drop) continue;
		ESupplyDropState State = Drop->GetState();
		if (State == ESupplyDropState::Depleted) continue;

		FVector DropLoc = Drop->GetActorLocation();
		float DistMeters = FVector::Dist(ViewLoc, DropLoc) / 100.f;

		FVector2D ScreenPos;
		bool bOnScreen = GetOwningPlayerController()->ProjectWorldLocationToScreen(DropLoc, ScreenPos, true);
		bool bInBounds = bOnScreen
			&& ScreenPos.X >= EdgeMargin && ScreenPos.X <= ScreenW - EdgeMargin
			&& ScreenPos.Y >= EdgeMargin && ScreenPos.Y <= ScreenH - EdgeMargin;

		if (bInBounds)
		{
			// Draw yellow diamond marker
			float Sz = 8.f;
			DrawLine(ScreenPos.X, ScreenPos.Y - Sz, ScreenPos.X + Sz, ScreenPos.Y, DropColor, 2.f);
			DrawLine(ScreenPos.X + Sz, ScreenPos.Y, ScreenPos.X, ScreenPos.Y + Sz, DropColor, 2.f);
			DrawLine(ScreenPos.X, ScreenPos.Y + Sz, ScreenPos.X - Sz, ScreenPos.Y, DropColor, 2.f);
			DrawLine(ScreenPos.X - Sz, ScreenPos.Y, ScreenPos.X, ScreenPos.Y - Sz, DropColor, 2.f);

			FString Label = (State == ESupplyDropState::Falling)
				? FString::Printf(TEXT("DROP [%dm]"), FMath::RoundToInt(DistMeters))
				: FString::Printf(TEXT("SUPPLY [%dm]"), FMath::RoundToInt(DistMeters));
			float TW, TH;
			GetTextSize(Label, TW, TH, HUDFont, 0.7f);
			float TX = ScreenPos.X - TW * 0.5f;
			float TY = ScreenPos.Y - 22.f;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f), TX - 4.f, TY - 2.f, TW + 8.f, TH + 4.f);
			DrawText(Label, DropColor, TX, TY, HUDFont, 0.7f);
		}
		else
		{
			// Off-screen edge indicator
			FVector2D Center(ScreenW * 0.5f, ScreenH * 0.5f);
			FVector2D Dir = bOnScreen ? (ScreenPos - Center) : (Center - ScreenPos);
			if (Dir.IsNearlyZero()) Dir = FVector2D(0.f, -1.f);
			Dir.Normalize();

			float HalfW = ScreenW * 0.5f - EdgeMargin;
			float HalfH = ScreenH * 0.5f - EdgeMargin;
			float SX = (FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER) ? HalfW / FMath::Abs(Dir.X) : BIG_NUMBER;
			float SY = (FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER) ? HalfH / FMath::Abs(Dir.Y) : BIG_NUMBER;
			FVector2D EdgePos = Center + Dir * FMath::Min(FMath::Min(SX, SY), 1000.f);

			float Sz = 6.f;
			DrawLine(EdgePos.X, EdgePos.Y - Sz, EdgePos.X + Sz, EdgePos.Y, DropColor, 2.f);
			DrawLine(EdgePos.X + Sz, EdgePos.Y, EdgePos.X, EdgePos.Y + Sz, DropColor, 2.f);
			DrawLine(EdgePos.X, EdgePos.Y + Sz, EdgePos.X - Sz, EdgePos.Y, DropColor, 2.f);
			DrawLine(EdgePos.X - Sz, EdgePos.Y, EdgePos.X, EdgePos.Y - Sz, DropColor, 2.f);

			FString DT = FString::Printf(TEXT("%dm"), FMath::RoundToInt(DistMeters));
			float DW, DH;
			GetTextSize(DT, DW, DH, HUDFont, 0.6f);
			DrawText(DT, DropColor, EdgePos.X - DW * 0.5f, EdgePos.Y + 10.f, HUDFont, 0.6f);
		}
	}
}

void AExoHUD::DrawSupplyDropAnnouncement()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS || GS->AnnouncementTimer <= 0.f) return;

	GS->AnnouncementTimer -= GetWorld()->GetDeltaSeconds();

	float Alpha = FMath::Clamp(GS->AnnouncementTimer / 1.5f, 0.f, 1.f);
	FLinearColor AnnColor(1.f, 0.85f, 0.15f, Alpha);

	FString AnnText = GS->SupplyDropAnnouncement;
	float TW, TH;
	GetTextSize(AnnText, TW, TH, HUDFont, 1.3f);
	float X = (Canvas->SizeX - TW) * 0.5f;
	float Y = Canvas->SizeY * 0.2f;

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.4f * Alpha), X - 15.f, Y - 5.f, TW + 30.f, TH + 10.f);
	DrawText(AnnText, AnnColor, X, Y, HUDFont, 1.3f);

	if (GS->AnnouncementTimer <= 0.f)
	{
		GS->SupplyDropAnnouncement.Empty();
	}
}
