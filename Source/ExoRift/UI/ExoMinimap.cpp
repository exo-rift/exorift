#include "UI/ExoMinimap.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Player/ExoCharacter.h"
#include "Map/ExoZoneSystem.h"
#include "Map/ExoPOI.h"
#include "EngineUtils.h"

void FExoMinimap::Draw(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config)
{
	if (!HUD || !Canvas) return;

	APawn* PlayerPawn = HUD->GetOwningPawn();
	if (!PlayerPawn) return;

	FVector PlayerPos = PlayerPawn->GetActorLocation();
	float CenterX = Config.ScreenX + Config.Size * 0.5f;
	float CenterY = Config.ScreenY + Config.Size * 0.5f;

	// Background
	HUD->DrawRect(FLinearColor(0.02f, 0.03f, 0.05f, 0.8f),
		Config.ScreenX, Config.ScreenY, Config.Size, Config.Size);

	// Border
	FLinearColor BorderColor(0.3f, 0.4f, 0.5f, 0.8f);
	HUD->DrawLine(Config.ScreenX, Config.ScreenY,
		Config.ScreenX + Config.Size, Config.ScreenY, BorderColor, Config.BorderWidth);
	HUD->DrawLine(Config.ScreenX + Config.Size, Config.ScreenY,
		Config.ScreenX + Config.Size, Config.ScreenY + Config.Size, BorderColor, Config.BorderWidth);
	HUD->DrawLine(Config.ScreenX + Config.Size, Config.ScreenY + Config.Size,
		Config.ScreenX, Config.ScreenY + Config.Size, BorderColor, Config.BorderWidth);
	HUD->DrawLine(Config.ScreenX, Config.ScreenY + Config.Size,
		Config.ScreenX, Config.ScreenY, BorderColor, Config.BorderWidth);

	// Draw zone circle
	AExoZoneSystem* Zone = nullptr;
	for (TActorIterator<AExoZoneSystem> It(HUD->GetWorld()); It; ++It)
	{
		Zone = *It;
		break;
	}
	if (Zone)
	{
		DrawZoneCircle(HUD, Canvas, Config, PlayerPos, Zone);
	}

	// Draw POI markers
	DrawPOIMarkers(HUD, Canvas, Config, PlayerPos);

	// Draw other players/bots as dots
	for (TActorIterator<AExoCharacter> It(HUD->GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive() || Char == PlayerPawn) continue;

		// Only show nearby enemies on minimap
		float Dist = FVector::Dist(PlayerPos, Char->GetActorLocation());
		if (Dist > Config.WorldRange) continue;

		FVector2D MinimapPos = WorldToMinimap(Char->GetActorLocation(), PlayerPos, Config);

		// Clamp to minimap bounds
		MinimapPos.X = FMath::Clamp(MinimapPos.X, Config.ScreenX + 4.f, Config.ScreenX + Config.Size - 4.f);
		MinimapPos.Y = FMath::Clamp(MinimapPos.Y, Config.ScreenY + 4.f, Config.ScreenY + Config.Size - 4.f);

		// Red dot for enemies
		HUD->DrawRect(FLinearColor(1.f, 0.2f, 0.2f, 0.9f),
			MinimapPos.X - Config.PlayerDotSize * 0.5f,
			MinimapPos.Y - Config.PlayerDotSize * 0.5f,
			Config.PlayerDotSize, Config.PlayerDotSize);
	}

	// Player dot (center, with direction indicator)
	FLinearColor PlayerColor(0.2f, 0.8f, 1.f, 1.f);
	float DotSize = Config.PlayerDotSize * 1.5f;
	HUD->DrawRect(PlayerColor, CenterX - DotSize * 0.5f, CenterY - DotSize * 0.5f, DotSize, DotSize);

	// Direction line
	FRotator PlayerRot = PlayerPawn->GetActorRotation();
	float DirLen = 12.f;
	FVector2D DirEnd(
		CenterX + FMath::Sin(FMath::DegreesToRadians(PlayerRot.Yaw)) * DirLen,
		CenterY - FMath::Cos(FMath::DegreesToRadians(PlayerRot.Yaw)) * DirLen
	);
	HUD->DrawLine(CenterX, CenterY, DirEnd.X, DirEnd.Y, PlayerColor, 2.f);

	// Cardinal direction labels
	float LabelOffset = Config.Size * 0.5f - 12.f;
	HUD->DrawText(TEXT("N"), FLinearColor(0.7f, 0.7f, 0.7f, 0.6f),
		CenterX - 4.f, Config.ScreenY + 4.f);
}

FVector2D FExoMinimap::WorldToMinimap(const FVector& WorldPos, const FVector& CenterPos,
	const FMinimapConfig& Config)
{
	FVector Offset = WorldPos - CenterPos;
	float Scale = Config.Size / (Config.WorldRange * 2.f);

	return FVector2D(
		Config.ScreenX + Config.Size * 0.5f + Offset.Y * Scale,  // Y axis = East/West
		Config.ScreenY + Config.Size * 0.5f - Offset.X * Scale   // X axis = North/South
	);
}

void FExoMinimap::DrawZoneCircle(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config,
	const FVector& CenterPos, AExoZoneSystem* Zone)
{
	FVector ZoneWorldCenter(Zone->GetCurrentCenter().X, Zone->GetCurrentCenter().Y, 0.f);
	FVector2D ZoneMiniCenter = WorldToMinimap(ZoneWorldCenter, CenterPos, Config);

	float ZoneRadiusWorld = Zone->GetCurrentRadius();
	float Scale = Config.Size / (Config.WorldRange * 2.f);
	float ZoneRadiusMinimap = ZoneRadiusWorld * Scale;

	// Draw circle approximation
	FLinearColor ZoneColor(0.1f, 0.4f, 1.f, 0.6f);
	int32 Segments = 32;
	float AngleStep = 2.f * PI / Segments;
	for (int32 i = 0; i < Segments; i++)
	{
		float A1 = i * AngleStep;
		float A2 = (i + 1) * AngleStep;
		FVector2D P1(ZoneMiniCenter.X + FMath::Cos(A1) * ZoneRadiusMinimap,
			ZoneMiniCenter.Y + FMath::Sin(A1) * ZoneRadiusMinimap);
		FVector2D P2(ZoneMiniCenter.X + FMath::Cos(A2) * ZoneRadiusMinimap,
			ZoneMiniCenter.Y + FMath::Sin(A2) * ZoneRadiusMinimap);
		HUD->DrawLine(P1.X, P1.Y, P2.X, P2.Y, ZoneColor, 1.5f);
	}

	// Target zone (where it's shrinking to)
	FVector TargetWorldCenter(Zone->GetTargetCenter().X, Zone->GetTargetCenter().Y, 0.f);
	FVector2D TargetMiniCenter = WorldToMinimap(TargetWorldCenter, CenterPos, Config);
	float TargetRadius = Zone->GetTargetRadius() * Scale;

	FLinearColor TargetColor(1.f, 1.f, 1.f, 0.3f);
	for (int32 i = 0; i < Segments; i++)
	{
		float A1 = i * AngleStep;
		float A2 = (i + 1) * AngleStep;
		FVector2D P1(TargetMiniCenter.X + FMath::Cos(A1) * TargetRadius,
			TargetMiniCenter.Y + FMath::Sin(A1) * TargetRadius);
		FVector2D P2(TargetMiniCenter.X + FMath::Cos(A2) * TargetRadius,
			TargetMiniCenter.Y + FMath::Sin(A2) * TargetRadius);
		HUD->DrawLine(P1.X, P1.Y, P2.X, P2.Y, TargetColor, 1.f);
	}
}

void FExoMinimap::DrawPOIMarkers(AHUD* HUD, UCanvas* Canvas, const FMinimapConfig& Config,
	const FVector& CenterPos)
{
	FLinearColor POIColor(0.9f, 0.8f, 0.3f, 0.7f);

	for (TActorIterator<AExoPOI> It(HUD->GetWorld()); It; ++It)
	{
		AExoPOI* POI = *It;
		if (!POI) continue;

		FVector2D MinimapPos = WorldToMinimap(POI->GetActorLocation(), CenterPos, Config);

		// Only draw if within minimap bounds
		if (MinimapPos.X < Config.ScreenX || MinimapPos.X > Config.ScreenX + Config.Size) continue;
		if (MinimapPos.Y < Config.ScreenY || MinimapPos.Y > Config.ScreenY + Config.Size) continue;

		// Diamond marker
		float MarkerSize = 3.f;
		HUD->DrawLine(MinimapPos.X, MinimapPos.Y - MarkerSize,
			MinimapPos.X + MarkerSize, MinimapPos.Y, POIColor, 1.5f);
		HUD->DrawLine(MinimapPos.X + MarkerSize, MinimapPos.Y,
			MinimapPos.X, MinimapPos.Y + MarkerSize, POIColor, 1.5f);
		HUD->DrawLine(MinimapPos.X, MinimapPos.Y + MarkerSize,
			MinimapPos.X - MarkerSize, MinimapPos.Y, POIColor, 1.5f);
		HUD->DrawLine(MinimapPos.X - MarkerSize, MinimapPos.Y,
			MinimapPos.X, MinimapPos.Y - MarkerSize, POIColor, 1.5f);
	}
}
