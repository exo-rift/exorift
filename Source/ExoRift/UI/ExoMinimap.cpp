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
	float HalfSize = Config.Size * 0.5f;

	// Dark background
	HUD->DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.85f),
		Config.ScreenX, Config.ScreenY, Config.Size, Config.Size);

	// Grid lines
	DrawGridLines(HUD, CenterX, CenterY, Config);

	// Zone circle (rotated)
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

	// FOV cone (player's viewing direction)
	DrawFOVCone(HUD, CenterX, CenterY, Config);

	// Enemy dots (rotated to match player heading)
	for (TActorIterator<AExoCharacter> It(HUD->GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive() || Char == PlayerPawn) continue;

		float Dist = FVector::Dist(PlayerPos, Char->GetActorLocation());
		if (Dist > Config.WorldRange) continue;

		FVector2D MinimapPos = WorldToMinimap(Char->GetActorLocation(), PlayerPos, PlayerYaw, Config);

		// Clamp to minimap bounds
		MinimapPos.X = FMath::Clamp(MinimapPos.X, Config.ScreenX + 4.f,
			Config.ScreenX + Config.Size - 4.f);
		MinimapPos.Y = FMath::Clamp(MinimapPos.Y, Config.ScreenY + 4.f,
			Config.ScreenY + Config.Size - 4.f);

		// Red diamond for enemies
		float DS = Config.PlayerDotSize;
		FLinearColor EnemyColor(1.f, 0.15f, 0.15f, 0.9f);
		HUD->DrawLine(MinimapPos.X, MinimapPos.Y - DS, MinimapPos.X + DS, MinimapPos.Y, EnemyColor, 1.5f);
		HUD->DrawLine(MinimapPos.X + DS, MinimapPos.Y, MinimapPos.X, MinimapPos.Y + DS, EnemyColor, 1.5f);
		HUD->DrawLine(MinimapPos.X, MinimapPos.Y + DS, MinimapPos.X - DS, MinimapPos.Y, EnemyColor, 1.5f);
		HUD->DrawLine(MinimapPos.X - DS, MinimapPos.Y, MinimapPos.X, MinimapPos.Y - DS, EnemyColor, 1.5f);
	}

	// Player chevron at center (always pointing up = forward)
	FLinearColor PlayerColor(0.2f, 0.9f, 1.f, 1.f);
	float CS = 7.f; // Chevron size
	HUD->DrawLine(CenterX, CenterY - CS, CenterX + CS * 0.6f, CenterY + CS * 0.4f, PlayerColor, 2.f);
	HUD->DrawLine(CenterX, CenterY - CS, CenterX - CS * 0.6f, CenterY + CS * 0.4f, PlayerColor, 2.f);
	HUD->DrawLine(CenterX - CS * 0.6f, CenterY + CS * 0.4f,
		CenterX + CS * 0.6f, CenterY + CS * 0.4f, PlayerColor, 1.5f);

	// Cardinal direction labels
	UFont* Font = nullptr;
	if (AExoCharacter* HC = Cast<AExoCharacter>(PlayerPawn))
		Font = Cast<UFont>(StaticLoadObject(UFont::StaticClass(), nullptr,
			TEXT("/Engine/EngineFonts/Roboto")));
	DrawCardinals(HUD, Font, CenterX, CenterY, PlayerYaw, Config);

	// Border (bright edge)
	FLinearColor BorderColor(0.25f, 0.4f, 0.55f, 0.9f);
	HUD->DrawLine(Config.ScreenX, Config.ScreenY,
		Config.ScreenX + Config.Size, Config.ScreenY, BorderColor, Config.BorderWidth);
	HUD->DrawLine(Config.ScreenX + Config.Size, Config.ScreenY,
		Config.ScreenX + Config.Size, Config.ScreenY + Config.Size, BorderColor, Config.BorderWidth);
	HUD->DrawLine(Config.ScreenX + Config.Size, Config.ScreenY + Config.Size,
		Config.ScreenX, Config.ScreenY + Config.Size, BorderColor, Config.BorderWidth);
	HUD->DrawLine(Config.ScreenX, Config.ScreenY + Config.Size,
		Config.ScreenX, Config.ScreenY, BorderColor, Config.BorderWidth);
}

FVector2D FExoMinimap::WorldToMinimap(const FVector& WorldPos, const FVector& CenterPos,
	float PlayerYaw, const FMinimapConfig& Config)
{
	FVector Offset = WorldPos - CenterPos;
	float Scale = Config.Size / (Config.WorldRange * 2.f);

	// Rotate offset by -PlayerYaw so minimap rotates with player
	float Rad = FMath::DegreesToRadians(-PlayerYaw);
	float RotX = Offset.X * FMath::Cos(Rad) - Offset.Y * FMath::Sin(Rad);
	float RotY = Offset.X * FMath::Sin(Rad) + Offset.Y * FMath::Cos(Rad);

	return FVector2D(
		Config.ScreenX + Config.Size * 0.5f + RotY * Scale,
		Config.ScreenY + Config.Size * 0.5f - RotX * Scale
	);
}

void FExoMinimap::DrawZoneCircle(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config,
	const FVector& CenterPos, float PlayerYaw, AExoZoneSystem* Zone)
{
	int32 Segments = 36;
	float AngleStep = 2.f * PI / Segments;

	// Current zone
	FVector ZoneWorld(Zone->GetCurrentCenter().X, Zone->GetCurrentCenter().Y, 0.f);
	FVector2D ZoneMini = WorldToMinimap(ZoneWorld, CenterPos, PlayerYaw, Config);
	float ZoneScale = Config.Size / (Config.WorldRange * 2.f);
	float ZoneR = Zone->GetCurrentRadius() * ZoneScale;

	FLinearColor ZoneColor(0.1f, 0.5f, 1.f, 0.6f);
	for (int32 i = 0; i < Segments; i++)
	{
		float A1 = i * AngleStep;
		float A2 = (i + 1) * AngleStep;
		HUD->DrawLine(
			ZoneMini.X + FMath::Cos(A1) * ZoneR, ZoneMini.Y + FMath::Sin(A1) * ZoneR,
			ZoneMini.X + FMath::Cos(A2) * ZoneR, ZoneMini.Y + FMath::Sin(A2) * ZoneR,
			ZoneColor, 1.5f);
	}

	// Target zone (white, dimmer)
	FVector TargetWorld(Zone->GetTargetCenter().X, Zone->GetTargetCenter().Y, 0.f);
	FVector2D TargetMini = WorldToMinimap(TargetWorld, CenterPos, PlayerYaw, Config);
	float TargetR = Zone->GetTargetRadius() * ZoneScale;

	FLinearColor TargetColor(1.f, 1.f, 1.f, 0.25f);
	for (int32 i = 0; i < Segments; i++)
	{
		float A1 = i * AngleStep;
		float A2 = (i + 1) * AngleStep;
		HUD->DrawLine(
			TargetMini.X + FMath::Cos(A1) * TargetR, TargetMini.Y + FMath::Sin(A1) * TargetR,
			TargetMini.X + FMath::Cos(A2) * TargetR, TargetMini.Y + FMath::Sin(A2) * TargetR,
			TargetColor, 1.f);
	}
}

void FExoMinimap::DrawFOVCone(AHUD* HUD, float CenterX, float CenterY,
	const FMinimapConfig& Config)
{
	// FOV cone — 60° wedge pointing up (forward)
	float ConeLen = Config.Size * 0.35f;
	float HalfAngle = 30.f;
	float LeftRad = FMath::DegreesToRadians(-90.f - HalfAngle);
	float RightRad = FMath::DegreesToRadians(-90.f + HalfAngle);

	FLinearColor ConeColor(0.2f, 0.8f, 1.f, 0.1f);

	// Draw filled cone with multiple lines
	int32 Lines = 8;
	for (int32 i = 0; i <= Lines; i++)
	{
		float T = (float)i / Lines;
		float Rad = FMath::Lerp(LeftRad, RightRad, T);
		float EndX = CenterX + FMath::Cos(Rad) * ConeLen;
		float EndY = CenterY + FMath::Sin(Rad) * ConeLen;
		HUD->DrawLine(CenterX, CenterY, EndX, EndY, ConeColor, 1.f);
	}

	// Cone edge lines (brighter)
	FLinearColor EdgeColor(0.2f, 0.8f, 1.f, 0.25f);
	HUD->DrawLine(CenterX, CenterY,
		CenterX + FMath::Cos(LeftRad) * ConeLen,
		CenterY + FMath::Sin(LeftRad) * ConeLen, EdgeColor, 1.5f);
	HUD->DrawLine(CenterX, CenterY,
		CenterX + FMath::Cos(RightRad) * ConeLen,
		CenterY + FMath::Sin(RightRad) * ConeLen, EdgeColor, 1.5f);
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

		// Only draw if within minimap bounds
		if (LX >= Config.ScreenX && LX <= Config.ScreenX + Config.Size - 8.f &&
			LY >= Config.ScreenY && LY <= Config.ScreenY + Config.Size - 12.f)
		{
			HUD->DrawText(C.Label, C.Color, LX, LY, Font, 0.8f);
		}
	}
}

void FExoMinimap::DrawGridLines(AHUD* HUD, float CenterX, float CenterY,
	const FMinimapConfig& Config)
{
	FLinearColor GridColor(0.15f, 0.2f, 0.25f, 0.2f);
	float HalfSize = Config.Size * 0.5f;

	// Cross through center
	HUD->DrawLine(CenterX, Config.ScreenY, CenterX, Config.ScreenY + Config.Size,
		GridColor, 0.5f);
	HUD->DrawLine(Config.ScreenX, CenterY, Config.ScreenX + Config.Size, CenterY,
		GridColor, 0.5f);

	// Range rings (25% and 50% of range)
	FLinearColor RingColor(0.15f, 0.2f, 0.3f, 0.15f);
	int32 Segs = 24;
	float AngleStep = 2.f * PI / Segs;
	for (float Pct : {0.25f, 0.5f})
	{
		float R = HalfSize * Pct * 2.f; // Scale to fill minimap
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
