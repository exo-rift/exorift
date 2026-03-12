// ExoHUDWorld.cpp — World markers: weather, abilities, supply drops, vehicles
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
	float PanelW = FMath::Max(TW + 20.f, 90.f);
	float PanelH = TH + 18.f;

	DrawRect(ColorBgDark, X - 5.f, Y - 3.f, PanelW, PanelH);
	// Left accent stripe
	DrawRect(FLinearColor(Col.R, Col.G, Col.B, 0.5f), X - 5.f, Y - 3.f, 2.f, PanelH);
	DrawText(Label, Col, X + 2.f, Y, HUDFont, 0.85f);

	// Visibility bar below label
	float VisMul = Weather->GetVisibilityMultiplier();
	float BarX = X - 3.f;
	float BarY = Y + TH + 4.f;
	float BarW = PanelW - 4.f;
	float BarH = 3.f;
	DrawRect(FLinearColor(0.1f, 0.1f, 0.12f, 0.5f), BarX, BarY, BarW, BarH);
	FLinearColor VisCol = FMath::Lerp(FLinearColor(1.f, 0.3f, 0.1f, 0.7f),
		FLinearColor(0.3f, 0.8f, 0.4f, 0.7f), VisMul);
	DrawRect(VisCol, BarX, BarY, BarW * VisMul, BarH);
}

void AExoHUD::DrawRainOverlay()
{
	AExoWeatherSystem* Weather = AExoWeatherSystem::Get(GetWorld());
	if (!Weather) return;

	float RainIntensity = Weather->GetRainIntensity();
	float LightningAlpha = Weather->GetLightningAlpha();

	// Lightning flash (drawn even if not raining, storm may be transitioning)
	if (LightningAlpha > 0.01f)
	{
		DrawRect(FLinearColor(0.85f, 0.9f, 1.f, LightningAlpha * 0.35f),
			0.f, 0.f, Canvas->SizeX, Canvas->SizeY);
	}

	if (RainIntensity <= 0.01f) return;

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	float Time = GetWorld()->GetTimeSeconds();
	float WindStr = Weather->GetWindStrength();

	// Screen-space rain streaks
	int32 NumStreaks = FMath::RoundToInt(RainIntensity * 80.f);
	float WindAngle = WindStr * 0.35f;

	for (int32 i = 0; i < NumStreaks; i++)
	{
		float Seed = i * 137.508f;
		float Speed = 800.f + (i % 5) * 200.f;
		float Y = FMath::Fmod(Seed * 7.31f + Time * Speed, H + 200.f) - 100.f;
		float X = FMath::Fmod(Seed * 11.71f + Time * WindStr * 150.f, W);

		float StreakLen = 25.f + (i % 4) * 15.f;
		float Alpha = RainIntensity * (0.05f + 0.03f * FMath::Sin(Seed * 0.3f));

		float EndX = X + WindAngle * StreakLen;
		float EndY = Y + StreakLen;

		FLinearColor StreakCol(0.55f, 0.65f, 0.82f, Alpha);
		DrawLine(X, Y, EndX, EndY, StreakCol, 1.f);
	}

	// Bottom wetness — subtle blue tint at screen bottom
	float WetAlpha = RainIntensity * 0.06f;
	float WetH = H * 0.05f;
	DrawRect(FLinearColor(0.08f, 0.12f, 0.25f, WetAlpha), 0.f, H - WetH, W, WetH);

	// Screen droplets — small water spots that fade in/out
	int32 NumDroplets = FMath::RoundToInt(RainIntensity * 15.f);
	for (int32 i = 0; i < NumDroplets; i++)
	{
		float Seed = i * 97.f + 3.14f;
		float Lifetime = 0.6f + (i % 3) * 0.4f;
		float Phase = FMath::Fmod(Time + Seed, Lifetime);
		float FadeIn = FMath::Clamp(Phase / 0.1f, 0.f, 1.f);
		float FadeOut = FMath::Clamp(1.f - Phase / Lifetime, 0.f, 1.f);
		float DropAlpha = FadeIn * FadeOut * RainIntensity * 0.12f;

		float DX = FMath::Fmod(Seed * 13.7f, W);
		float DY = FMath::Fmod(Seed * 9.3f, H);
		float DSize = 2.f + (i % 3);

		DrawRect(FLinearColor(0.45f, 0.55f, 0.75f, DropAlpha),
			DX, DY, DSize, DSize * 1.5f);
		// Highlight dot at top of droplet
		DrawRect(FLinearColor(0.7f, 0.8f, 0.95f, DropAlpha * 0.6f),
			DX + DSize * 0.3f, DY, DSize * 0.4f, DSize * 0.4f);
	}
}

void AExoHUD::DrawAbilities()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwningPawn());
	if (!Char || !Char->GetAbilityComponent()) return;
	const TArray<FExoAbility>& Abilities = Char->GetAbilityComponent()->GetAbilities();
	if (Abilities.Num() == 0) return;

	const float Time = GetWorld()->GetTimeSeconds();
	const float SlotW = 70.f, SlotH = 58.f, SlotGap = 10.f;
	const int32 Count = FMath::Min(Abilities.Num(), 5);
	float TotalW = Count * SlotW + (Count - 1) * SlotGap;
	float StartX = (Canvas->SizeX - TotalW) * 0.5f;
	float BaseY = Canvas->SizeY - 100.f;

	const TCHAR* AbilNames[] = {
		TEXT("DASH"), TEXT("SCAN"), TEXT("SHIELD"), TEXT("GRAPPLE"), TEXT("DECOY")
	};
	const TCHAR* Icons[] = { TEXT("D"), TEXT("S"), TEXT("B"), TEXT("G"), TEXT("H") };
	const FLinearColor ReadyColors[] = {
		FLinearColor(0.f, 0.9f, 1.f, 0.9f),   // Cyan - Dash
		FLinearColor(1.f, 0.9f, 0.1f, 0.9f),   // Yellow - Scan
		FLinearColor(0.2f, 0.5f, 1.f, 0.9f),   // Blue - Shield
		FLinearColor(0.2f, 1.f, 0.5f, 0.9f),   // Green - Grapple
		FLinearColor(0.6f, 0.3f, 1.f, 0.9f)    // Purple - Decoy
	};
	const FLinearColor DimGrey(0.25f, 0.25f, 0.3f, 0.6f);

	for (int32 i = 0; i < Count; ++i)
	{
		const FExoAbility& Ab = Abilities[i];
		float X = StartX + i * (SlotW + SlotGap);
		bool bReady = Ab.bIsReady();
		int32 TypeIdx = FMath::Clamp(static_cast<int32>(Ab.Type), 0, 4);
		FLinearColor AccentCol = ReadyColors[TypeIdx];

		// --- Background panel ---
		FLinearColor BgCol = bReady
			? FLinearColor(0.04f, 0.06f, 0.1f, 0.8f)
			: FLinearColor(0.03f, 0.03f, 0.05f, 0.85f);
		DrawRect(BgCol, X, BaseY, SlotW, SlotH);

		if (!bReady)
		{
			// --- Cooldown overlay: fill sweeps left-to-right as CD elapses ---
			float CdPct = FMath::Clamp(Ab.CooldownRemaining / Ab.Cooldown, 0.f, 1.f);
			// Dark overlay covers the remaining-cooldown portion (right side)
			float FillW = (SlotW - 2.f) * CdPct;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f),
				X + 1.f + (SlotW - 2.f - FillW), BaseY + 1.f,
				FillW, SlotH - 2.f);

			// Thin progress bar at the bottom showing elapsed fraction
			float ElapsedPct = 1.f - CdPct;
			float BarH = 3.f;
			FLinearColor BarCol = AccentCol;
			BarCol.A = 0.7f;
			DrawRect(FLinearColor(0.08f, 0.08f, 0.1f, 0.6f),
				X + 1.f, BaseY + SlotH - BarH - 1.f, SlotW - 2.f, BarH);
			if (ElapsedPct > 0.01f)
			{
				DrawRect(BarCol,
					X + 1.f, BaseY + SlotH - BarH - 1.f,
					(SlotW - 2.f) * ElapsedPct, BarH);
			}

			// Dimmed icon letter
			FLinearColor DimIcon = AccentCol;
			DimIcon.A = 0.3f;
			DrawText(Icons[TypeIdx], DimIcon, X + 24.f, BaseY + 4.f, HUDFont, 1.2f);

			// Cooldown remaining: large centered text
			FString CdText = FString::Printf(TEXT("%.1f"), Ab.CooldownRemaining);
			float CdW, CdH;
			GetTextSize(CdText, CdW, CdH, HUDFont, 0.95f);
			DrawText(CdText, FLinearColor(0.f, 0.f, 0.f, 0.5f),
				X + (SlotW - CdW) * 0.5f + 1.f,
				BaseY + (SlotH - CdH) * 0.5f + 1.f, HUDFont, 0.95f);
			DrawText(CdText, ColorWhite,
				X + (SlotW - CdW) * 0.5f,
				BaseY + (SlotH - CdH) * 0.5f, HUDFont, 0.95f);

			// Border: dimmed accent color
			FLinearColor BorderCol = DimGrey;
			DrawLine(X, BaseY, X + SlotW, BaseY, BorderCol);
			DrawLine(X, BaseY + SlotH, X + SlotW, BaseY + SlotH, BorderCol);
			DrawLine(X, BaseY, X, BaseY + SlotH, BorderCol);
			DrawLine(X + SlotW, BaseY, X + SlotW, BaseY + SlotH, BorderCol);
		}
		else
		{
			// --- Ready: bright pulsing glow ---
			float Pulse = 0.15f + 0.12f * FMath::Abs(FMath::Sin(Time * 2.5f + i * 0.8f));
			FLinearColor GlowCol(AccentCol.R, AccentCol.G, AccentCol.B, Pulse);
			DrawRect(GlowCol, X + 1.f, BaseY + 1.f, SlotW - 2.f, SlotH - 2.f);

			// Bright icon letter
			DrawText(Icons[TypeIdx], AccentCol, X + 24.f, BaseY + 4.f, HUDFont, 1.2f);

			// "READY" text below icon
			FString ReadyTxt = TEXT("READY");
			float RW, RH;
			GetTextSize(ReadyTxt, RW, RH, HUDFont, 0.55f);
			FLinearColor ReadyCol(AccentCol.R, AccentCol.G, AccentCol.B, 0.7f);
			DrawText(ReadyTxt, ReadyCol,
				X + (SlotW - RW) * 0.5f,
				BaseY + SlotH - RH - 6.f, HUDFont, 0.55f);

			// Bright border
			DrawLine(X, BaseY, X + SlotW, BaseY, AccentCol);
			DrawLine(X, BaseY + SlotH, X + SlotW, BaseY + SlotH, AccentCol);
			DrawLine(X, BaseY, X, BaseY + SlotH, AccentCol);
			DrawLine(X + SlotW, BaseY, X + SlotW, BaseY + SlotH, AccentCol);

			// Corner bracket accents
			float BL = 8.f;
			FLinearColor CornerCol(AccentCol.R, AccentCol.G, AccentCol.B, 0.5f);
			DrawLine(X, BaseY, X + BL, BaseY, CornerCol);
			DrawLine(X, BaseY, X, BaseY + BL, CornerCol);
			DrawLine(X + SlotW, BaseY + SlotH, X + SlotW - BL, BaseY + SlotH, CornerCol);
			DrawLine(X + SlotW, BaseY + SlotH, X + SlotW, BaseY + SlotH - BL, CornerCol);
		}

		// --- Ability name label below the slot ---
		const TCHAR* Name = AbilNames[TypeIdx];
		float NW, NH;
		GetTextSize(Name, NW, NH, HUDFont, 0.5f);
		FLinearColor NameCol = bReady
			? FLinearColor(AccentCol.R, AccentCol.G, AccentCol.B, 0.8f)
			: FLinearColor(0.45f, 0.45f, 0.5f, 0.6f);
		DrawText(Name, NameCol,
			X + (SlotW - NW) * 0.5f, BaseY + SlotH + 3.f, HUDFont, 0.5f);
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

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	for (AExoSupplyDrop* Drop : Manager->GetActiveDrops())
	{
		if (!Drop) continue;
		ESupplyDropState State = Drop->GetState();
		if (State == ESupplyDropState::Depleted) continue;

		FVector DropLoc = Drop->GetActorLocation();
		float DistMeters = FVector::Dist(ViewLoc, DropLoc) / 100.f;

		FVector2D ScreenPos;
		bool bOnScreen = PC->ProjectWorldLocationToScreen(DropLoc, ScreenPos, true);
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

// DrawSupplyDropAnnouncement, DrawLocationBanner, DrawZiplinePrompt in ExoHUDWorldPrompts.cpp
