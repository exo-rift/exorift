// ExoMaterialFactory.h — Runtime emissive materials for VFX
#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

/**
 * Creates and caches runtime materials with proper EmissiveColor support.
 * BasicShapeMaterial has no emissive parameter, so SetVectorParameterValue
 * for "EmissiveColor" silently fails — no bloom, no glow.
 * These materials fix that with actual emissive output.
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
};
