// ExoLevelBuilderTerrain.cpp — Ground plane, terrain patches, ridges, hills, boulders
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"

void AExoLevelBuilder::BuildTerrain()
{
	// Large ground plane — warm rocky surface
	float GroundHalf = MapHalfSize * 1.5f;
	float GroundScale = GroundHalf / 50.f;
	SpawnStaticMesh(
		FVector(0.f, 0.f, -50.f),
		FVector(GroundScale, GroundScale, 1.f),
		FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.12f, 0.1f, 0.085f)); // Sandy rock base

	// Terrain variation — raised patches with warm/cool variety
	struct FTerrainPatch { FVector Pos; float SizeX; float SizeY; float Height; FLinearColor Color; };
	TArray<FTerrainPatch> Patches = {
		{{-80000.f, -60000.f, 0.f}, 40000.f, 30000.f, 200.f, FLinearColor(0.13f, 0.11f, 0.09f)},
		{{60000.f, 80000.f, 0.f}, 35000.f, 25000.f, 150.f, FLinearColor(0.1f, 0.11f, 0.12f)},
		{{-40000.f, 100000.f, 0.f}, 50000.f, 20000.f, 300.f, FLinearColor(0.14f, 0.12f, 0.09f)},
		{{100000.f, -50000.f, 0.f}, 30000.f, 40000.f, 250.f, FLinearColor(0.1f, 0.1f, 0.115f)},
		{{-120000.f, 30000.f, 0.f}, 25000.f, 35000.f, 180.f, FLinearColor(0.12f, 0.11f, 0.1f)},
	};

	for (const auto& P : Patches)
	{
		SpawnStaticMesh(
			FVector(P.Pos.X, P.Pos.Y, P.Height * 0.5f),
			FVector(P.SizeX / 100.f, P.SizeY / 100.f, P.Height / 100.f),
			FRotator::ZeroRotator, CubeMesh, P.Color);
	}

	// Rocky ridgelines — varied brown/grey rock tones
	struct FRidge { FVector Start; FVector End; float Height; float Width; };
	TArray<FRidge> Ridges = {
		{{30000.f, 20000.f, 0.f}, {50000.f, 35000.f, 0.f}, 400.f, 2000.f},
		{{-40000.f, -10000.f, 0.f}, {-60000.f, -30000.f, 0.f}, 350.f, 1800.f},
		{{70000.f, -80000.f, 0.f}, {90000.f, -60000.f, 0.f}, 500.f, 2500.f},
		{{-90000.f, 70000.f, 0.f}, {-70000.f, 80000.f, 0.f}, 300.f, 1500.f},
		{{100000.f, 30000.f, 0.f}, {110000.f, 50000.f, 0.f}, 450.f, 2200.f},
	};
	for (const auto& R : Ridges)
	{
		FVector Dir = R.End - R.Start;
		float Len = Dir.Size();
		FRotator Rot = Dir.Rotation();
		int32 Segments = FMath::Max(2, FMath::RoundToInt32(Len / 5000.f));
		for (int32 i = 0; i < Segments; i++)
		{
			float T = (float)i / (float)Segments;
			FVector Pos = FMath::Lerp(R.Start, R.End, T + 0.5f / Segments);
			float SegH = R.Height * (0.6f + 0.4f * FMath::Sin(T * PI));
			float SegW = R.Width * (0.8f + 0.2f * FMath::Sin(T * 3.f));
			// Vary rock color per segment
			float CV = 0.1f + (i % 3) * 0.02f;
			SpawnStaticMesh(
				FVector(Pos.X, Pos.Y, SegH * 0.5f),
				FVector(Len / Segments / 100.f, SegW / 100.f, SegH / 100.f),
				FRotator(FMath::RandRange(-3.f, 3.f), Rot.Yaw, 0.f),
				CubeMesh, FLinearColor(CV * 1.15f, CV, CV * 0.85f));
		}
	}

	// Hills — rounded terrain bumps with warmer earth tones
	struct FHill { FVector Pos; float Radius; float Height; };
	TArray<FHill> Hills = {
		{{-20000.f, 40000.f, -100.f}, 6000.f, 600.f},
		{{50000.f, -20000.f, -100.f}, 5000.f, 500.f},
		{{-60000.f, 70000.f, -100.f}, 7000.f, 400.f},
		{{80000.f, 60000.f, -100.f}, 5500.f, 550.f},
		{{-100000.f, -100000.f, -100.f}, 8000.f, 350.f},
		{{110000.f, -30000.f, -100.f}, 4500.f, 450.f},
	};
	for (int32 i = 0; i < Hills.Num(); i++)
	{
		const auto& H = Hills[i];
		float SR = H.Radius / 50.f;
		float SH = H.Height / 50.f;
		// Alternate between warm grey and cool grey
		FLinearColor HillCol = (i % 2 == 0)
			? FLinearColor(0.14f, 0.12f, 0.1f)   // Warm sandy brown
			: FLinearColor(0.1f, 0.11f, 0.13f);  // Cool blue-grey
		SpawnStaticMesh(H.Pos,
			FVector(SR, SR, SH), FRotator::ZeroRotator, SphereMesh, HillCol);
	}

	// Scattered boulders across the map — break up flat ground
	struct FBoulder { FVector Pos; float Scale; FRotator Rot; FLinearColor Col; };
	TArray<FBoulder> Boulders = {
		// Near hub
		{{8000.f, -12000.f, 0.f}, 1.8f, {5.f, 30.f, 0.f}, {0.13f, 0.11f, 0.1f}},
		{{-14000.f, 7000.f, 0.f}, 2.2f, {-3.f, 120.f, 8.f}, {0.1f, 0.1f, 0.12f}},
		// Mid-field
		{{30000.f, 50000.f, 0.f}, 3.f, {10.f, 200.f, -5.f}, {0.14f, 0.12f, 0.11f}},
		{{-45000.f, -35000.f, 0.f}, 2.5f, {-8.f, 70.f, 3.f}, {0.11f, 0.11f, 0.13f}},
		{{55000.f, 30000.f, 0.f}, 1.5f, {12.f, 310.f, 0.f}, {0.12f, 0.1f, 0.09f}},
		{{-25000.f, 55000.f, 0.f}, 2.8f, {0.f, 160.f, 7.f}, {0.1f, 0.11f, 0.12f}},
		// Far field
		{{90000.f, -80000.f, 0.f}, 4.f, {15.f, 45.f, -10.f}, {0.13f, 0.12f, 0.1f}},
		{{-80000.f, 90000.f, 0.f}, 3.5f, {-5.f, 250.f, 0.f}, {0.1f, 0.09f, 0.11f}},
		{{-110000.f, -50000.f, 0.f}, 5.f, {8.f, 90.f, -3.f}, {0.11f, 0.12f, 0.1f}},
		{{70000.f, -110000.f, 0.f}, 3.f, {-12.f, 180.f, 5.f}, {0.12f, 0.11f, 0.12f}},
		// Along roads
		{{20000.f, 5000.f, 0.f}, 1.2f, {3.f, 55.f, 0.f}, {0.13f, 0.12f, 0.11f}},
		{{-5000.f, 30000.f, 0.f}, 1.5f, {-6.f, 140.f, 4.f}, {0.11f, 0.1f, 0.12f}},
		{{40000.f, -5000.f, 0.f}, 1.8f, {0.f, 220.f, -2.f}, {0.1f, 0.11f, 0.1f}},
		{{-30000.f, -10000.f, 0.f}, 2.f, {7.f, 340.f, 0.f}, {0.12f, 0.12f, 0.11f}},
	};
	for (const auto& B : Boulders)
	{
		SpawnStaticMesh(
			B.Pos + FVector(0.f, 0.f, B.Scale * 15.f),
			FVector(B.Scale, B.Scale * 0.7f, B.Scale * 0.5f),
			B.Rot, SphereMesh, B.Col);
		// Cluster: 1-2 smaller rocks nearby
		SpawnStaticMesh(
			B.Pos + FVector(B.Scale * 60.f, B.Scale * 40.f, B.Scale * 5.f),
			FVector(B.Scale * 0.3f, B.Scale * 0.25f, B.Scale * 0.2f),
			FRotator(B.Rot.Pitch + 15.f, B.Rot.Yaw + 60.f, 0.f),
			SphereMesh, FLinearColor(B.Col.R * 0.9f, B.Col.G * 0.95f, B.Col.B));
	}

	// Concrete pads around compound areas — lighter ground surfaces
	FLinearColor ConcretePad(0.16f, 0.15f, 0.14f);
	FLinearColor ConcreteEdge(0.1f, 0.1f, 0.11f);
	struct FPad { FVector Pos; float SX; float SY; };
	TArray<FPad> CompoundPads = {
		{{0.f, 0.f, 5.f}, 200.f, 150.f},         // Hub
		{{0.f, 80000.f, 5.f}, 250.f, 120.f},      // North
		{{0.f, -80000.f, 5.f}, 200.f, 150.f},     // South
		{{80000.f, 0.f, 5.f}, 150.f, 200.f},      // East
		{{-80000.f, 0.f, 5.f}, 120.f, 180.f},     // West
	};
	for (const auto& Pad : CompoundPads)
	{
		// Main pad
		SpawnStaticMesh(Pad.Pos,
			FVector(Pad.SX, Pad.SY, 0.1f), FRotator::ZeroRotator, CubeMesh, ConcretePad);
		// Darker edge border — Z=2 below pad to avoid z-fighting
		SpawnStaticMesh(Pad.Pos + FVector(0.f, 0.f, -3.f),
			FVector(Pad.SX + 2.f, Pad.SY + 2.f, 0.05f), FRotator::ZeroRotator, CubeMesh,
			ConcreteEdge);
	}

	// Drainage channels — thin dark depressions suggesting dried waterways
	FLinearColor ChannelColor(0.06f, 0.06f, 0.07f);
	struct FChannel { FVector Start; FVector End; float Width; };
	TArray<FChannel> Channels = {
		{{20000.f, 5000.f, 2.f}, {35000.f, 25000.f, 2.f}, 3.f},
		{{-15000.f, -10000.f, 2.f}, {-30000.f, -35000.f, 2.f}, 4.f},
		{{50000.f, -10000.f, 2.f}, {65000.f, -25000.f, 2.f}, 3.f},
		{{-40000.f, 50000.f, 2.f}, {-55000.f, 65000.f, 2.f}, 3.5f},
		{{10000.f, -30000.f, 2.f}, {25000.f, -50000.f, 2.f}, 2.5f},
	};
	for (const auto& Ch : Channels)
	{
		FVector Dir = Ch.End - Ch.Start;
		float Len = Dir.Size();
		FVector Mid = (Ch.Start + Ch.End) * 0.5f;
		FRotator ChRot = Dir.Rotation();
		SpawnStaticMesh(Mid, FVector(Len / 100.f, Ch.Width, 0.02f),
			ChRot, CubeMesh, ChannelColor);
	}

	// Worn path strips between compounds — subtle ground markings
	FLinearColor PathColor(0.14f, 0.12f, 0.1f);
	SpawnStaticMesh(FVector(0.f, 40000.f, 14.f),
		FVector(8.f, 400.f, 0.05f), FRotator::ZeroRotator, CubeMesh, PathColor);
	SpawnStaticMesh(FVector(0.f, -40000.f, 14.f),
		FVector(8.f, 400.f, 0.05f), FRotator::ZeroRotator, CubeMesh, PathColor);
	SpawnStaticMesh(FVector(40000.f, 0.f, 14.f),
		FVector(400.f, 8.f, 0.05f), FRotator::ZeroRotator, CubeMesh, PathColor);
	SpawnStaticMesh(FVector(-40000.f, 0.f, 14.f),
		FVector(400.f, 8.f, 0.05f), FRotator::ZeroRotator, CubeMesh, PathColor);

	// Expansion joints on concrete pads — creates a cast-panel grid pattern
	FLinearColor JointColor(0.07f, 0.07f, 0.08f);
	float JointSpacing = 2000.f;
	for (const auto& Pad : CompoundPads)
	{
		float ExtX = Pad.SX * 50.f; // Half-extent in world units
		float ExtY = Pad.SY * 50.f;
		for (float JY = -ExtY + JointSpacing; JY < ExtY; JY += JointSpacing)
		{
			SpawnStaticMesh(Pad.Pos + FVector(0.f, JY, 13.f),
				FVector(Pad.SX * 0.98f, 0.03f, 0.015f),
				FRotator::ZeroRotator, CubeMesh, JointColor);
		}
		for (float JX = -ExtX + JointSpacing; JX < ExtX; JX += JointSpacing)
		{
			SpawnStaticMesh(Pad.Pos + FVector(JX, 0.f, 13.f),
				FVector(0.03f, Pad.SY * 0.98f, 0.015f),
				FRotator::ZeroRotator, CubeMesh, JointColor);
		}
	}

	// Ground stains near compounds — oil/chemical spills for visual variation
	FLinearColor StainDark(0.08f, 0.07f, 0.065f);
	struct FStain { FVector Pos; float SX; float SY; float Yaw; };
	TArray<FStain> Stains = {
		{{2000.f, 3000.f, 15.f}, 8.f, 5.f, 15.f},
		{{-4000.f, -2000.f, 15.f}, 6.f, 4.f, -30.f},
		{{15000.f, 1000.f, 15.f}, 10.f, 3.f, 5.f},
		{{-1000.f, 20000.f, 15.f}, 4.f, 8.f, -20.f},
		{{5000.f, 78000.f, 15.f}, 7.f, 6.f, 40.f},
		{{-82000.f, 2000.f, 15.f}, 5.f, 5.f, -15.f},
		{{78000.f, -5000.f, 15.f}, 8.f, 4.f, 60.f},
		{{-3000.f, -78000.f, 15.f}, 6.f, 7.f, -45.f},
		// Near roads and scattered structures
		{{30000.f, 12000.f, 15.f}, 5.f, 3.f, 70.f},
		{{-20000.f, -35000.f, 15.f}, 4.f, 6.f, -50.f},
		{{45000.f, -20000.f, 15.f}, 7.f, 3.f, 25.f},
		{{-55000.f, 40000.f, 15.f}, 4.f, 5.f, -10.f},
	};
	for (const auto& S : Stains)
	{
		SpawnStaticMesh(S.Pos, FVector(S.SX, S.SY, 0.02f),
			FRotator(0.f, S.Yaw, 0.f), CubeMesh, StainDark);
	}

	// Map edge cliffs — define play area boundary
	FLinearColor CliffDark(0.07f, 0.07f, 0.08f);
	float EdgeDist = MapHalfSize * 1.1f;
	float CliffH = 3000.f;
	float CliffThick = 2000.f;
	// North edge
	SpawnStaticMesh(FVector(0.f, EdgeDist, CliffH * 0.5f),
		FVector(EdgeDist * 2.f / 100.f, CliffThick / 100.f, CliffH / 100.f),
		FRotator::ZeroRotator, CubeMesh, CliffDark);
	// South edge
	SpawnStaticMesh(FVector(0.f, -EdgeDist, CliffH * 0.5f),
		FVector(EdgeDist * 2.f / 100.f, CliffThick / 100.f, CliffH / 100.f),
		FRotator::ZeroRotator, CubeMesh, CliffDark);
	// East edge
	SpawnStaticMesh(FVector(EdgeDist, 0.f, CliffH * 0.5f),
		FVector(CliffThick / 100.f, EdgeDist * 2.f / 100.f, CliffH / 100.f),
		FRotator::ZeroRotator, CubeMesh, CliffDark);
	// West edge
	SpawnStaticMesh(FVector(-EdgeDist, 0.f, CliffH * 0.5f),
		FVector(CliffThick / 100.f, EdgeDist * 2.f / 100.f, CliffH / 100.f),
		FRotator::ZeroRotator, CubeMesh, CliffDark);

	// Geological strata bands — layered rock faces on cliff walls
	FLinearColor Strata[] = {
		{0.08f, 0.07f, 0.09f},    // Cool bluish
		{0.1f, 0.085f, 0.07f},    // Rusty warm
		{0.06f, 0.06f, 0.07f},    // Dark layer
		{0.11f, 0.1f, 0.09f},     // Lighter warm
	};
	float StrataH = CliffH / 5.f;
	for (int32 s = 0; s < 4; s++)
	{
		float BandZ = StrataH * (s + 1);
		float BandH = StrataH * 0.25f;
		// North/South cliff faces (extend along X)
		SpawnStaticMesh(FVector(0.f, EdgeDist + 2.f, BandZ),
			FVector(EdgeDist * 2.f / 100.f, 0.04f, BandH / 100.f),
			FRotator::ZeroRotator, CubeMesh, Strata[s]);
		SpawnStaticMesh(FVector(0.f, -EdgeDist - 2.f, BandZ),
			FVector(EdgeDist * 2.f / 100.f, 0.04f, BandH / 100.f),
			FRotator::ZeroRotator, CubeMesh, Strata[s]);
		// East/West cliff faces (extend along Y)
		SpawnStaticMesh(FVector(EdgeDist + 2.f, 0.f, BandZ),
			FVector(0.04f, EdgeDist * 2.f / 100.f, BandH / 100.f),
			FRotator::ZeroRotator, CubeMesh, Strata[s]);
		SpawnStaticMesh(FVector(-EdgeDist - 2.f, 0.f, BandZ),
			FVector(0.04f, EdgeDist * 2.f / 100.f, BandH / 100.f),
			FRotator::ZeroRotator, CubeMesh, Strata[s]);
	}

	// Mid-field rock outcrops — larger formations between compounds
	struct FOutcrop { FVector Pos; float Scale; float Yaw; };
	TArray<FOutcrop> Outcrops = {
		{{35000.f, 35000.f, 0.f}, 6.f, 25.f},
		{{-45000.f, -25000.f, 0.f}, 5.f, 140.f},
		{{55000.f, -55000.f, 0.f}, 7.f, 75.f},
		{{-65000.f, 55000.f, 0.f}, 5.5f, -30.f},
		{{25000.f, -45000.f, 0.f}, 4.5f, 200.f},
		{{-30000.f, 65000.f, 0.f}, 6.5f, 310.f},
		{{75000.f, 30000.f, 0.f}, 5.f, 55.f},
		{{-55000.f, -65000.f, 0.f}, 4.f, 160.f},
	};
	for (int32 oi = 0; oi < Outcrops.Num(); oi++)
	{
		const auto& OC = Outcrops[oi];
		FRotator OCRot(0.f, OC.Yaw, 0.f);

		// Use KayKit terrain meshes when available
		if (bHasKayKitAssets)
		{
			UStaticMesh* TerrainMesh = (oi % 2 == 0) ? KK_TerrainTall : KK_TerrainLow;
			if (TerrainMesh)
			{
				SpawnRawMesh(OC.Pos, FVector(OC.Scale * 0.5f),
					FRotator(0.f, OC.Yaw, 0.f), TerrainMesh);
			}
			if (KK_Rock)
			{
				SpawnRawMesh(OC.Pos + OCRot.RotateVector(FVector(OC.Scale * 60.f, 0.f, 0.f)),
					FVector(OC.Scale * 0.4f), FRotator(0.f, OC.Yaw + 45.f, 0.f), KK_Rock);
			}
			continue;
		}

		FLinearColor RockCol(0.1f, 0.09f, 0.085f);
		// Main slab
		SpawnStaticMesh(OC.Pos + FVector(0.f, 0.f, OC.Scale * 25.f),
			FVector(OC.Scale * 1.5f, OC.Scale, OC.Scale * 0.4f),
			FRotator(5.f, OC.Yaw, 3.f), CubeMesh, RockCol);
		// Leaning spire
		SpawnStaticMesh(OC.Pos + OCRot.RotateVector(FVector(OC.Scale * 50.f, 0.f, OC.Scale * 40.f)),
			FVector(OC.Scale * 0.3f, OC.Scale * 0.25f, OC.Scale * 0.8f),
			FRotator(-8.f, OC.Yaw + 15.f, 5.f), CubeMesh,
			FLinearColor(0.09f, 0.08f, 0.095f));
		// Scattered fragments
		SpawnStaticMesh(OC.Pos + OCRot.RotateVector(FVector(-OC.Scale * 40.f, OC.Scale * 30.f, OC.Scale * 8.f)),
			FVector(OC.Scale * 0.5f, OC.Scale * 0.35f, OC.Scale * 0.2f),
			FRotator(12.f, OC.Yaw + 60.f, -4.f), CubeMesh,
			FLinearColor(0.11f, 0.1f, 0.09f));
		SpawnStaticMesh(OC.Pos + OCRot.RotateVector(FVector(OC.Scale * 30.f, -OC.Scale * 35.f, OC.Scale * 5.f)),
			FVector(OC.Scale * 0.4f, OC.Scale * 0.3f, OC.Scale * 0.15f),
			FRotator(8.f, OC.Yaw - 30.f, 6.f), CubeMesh,
			FLinearColor(0.095f, 0.09f, 0.085f));
	}

	// Cliff-base rubble — debris at foot of boundary walls
	for (float Angle = 0.f; Angle < 360.f; Angle += 25.f)
	{
		float Rad = FMath::DegreesToRadians(Angle);
		float D = EdgeDist - 400.f;
		FVector RPos(FMath::Cos(Rad) * D, FMath::Sin(Rad) * D, 0.f);
		float S = 1.5f + FMath::Abs(FMath::Sin(Angle * 0.1f)) * 2.f;
		SpawnStaticMesh(RPos + FVector(0.f, 0.f, S * 15.f),
			FVector(S * 1.3f, S * 0.8f, S * 0.5f),
			FRotator(FMath::Sin(Angle) * 8.f, Angle + 90.f, 0.f),
			CubeMesh, FLinearColor(0.07f, 0.07f, 0.08f));
	}
}
