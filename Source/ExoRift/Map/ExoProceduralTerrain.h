#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoProceduralTerrain.generated.h"

class UProceduralMeshComponent;

UENUM(BlueprintType)
enum class ETerrainBiome : uint8
{
	Plains,
	Hills,
	Rocky,
	Coastal
};

/**
 * Generates a full 3D terrain using ProceduralMeshComponent.
 * Creates a heightmap-based island with varied elevation for the BR map.
 */
UCLASS()
class EXORIFT_API AExoProceduralTerrain : public AActor
{
	GENERATED_BODY()

public:
	AExoProceduralTerrain();

	/** Total terrain extent in each direction from center (full map = 2x this). */
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float MapExtent = 150000.f;

	/** Grid resolution — number of vertices per axis. Higher = smoother but heavier. */
	UPROPERTY(EditAnywhere, Category = "Terrain", meta = (ClampMin = "32", ClampMax = "512"))
	int32 GridResolution = 256;

	/** Maximum terrain height. */
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float MaxHeight = 8000.f;

	/** Random seed for deterministic generation. */
	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 Seed = 42;

	/** Whether terrain has been generated. */
	bool IsGenerated() const { return bGenerated; }

	/** Get terrain height at a world XY position. */
	float GetHeightAtLocation(const FVector2D& WorldXY) const;

	/** Generate the terrain mesh. Called by WorldBuilder. */
	void GenerateTerrain();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Terrain")
	UProceduralMeshComponent* TerrainMesh;

private:
	/** Sample the heightmap at normalized coordinates [0..1]. */
	float SampleHeight(float NormX, float NormY) const;

	/** Island falloff mask — ensures edges are at sea level. */
	float IslandMask(float NormX, float NormY) const;

	/** Multi-octave noise function. */
	float FractalNoise(float X, float Y, int32 Octaves) const;

	/** Simple hash-based noise. */
	float HashNoise(int32 IX, int32 IY) const;

	/** Smooth noise with interpolation. */
	float SmoothNoise(float X, float Y) const;

	bool bGenerated = false;
};
