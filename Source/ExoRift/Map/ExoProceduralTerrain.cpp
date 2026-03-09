#include "Map/ExoProceduralTerrain.h"
#include "ProceduralMeshComponent.h"
#include "ExoRift.h"

AExoProceduralTerrain::AExoProceduralTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	TerrainMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainMesh"));
	RootComponent = TerrainMesh;
	TerrainMesh->bUseAsyncCooking = true;
	TerrainMesh->SetCastShadow(true);
}

void AExoProceduralTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void AExoProceduralTerrain::GenerateTerrain()
{
	if (bGenerated) return;

	UE_LOG(LogExoRift, Log, TEXT("ProceduralTerrain: generating %dx%d grid, extent=%.0f"),
		GridResolution, GridResolution, MapExtent);

	const int32 NumVerts = GridResolution * GridResolution;
	const int32 NumTris = (GridResolution - 1) * (GridResolution - 1) * 6;
	const float CellSize = (MapExtent * 2.f) / (GridResolution - 1);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FColor> VertexColors;

	Vertices.Reserve(NumVerts);
	UVs.Reserve(NumVerts);
	VertexColors.Reserve(NumVerts);
	Triangles.Reserve(NumTris);
	Normals.SetNum(NumVerts);

	// Generate vertices
	for (int32 Y = 0; Y < GridResolution; ++Y)
	{
		for (int32 X = 0; X < GridResolution; ++X)
		{
			float NormX = static_cast<float>(X) / (GridResolution - 1);
			float NormY = static_cast<float>(Y) / (GridResolution - 1);

			float WorldX = -MapExtent + NormX * MapExtent * 2.f;
			float WorldY = -MapExtent + NormY * MapExtent * 2.f;
			float Height = SampleHeight(NormX, NormY);

			Vertices.Add(FVector(WorldX, WorldY, Height));
			UVs.Add(FVector2D(NormX * 100.f, NormY * 100.f));

			// Color by height: green plains, brown hills, grey peaks, blue below sea
			FColor VertColor;
			if (Height < 0.f)
			{
				VertColor = FColor(80, 120, 160); // underwater
			}
			else if (Height < MaxHeight * 0.2f)
			{
				VertColor = FColor(90, 140, 60); // grass plains
			}
			else if (Height < MaxHeight * 0.5f)
			{
				VertColor = FColor(120, 110, 70); // dirt/hills
			}
			else if (Height < MaxHeight * 0.8f)
			{
				VertColor = FColor(140, 130, 110); // rock
			}
			else
			{
				VertColor = FColor(180, 175, 170); // peak
			}
			VertexColors.Add(VertColor);
		}
	}

	// Generate triangles
	for (int32 Y = 0; Y < GridResolution - 1; ++Y)
	{
		for (int32 X = 0; X < GridResolution - 1; ++X)
		{
			int32 BL = Y * GridResolution + X;
			int32 BR = BL + 1;
			int32 TL = BL + GridResolution;
			int32 TR = TL + 1;

			Triangles.Add(BL);
			Triangles.Add(TL);
			Triangles.Add(BR);

			Triangles.Add(BR);
			Triangles.Add(TL);
			Triangles.Add(TR);
		}
	}

	// Calculate normals
	for (int32 i = 0; i < Triangles.Num(); i += 3)
	{
		const FVector& V0 = Vertices[Triangles[i]];
		const FVector& V1 = Vertices[Triangles[i + 1]];
		const FVector& V2 = Vertices[Triangles[i + 2]];

		FVector Edge1 = V1 - V0;
		FVector Edge2 = V2 - V0;
		FVector FaceNormal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

		Normals[Triangles[i]] += FaceNormal;
		Normals[Triangles[i + 1]] += FaceNormal;
		Normals[Triangles[i + 2]] += FaceNormal;
	}
	for (FVector& N : Normals)
	{
		N = N.GetSafeNormal();
	}

	TArray<FProcMeshTangent> Tangents;

	TerrainMesh->CreateMeshSection_LinearColor(
		0, Vertices, Triangles, Normals, UVs,
		TArray<FLinearColor>(), Tangents, true);

	// Apply vertex colors separately via the FColor overload
	TerrainMesh->ClearMeshSection(0);
	TerrainMesh->CreateMeshSection(
		0, Vertices, Triangles, Normals, UVs,
		VertexColors, Tangents, true);

	TerrainMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TerrainMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TerrainMesh->SetCollisionObjectType(ECC_WorldStatic);

	bGenerated = true;
	UE_LOG(LogExoRift, Log, TEXT("ProceduralTerrain: generated %d verts, %d tris"),
		Vertices.Num(), Triangles.Num() / 3);
}

float AExoProceduralTerrain::GetHeightAtLocation(const FVector2D& WorldXY) const
{
	float NormX = (WorldXY.X + MapExtent) / (MapExtent * 2.f);
	float NormY = (WorldXY.Y + MapExtent) / (MapExtent * 2.f);
	NormX = FMath::Clamp(NormX, 0.f, 1.f);
	NormY = FMath::Clamp(NormY, 0.f, 1.f);
	return SampleHeight(NormX, NormY);
}

float AExoProceduralTerrain::SampleHeight(float NormX, float NormY) const
{
	float Mask = IslandMask(NormX, NormY);
	if (Mask <= 0.f) return -500.f;

	// Large terrain features
	float LargeNoise = FractalNoise(NormX * 3.f + Seed, NormY * 3.f + Seed, 5);

	// Medium hills
	float MediumNoise = FractalNoise(NormX * 8.f + Seed * 2.f, NormY * 8.f + Seed * 2.f, 3) * 0.3f;

	// Small detail
	float DetailNoise = FractalNoise(NormX * 20.f + Seed * 3.f, NormY * 20.f + Seed * 3.f, 2) * 0.08f;

	float Combined = (LargeNoise + MediumNoise + DetailNoise);

	// Create some flat areas for POIs (plateau effect)
	float CenterDist = FMath::Sqrt(FMath::Square(NormX - 0.5f) + FMath::Square(NormY - 0.5f));
	float PlateauFactor = FMath::Clamp(1.f - CenterDist * 3.f, 0.f, 1.f);
	Combined = FMath::Lerp(Combined, FMath::Clamp(Combined, 0.1f, 0.4f), PlateauFactor * 0.5f);

	return Combined * MaxHeight * Mask;
}

float AExoProceduralTerrain::IslandMask(float NormX, float NormY) const
{
	// Elliptical island shape with some noise on the edge
	float DX = (NormX - 0.5f) * 2.f;
	float DY = (NormY - 0.5f) * 2.f;
	float Dist = FMath::Sqrt(DX * DX + DY * DY);

	// Add edge noise for organic coastline
	float EdgeNoise = FractalNoise(NormX * 6.f + 100.f, NormY * 6.f + 100.f, 3) * 0.15f;
	float EdgeThreshold = 0.85f + EdgeNoise;

	if (Dist > EdgeThreshold) return 0.f;

	// Smooth falloff near edges
	float FalloffStart = EdgeThreshold - 0.2f;
	if (Dist > FalloffStart)
	{
		return 1.f - (Dist - FalloffStart) / (EdgeThreshold - FalloffStart);
	}

	return 1.f;
}

float AExoProceduralTerrain::FractalNoise(float X, float Y, int32 Octaves) const
{
	float Value = 0.f;
	float Amplitude = 1.f;
	float Frequency = 1.f;
	float MaxAmplitude = 0.f;

	for (int32 i = 0; i < Octaves; ++i)
	{
		Value += SmoothNoise(X * Frequency, Y * Frequency) * Amplitude;
		MaxAmplitude += Amplitude;
		Amplitude *= 0.5f;
		Frequency *= 2.f;
	}

	return (MaxAmplitude > 0.f) ? Value / MaxAmplitude : 0.f;
}

float AExoProceduralTerrain::HashNoise(int32 IX, int32 IY) const
{
	int32 N = IX + IY * 57 + Seed * 131;
	N = (N << 13) ^ N;
	return (1.f - ((N * (N * N * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.f);
}

float AExoProceduralTerrain::SmoothNoise(float X, float Y) const
{
	int32 IX = FMath::FloorToInt(X);
	int32 IY = FMath::FloorToInt(Y);
	float FracX = X - IX;
	float FracY = Y - IY;

	// Smoothstep interpolation
	FracX = FracX * FracX * (3.f - 2.f * FracX);
	FracY = FracY * FracY * (3.f - 2.f * FracY);

	float V00 = HashNoise(IX, IY);
	float V10 = HashNoise(IX + 1, IY);
	float V01 = HashNoise(IX, IY + 1);
	float V11 = HashNoise(IX + 1, IY + 1);

	float I0 = FMath::Lerp(V00, V10, FracX);
	float I1 = FMath::Lerp(V01, V11, FracX);

	return FMath::Lerp(I0, I1, FracY);
}
