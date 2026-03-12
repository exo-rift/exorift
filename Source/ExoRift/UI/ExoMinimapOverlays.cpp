// Copyright Spot Cloud b.v. (2026). All Rights Reserved.

#include "UI/ExoMinimap.h"
#include "UI/ExoLocationNames.h"
#include "GameFramework/HUD.h"
#include "Engine/Font.h"
#include "Player/ExoCharacter.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoSupplyDrop.h"
#include "EngineUtils.h"

void FExoMinimap::DrawZoneCircle(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config,
	const FVector& CenterPos, float PlayerYaw, AExoZoneSystem* Zone)
{
	int32 Segments = 48;
	float AngleStep = 2.f * PI / Segments;
	float Time = HUD->GetWorld()->GetTimeSeconds();
	float ZoneScale = Config.Size / (Config.WorldRange * 2.f);

	float CenterX = Config.ScreenX + Config.Size * 0.5f;
	float CenterY = Config.ScreenY + Config.Size * 0.5f;

	// --- Outside-zone red tint overlay ---
	bool bOutside = !Zone->IsInsideZone(FVector(CenterPos.X, CenterPos.Y, 0.f));
	if (bOutside)
	{
		float PulseAlpha = 0.06f + 0.04f * FMath::Sin(Time * 3.f);
		HUD->DrawRect(FLinearColor(0.8f, 0.05f, 0.05f, PulseAlpha),
			Config.ScreenX, Config.ScreenY, Config.Size, Config.Size);
	}

	// --- Target zone (drawn first, underneath current zone) ---
	FVector TargetWorld(Zone->GetTargetCenter().X, Zone->GetTargetCenter().Y, 0.f);
	FVector2D TargetMini = WorldToMinimap(TargetWorld, CenterPos, PlayerYaw, Config);
	float TargetR = Zone->GetTargetRadius() * ZoneScale;

	bool bShowTarget = Zone->IsShrinking() ||
		(Zone->GetTargetRadius() < Zone->GetCurrentRadius() - 10.f);

	if (bShowTarget)
	{
		// Dashed circle — alternating bright/dim segments
		for (int32 i = 0; i < Segments; i++)
		{
			float A1 = i * AngleStep;
			float A2 = (i + 1) * AngleStep;
			FLinearColor TargetColor = (i % 2 == 0)
				? FLinearColor(1.f, 1.f, 1.f, 0.35f)
				: FLinearColor(1.f, 1.f, 1.f, 0.1f);
			HUD->DrawLine(
				TargetMini.X + FMath::Cos(A1) * TargetR,
				TargetMini.Y + FMath::Sin(A1) * TargetR,
				TargetMini.X + FMath::Cos(A2) * TargetR,
				TargetMini.Y + FMath::Sin(A2) * TargetR,
				TargetColor, 1.f);
		}
	}

	// --- Current zone boundary ---
	FVector ZoneWorld(Zone->GetCurrentCenter().X, Zone->GetCurrentCenter().Y, 0.f);
	FVector2D ZoneMini = WorldToMinimap(ZoneWorld, CenterPos, PlayerYaw, Config);
	float ZoneR = Zone->GetCurrentRadius() * ZoneScale;

	// Pulse brightness when shrinking
	float ZoneAlpha = 0.7f;
	float ZoneThickness = 1.5f;
	if (Zone->IsShrinking())
	{
		ZoneAlpha = 0.5f + 0.35f * FMath::Abs(FMath::Sin(Time * 2.5f));
		ZoneThickness = 2.f;
	}

	FLinearColor ZoneColor(0.15f, 0.7f, 1.f, ZoneAlpha);
	for (int32 i = 0; i < Segments; i++)
	{
		float A1 = i * AngleStep;
		float A2 = (i + 1) * AngleStep;
		HUD->DrawLine(
			ZoneMini.X + FMath::Cos(A1) * ZoneR,
			ZoneMini.Y + FMath::Sin(A1) * ZoneR,
			ZoneMini.X + FMath::Cos(A2) * ZoneR,
			ZoneMini.Y + FMath::Sin(A2) * ZoneR,
			ZoneColor, ZoneThickness);
	}

	// --- Outer glow on the current zone circle (subtle cyan halo) ---
	float GlowR = ZoneR + 2.f;
	FLinearColor GlowColor(0.1f, 0.5f, 0.9f, 0.12f);
	for (int32 i = 0; i < Segments; i++)
	{
		float A1 = i * AngleStep;
		float A2 = (i + 1) * AngleStep;
		HUD->DrawLine(
			ZoneMini.X + FMath::Cos(A1) * GlowR,
			ZoneMini.Y + FMath::Sin(A1) * GlowR,
			ZoneMini.X + FMath::Cos(A2) * GlowR,
			ZoneMini.Y + FMath::Sin(A2) * GlowR,
			GlowColor, 3.f);
	}

	// --- Direction arrow to zone center when player is outside ---
	if (bOutside)
	{
		// Arrow from player (minimap center) toward zone center
		FVector2D ToZone = ZoneMini - FVector2D(CenterX, CenterY);
		float ArrowDist = ToZone.Size();
		if (ArrowDist > 5.f)
		{
			FVector2D Dir = ToZone / ArrowDist;
			// Place arrow near minimap edge, clamped inside
			float MaxDist = Config.Size * 0.4f;
			float ClampedDist = FMath::Min(ArrowDist, MaxDist);
			FVector2D ArrowTip = FVector2D(CenterX, CenterY) + Dir * ClampedDist;

			float Pulse = 0.6f + 0.4f * FMath::Sin(Time * 4.f);
			FLinearColor ArrowCol(1.f, 0.3f, 0.2f, 0.8f * Pulse);

			float ArrowLen = 8.f;
			float ArrowWidth = 4.f;
			FVector2D Back = ArrowTip - Dir * ArrowLen;
			FVector2D Perp(-Dir.Y, Dir.X);

			HUD->DrawLine(ArrowTip.X, ArrowTip.Y,
				Back.X + Perp.X * ArrowWidth, Back.Y + Perp.Y * ArrowWidth,
				ArrowCol, 2.f);
			HUD->DrawLine(ArrowTip.X, ArrowTip.Y,
				Back.X - Perp.X * ArrowWidth, Back.Y - Perp.Y * ArrowWidth,
				ArrowCol, 2.f);
			HUD->DrawLine(Back.X + Perp.X * ArrowWidth, Back.Y + Perp.Y * ArrowWidth,
				Back.X - Perp.X * ArrowWidth, Back.Y - Perp.Y * ArrowWidth,
				ArrowCol, 1.5f);
		}
	}
}

void FExoMinimap::DrawPOILabels(AHUD* HUD, UFont* Font, const FMinimapConfig& Config,
	const FVector& CenterPos, float PlayerYaw)
{
	if (!Font || !HUD) return;

	// Major compound POIs — show abbreviated names on minimap
	struct FPOI { FString Label; FVector WorldPos; };
	static const FPOI POIs[] = {
		{TEXT("CMD"), FVector(0.f, 0.f, 0.f)},
		{TEXT("IND"), FVector(0.f, 16000.f, 0.f)},
		{TEXT("LAB"), FVector(0.f, -16000.f, 0.f)},
		{TEXT("PWR"), FVector(16000.f, 0.f, 0.f)},
		{TEXT("BAR"), FVector(-16000.f, 0.f, 0.f)},
	};

	for (const auto& P : POIs)
	{
		float Dist = FVector::Dist2D(CenterPos, P.WorldPos);
		if (Dist > Config.WorldRange) continue;

		FVector2D Pos = WorldToMinimap(P.WorldPos, CenterPos, PlayerYaw, Config);

		// Check if within minimap bounds
		if (Pos.X < Config.ScreenX + 10.f || Pos.X > Config.ScreenX + Config.Size - 30.f ||
			Pos.Y < Config.ScreenY + 10.f || Pos.Y > Config.ScreenY + Config.Size - 10.f)
			continue;

		FLinearColor LabelCol(0.5f, 0.7f, 0.9f, 0.5f);
		HUD->DrawText(P.Label, LabelCol, Pos.X - 8.f, Pos.Y - 5.f, Font, 0.5f);
	}
}

void FExoMinimap::DrawCurrentLocation(AHUD* HUD, UFont* Font, const FMinimapConfig& Config,
	const FVector& PlayerPos)
{
	if (!Font || !HUD) return;

	FString LocName = FExoLocationNames::GetLocationAt(PlayerPos);
	if (LocName.IsEmpty()) LocName = TEXT("WILDERNESS");

	float TW, TH;
	HUD->GetTextSize(LocName, TW, TH, Font, 0.7f);

	float X = Config.ScreenX + (Config.Size - TW) * 0.5f;
	float Y = Config.ScreenY + Config.Size + 6.f;

	// Background
	HUD->DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.7f),
		X - 6.f, Y - 2.f, TW + 12.f, TH + 4.f);
	// Text
	FLinearColor TextCol = LocName == TEXT("WILDERNESS")
		? FLinearColor(0.5f, 0.5f, 0.55f, 0.7f)
		: FLinearColor(0.6f, 0.8f, 1.f, 0.85f);
	HUD->DrawText(LocName, TextCol, X, Y, Font, 0.7f);
}

void FExoMinimap::DrawTeammates(AHUD* HUD, const FMinimapConfig& Config,
	const FVector& CenterPos, float PlayerYaw, APawn* LocalPawn)
{
	if (!HUD || !LocalPawn) return;

	// TODO: When a team system is added, filter to only show actual teammates.
	// For now (solo BR), draw all other alive characters as grey directional
	// chevrons so the minimap feels populated and gives spatial awareness.

	for (TActorIterator<AExoCharacter> It(HUD->GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive() || Char == LocalPawn) continue;

		float Dist = FVector::Dist(CenterPos, Char->GetActorLocation());
		if (Dist > Config.WorldRange) continue;

		FVector2D Pos = WorldToMinimap(Char->GetActorLocation(), CenterPos, PlayerYaw, Config);
		Pos.X = FMath::Clamp(Pos.X, Config.ScreenX + 4.f, Config.ScreenX + Config.Size - 4.f);
		Pos.Y = FMath::Clamp(Pos.Y, Config.ScreenY + 4.f, Config.ScreenY + Config.Size - 4.f);

		// Rotated directional chevron pointing in their facing direction
		float CharYaw = Char->GetActorRotation().Yaw;
		float RelYaw = FMath::DegreesToRadians(CharYaw - PlayerYaw);

		// Neutral grey for non-team players; swap to blue (0.3, 0.6, 1.0) for teammates
		FLinearColor DotColor(0.65f, 0.7f, 0.75f, 0.7f);
		float CS = Config.PlayerDotSize + 1.f; // Slightly larger than enemy dots

		// Chevron tip points in their facing direction (up on minimap = forward)
		float TipX = Pos.X + FMath::Sin(RelYaw) * CS;
		float TipY = Pos.Y - FMath::Cos(RelYaw) * CS;

		// Two base corners spread behind the tip
		float BaseAngle = PI * 0.75f; // 135 degrees back from tip direction
		float LeftX = Pos.X + FMath::Sin(RelYaw - BaseAngle) * CS * 0.7f;
		float LeftY = Pos.Y - FMath::Cos(RelYaw - BaseAngle) * CS * 0.7f;
		float RightX = Pos.X + FMath::Sin(RelYaw + BaseAngle) * CS * 0.7f;
		float RightY = Pos.Y - FMath::Cos(RelYaw + BaseAngle) * CS * 0.7f;

		// Shadow
		FLinearColor ShadowCol(0.f, 0.f, 0.f, 0.35f);
		HUD->DrawLine(TipX + 1.f, TipY + 1.f, LeftX + 1.f, LeftY + 1.f, ShadowCol, 1.5f);
		HUD->DrawLine(TipX + 1.f, TipY + 1.f, RightX + 1.f, RightY + 1.f, ShadowCol, 1.5f);
		HUD->DrawLine(LeftX + 1.f, LeftY + 1.f, RightX + 1.f, RightY + 1.f, ShadowCol, 1.f);

		// Chevron outline
		HUD->DrawLine(TipX, TipY, LeftX, LeftY, DotColor, 1.5f);
		HUD->DrawLine(TipX, TipY, RightX, RightY, DotColor, 1.5f);
		HUD->DrawLine(LeftX, LeftY, RightX, RightY, DotColor, 1.f);
	}
}

void FExoMinimap::DrawSupplyDrops(AHUD* HUD, const FMinimapConfig& Config,
	const FVector& CenterPos, float PlayerYaw)
{
	float Time = HUD->GetWorld()->GetTimeSeconds();

	for (TActorIterator<AExoSupplyDrop> It(HUD->GetWorld()); It; ++It)
	{
		AExoSupplyDrop* Drop = *It;
		if (!Drop) continue;

		FVector DropPos = Drop->GetActorLocation();
		float Dist = FVector::Dist2D(CenterPos, DropPos);
		if (Dist > Config.WorldRange) continue;

		FVector2D Pos = WorldToMinimap(DropPos, CenterPos, PlayerYaw, Config);
		Pos.X = FMath::Clamp(Pos.X, Config.ScreenX + 4.f, Config.ScreenX + Config.Size - 4.f);
		Pos.Y = FMath::Clamp(Pos.Y, Config.ScreenY + 4.f, Config.ScreenY + Config.Size - 4.f);

		// Pulsing golden diamond
		float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 4.f);
		FLinearColor DropColor(1.f, 0.8f, 0.2f, 0.9f * Pulse);
		float DS = Config.PlayerDotSize * 1.5f;

		// Outer glow
		FLinearColor GlowCol(1.f, 0.7f, 0.1f, 0.2f * Pulse);
		float GS = DS + 3.f;
		HUD->DrawLine(Pos.X, Pos.Y - GS, Pos.X + GS, Pos.Y, GlowCol, 2.f);
		HUD->DrawLine(Pos.X + GS, Pos.Y, Pos.X, Pos.Y + GS, GlowCol, 2.f);
		HUD->DrawLine(Pos.X, Pos.Y + GS, Pos.X - GS, Pos.Y, GlowCol, 2.f);
		HUD->DrawLine(Pos.X - GS, Pos.Y, Pos.X, Pos.Y - GS, GlowCol, 2.f);

		// Solid diamond
		HUD->DrawLine(Pos.X, Pos.Y - DS, Pos.X + DS, Pos.Y, DropColor, 2.f);
		HUD->DrawLine(Pos.X + DS, Pos.Y, Pos.X, Pos.Y + DS, DropColor, 2.f);
		HUD->DrawLine(Pos.X, Pos.Y + DS, Pos.X - DS, Pos.Y, DropColor, 2.f);
		HUD->DrawLine(Pos.X - DS, Pos.Y, Pos.X, Pos.Y - DS, DropColor, 2.f);
	}
}
