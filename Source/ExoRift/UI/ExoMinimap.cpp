// Copyright Spot Cloud b.v. (2026). All Rights Reserved.

#include "UI/ExoMinimap.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Player/ExoCharacter.h"
#include "Map/ExoZoneSystem.h"
#include "EngineUtils.h"

void FExoMinimap::Draw(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config)
{
	if (!HUD || !Canvas) return;

	APawn* PlayerPawn = HUD->GetOwningPawn();
	if (!PlayerPawn) return;

	FVector PlayerPos = PlayerPawn->GetActorLocation();
	float PlayerYaw = PlayerPawn->GetControlRotation().Yaw;
	float CenterX = Config.ScreenX + Config.Size * 0.5f;
	float CenterY = Config.ScreenY + Config.Size * 0.5f;

	// Dark background
	HUD->DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.85f),
		Config.ScreenX, Config.ScreenY, Config.Size, Config.Size);

	// Subtle outer glow
	HUD->DrawRect(FLinearColor(0.f, 0.15f, 0.3f, 0.08f),
		Config.ScreenX - 2.f, Config.ScreenY - 2.f,
		Config.Size + 4.f, Config.Size + 4.f);

	// Grid lines
	DrawGridLines(HUD, CenterX, CenterY, Config);

	// Zone circle
	AExoZoneSystem* Zone = nullptr;
	for (TActorIterator<AExoZoneSystem> It(HUD->GetWorld()); It; ++It)
	{
		Zone = *It;
		break;
	}
	if (Zone)
	{
		DrawZoneCircle(HUD, Canvas, Config, PlayerPos, PlayerYaw, Zone);
	}

	// Radar sweep animation
	DrawRadarSweep(HUD, CenterX, CenterY, Config);

	// FOV cone
	DrawFOVCone(HUD, CenterX, CenterY, Config);

	// Supply drop markers
	DrawSupplyDrops(HUD, Config, PlayerPos, PlayerYaw);

	// Teammate / other player markers (drawn before enemies so red overlays grey)
	DrawTeammates(HUD, Config, PlayerPos, PlayerYaw, PlayerPawn);

	// Enemy dots
	for (TActorIterator<AExoCharacter> It(HUD->GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive() || Char == PlayerPawn) continue;

		float Dist = FVector::Dist(PlayerPos, Char->GetActorLocation());
		if (Dist > Config.WorldRange) continue;

		FVector2D MinimapPos = WorldToMinimap(Char->GetActorLocation(), PlayerPos, PlayerYaw, Config);

		MinimapPos.X = FMath::Clamp(MinimapPos.X, Config.ScreenX + 4.f,
			Config.ScreenX + Config.Size - 4.f);
		MinimapPos.Y = FMath::Clamp(MinimapPos.Y, Config.ScreenY + 4.f,
			Config.ScreenY + Config.Size - 4.f);

		float DS = Config.PlayerDotSize;
		FLinearColor EnemyColor(1.f, 0.15f, 0.15f, 0.9f);
		// Shadow
		FLinearColor ShadowCol(0.f, 0.f, 0.f, 0.4f);
		HUD->DrawRect(ShadowCol, MinimapPos.X - DS - 1.f, MinimapPos.Y - DS - 1.f,
			DS * 2.f + 2.f, DS * 2.f + 2.f);
		// Diamond
		HUD->DrawLine(MinimapPos.X, MinimapPos.Y - DS, MinimapPos.X + DS, MinimapPos.Y, EnemyColor, 1.5f);
		HUD->DrawLine(MinimapPos.X + DS, MinimapPos.Y, MinimapPos.X, MinimapPos.Y + DS, EnemyColor, 1.5f);
		HUD->DrawLine(MinimapPos.X, MinimapPos.Y + DS, MinimapPos.X - DS, MinimapPos.Y, EnemyColor, 1.5f);
		HUD->DrawLine(MinimapPos.X - DS, MinimapPos.Y, MinimapPos.X, MinimapPos.Y - DS, EnemyColor, 1.5f);
	}

	// Player chevron at center
	FLinearColor PlayerColor(0.2f, 0.9f, 1.f, 1.f);
	float CS = 7.f;
	// Shadow behind chevron
	FLinearColor ChevShadow(0.f, 0.f, 0.f, 0.5f);
	HUD->DrawLine(CenterX + 1.f, CenterY - CS + 1.f, CenterX + CS * 0.6f + 1.f, CenterY + CS * 0.4f + 1.f, ChevShadow, 2.f);
	HUD->DrawLine(CenterX + 1.f, CenterY - CS + 1.f, CenterX - CS * 0.6f + 1.f, CenterY + CS * 0.4f + 1.f, ChevShadow, 2.f);
	// Chevron
	HUD->DrawLine(CenterX, CenterY - CS, CenterX + CS * 0.6f, CenterY + CS * 0.4f, PlayerColor, 2.f);
	HUD->DrawLine(CenterX, CenterY - CS, CenterX - CS * 0.6f, CenterY + CS * 0.4f, PlayerColor, 2.f);
	HUD->DrawLine(CenterX - CS * 0.6f, CenterY + CS * 0.4f,
		CenterX + CS * 0.6f, CenterY + CS * 0.4f, PlayerColor, 1.5f);

	// Cardinal direction labels
	UFont* Font = nullptr;
	if (Cast<AExoCharacter>(PlayerPawn))
		Font = Cast<UFont>(StaticLoadObject(UFont::StaticClass(), nullptr,
			TEXT("/Engine/EngineFonts/Roboto")));
	DrawCardinals(HUD, Font, CenterX, CenterY, PlayerYaw, Config);

	// POI labels on minimap
	DrawPOILabels(HUD, Font, Config, PlayerPos, PlayerYaw);

	// Border with corner accents
	FLinearColor BorderColor(0.25f, 0.4f, 0.55f, 0.9f);
	float SX = Config.ScreenX, SY = Config.ScreenY, SS = Config.Size;
	HUD->DrawLine(SX, SY, SX + SS, SY, BorderColor, Config.BorderWidth);
	HUD->DrawLine(SX + SS, SY, SX + SS, SY + SS, BorderColor, Config.BorderWidth);
	HUD->DrawLine(SX + SS, SY + SS, SX, SY + SS, BorderColor, Config.BorderWidth);
	HUD->DrawLine(SX, SY + SS, SX, SY, BorderColor, Config.BorderWidth);

	// Corner bracket accents (brighter)
	float BL = 15.f;
	FLinearColor AccCol(0.f, 0.6f, 1.f, 0.5f);
	HUD->DrawLine(SX, SY, SX + BL, SY, AccCol, 2.f);
	HUD->DrawLine(SX, SY, SX, SY + BL, AccCol, 2.f);
	HUD->DrawLine(SX + SS, SY, SX + SS - BL, SY, AccCol, 2.f);
	HUD->DrawLine(SX + SS, SY, SX + SS, SY + BL, AccCol, 2.f);
	HUD->DrawLine(SX, SY + SS, SX + BL, SY + SS, AccCol, 2.f);
	HUD->DrawLine(SX, SY + SS, SX, SY + SS - BL, AccCol, 2.f);
	HUD->DrawLine(SX + SS, SY + SS, SX + SS - BL, SY + SS, AccCol, 2.f);
	HUD->DrawLine(SX + SS, SY + SS, SX + SS, SY + SS - BL, AccCol, 2.f);

	// Current location name below minimap
	DrawCurrentLocation(HUD, Font, Config, PlayerPos);
}

FVector2D FExoMinimap::WorldToMinimap(const FVector& WorldPos, const FVector& CenterPos,
	float PlayerYaw, const FMinimapConfig& Config)
{
	FVector Offset = WorldPos - CenterPos;
	float Scale = Config.Size / (Config.WorldRange * 2.f);

	float Rad = FMath::DegreesToRadians(-PlayerYaw);
	float RotX = Offset.X * FMath::Cos(Rad) - Offset.Y * FMath::Sin(Rad);
	float RotY = Offset.X * FMath::Sin(Rad) + Offset.Y * FMath::Cos(Rad);

	return FVector2D(
		Config.ScreenX + Config.Size * 0.5f + RotY * Scale,
		Config.ScreenY + Config.Size * 0.5f - RotX * Scale
	);
}

void FExoMinimap::DrawFOVCone(AHUD* HUD, float CenterX, float CenterY,
	const FMinimapConfig& Config)
{
	float ConeLen = Config.Size * 0.35f;
	float HalfAngle = 30.f;
	float LeftRad = FMath::DegreesToRadians(-90.f - HalfAngle);
	float RightRad = FMath::DegreesToRadians(-90.f + HalfAngle);

	FLinearColor ConeColor(0.2f, 0.8f, 1.f, 0.08f);

	int32 Lines = 8;
	for (int32 i = 0; i <= Lines; i++)
	{
		float T = (float)i / Lines;
		float Rad = FMath::Lerp(LeftRad, RightRad, T);
		float EndX = CenterX + FMath::Cos(Rad) * ConeLen;
		float EndY = CenterY + FMath::Sin(Rad) * ConeLen;
		HUD->DrawLine(CenterX, CenterY, EndX, EndY, ConeColor, 1.f);
	}

	FLinearColor EdgeColor(0.2f, 0.8f, 1.f, 0.25f);
	HUD->DrawLine(CenterX, CenterY,
		CenterX + FMath::Cos(LeftRad) * ConeLen,
		CenterY + FMath::Sin(LeftRad) * ConeLen, EdgeColor, 1.5f);
	HUD->DrawLine(CenterX, CenterY,
		CenterX + FMath::Cos(RightRad) * ConeLen,
		CenterY + FMath::Sin(RightRad) * ConeLen, EdgeColor, 1.5f);

	// Arc at the end of the FOV cone
	FLinearColor ArcCol(0.2f, 0.8f, 1.f, 0.15f);
	int32 ArcSegs = 8;
	for (int32 i = 0; i < ArcSegs; i++)
	{
		float T0 = (float)i / ArcSegs;
		float T1 = (float)(i + 1) / ArcSegs;
		float R0 = FMath::Lerp(LeftRad, RightRad, T0);
		float R1 = FMath::Lerp(LeftRad, RightRad, T1);
		HUD->DrawLine(
			CenterX + FMath::Cos(R0) * ConeLen, CenterY + FMath::Sin(R0) * ConeLen,
			CenterX + FMath::Cos(R1) * ConeLen, CenterY + FMath::Sin(R1) * ConeLen,
			ArcCol, 1.f);
	}
}

void FExoMinimap::DrawCardinals(AHUD* HUD, UFont* Font, float CenterX, float CenterY,
	float PlayerYaw, const FMinimapConfig& Config)
{
	if (!Font) return;

	struct FCardinal { const TCHAR* Label; float WorldYaw; FLinearColor Color; };
	FCardinal Cards[] = {
		{TEXT("N"), 0.f,   FLinearColor(1.f, 0.3f, 0.3f, 0.8f)},
		{TEXT("E"), 90.f,  FLinearColor(0.7f, 0.7f, 0.7f, 0.5f)},
		{TEXT("S"), 180.f, FLinearColor(0.7f, 0.7f, 0.7f, 0.5f)},
		{TEXT("W"), 270.f, FLinearColor(0.7f, 0.7f, 0.7f, 0.5f)},
	};

	float LabelDist = Config.Size * 0.5f - 14.f;
	for (const auto& C : Cards)
	{
		float RelAngle = FMath::DegreesToRadians(C.WorldYaw - PlayerYaw - 90.f);
		float LX = CenterX + FMath::Cos(RelAngle) * LabelDist - 4.f;
		float LY = CenterY + FMath::Sin(RelAngle) * LabelDist - 6.f;

		if (LX >= Config.ScreenX && LX <= Config.ScreenX + Config.Size - 8.f &&
			LY >= Config.ScreenY && LY <= Config.ScreenY + Config.Size - 12.f)
		{
			// Shadow for readability
			HUD->DrawText(C.Label, FLinearColor(0.f, 0.f, 0.f, C.Color.A * 0.5f),
				LX + 1.f, LY + 1.f, Font, 0.8f);
			HUD->DrawText(C.Label, C.Color, LX, LY, Font, 0.8f);
		}
	}
}

void FExoMinimap::DrawGridLines(AHUD* HUD, float CenterX, float CenterY,
	const FMinimapConfig& Config)
{
	FLinearColor GridColor(0.15f, 0.2f, 0.25f, 0.15f);
	float HalfSize = Config.Size * 0.5f;

	// Cross through center
	HUD->DrawLine(CenterX, Config.ScreenY, CenterX, Config.ScreenY + Config.Size,
		GridColor, 0.5f);
	HUD->DrawLine(Config.ScreenX, CenterY, Config.ScreenX + Config.Size, CenterY,
		GridColor, 0.5f);

	// Range rings (25%, 50%, 75%)
	FLinearColor RingColor(0.15f, 0.2f, 0.3f, 0.12f);
	int32 Segs = 24;
	float AngleStep = 2.f * PI / Segs;
	for (float Pct : {0.25f, 0.5f, 0.75f})
	{
		float R = HalfSize * Pct * 2.f;
		for (int32 i = 0; i < Segs; i++)
		{
			float A1 = i * AngleStep;
			float A2 = (i + 1) * AngleStep;
			HUD->DrawLine(
				CenterX + FMath::Cos(A1) * R, CenterY + FMath::Sin(A1) * R,
				CenterX + FMath::Cos(A2) * R, CenterY + FMath::Sin(A2) * R,
				RingColor, 0.5f);
		}
	}
}

void FExoMinimap::DrawRadarSweep(AHUD* HUD, float CenterX, float CenterY,
	const FMinimapConfig& Config)
{
	float Time = HUD->GetWorld()->GetTimeSeconds();
	float SweepAngle = FMath::Fmod(Time * 0.8f, 1.f) * 2.f * PI; // Full revolution every ~7.8s
	float HalfSize = Config.Size * 0.5f;

	// Sweep line — bright leading edge
	FLinearColor SweepColor(0.1f, 0.6f, 1.f, 0.35f);
	float SweepEndX = CenterX + FMath::Cos(SweepAngle) * HalfSize;
	float SweepEndY = CenterY + FMath::Sin(SweepAngle) * HalfSize;
	HUD->DrawLine(CenterX, CenterY, SweepEndX, SweepEndY, SweepColor, 1.5f);

	// Trailing fade — 6 lines behind the sweep creating a gradient tail
	for (int32 i = 1; i <= 6; i++)
	{
		float TrailAngle = SweepAngle - (float)i * 0.06f;
		float TrailAlpha = 0.35f * (1.f - (float)i / 7.f);
		FLinearColor TrailCol(0.05f, 0.3f, 0.7f, TrailAlpha);
		float TX = CenterX + FMath::Cos(TrailAngle) * HalfSize;
		float TY = CenterY + FMath::Sin(TrailAngle) * HalfSize;
		HUD->DrawLine(CenterX, CenterY, TX, TY, TrailCol, 1.f);
	}
}
