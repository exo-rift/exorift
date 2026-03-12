// ExoProceduralTextures.h — Runtime procedural noise textures for PBR detail
#pragma once

#include "CoreMinimal.h"

class UTexture2D;

/**
 * Generates tileable noise textures at runtime for surface detail.
 * Used by ExoMaterialFactory to break up flat-colored surfaces with
 * natural stain/wear/scratch variation via world-position UV tiling.
 */
struct FExoProceduralTextures
{
	/** Broad noise for ground/terrain — stains, cracks, natural color variation. */
	static UTexture2D* GetGroundNoise();

	/** Finer noise for metal/structural — scratches, wear, weathering patterns. */
	static UTexture2D* GetMetalNoise();

	/** Tangent-space normal map for ground — broad bumpy terrain relief. */
	static UTexture2D* GetGroundNormal();

	/** Tangent-space normal map for metal — fine panel lines and scratches. */
	static UTexture2D* GetMetalNormal();
};
