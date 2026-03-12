// ExoMaterialFactory.h — Runtime emissive materials for VFX
#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

/**
 * Creates and caches runtime PBR materials with emissive, textured, and glass support.
 * All materials are built at runtime with proper parameter graphs.
 */
class FExoMaterialFactory
{
public:
	/** Additive unlit — for tracers, flashes, sparks (pure energy overlay with bloom). */
	static UMaterialInterface* GetEmissiveAdditive();

	/** Opaque unlit — self-lit solid with bloom (glowing structural elements). */
	static UMaterialInterface* GetEmissiveOpaque();

	/** Default-lit + emissive — PBR surface that also glows (BaseColor + EmissiveColor). */
	static UMaterialInterface* GetLitEmissive();

	/** Default-lit + detail texture — PBR with world-position-tiled noise for surface detail. */
	static UMaterialInterface* GetLitTextured();

	/** Translucent glass — semi-transparent PBR with specular + emissive for window interiors. */
	static UMaterialInterface* GetGlassTranslucent();
};
