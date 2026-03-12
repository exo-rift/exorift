// ExoMaterialFactory.cpp — Runtime emissive and PBR material creation
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoProceduralTextures.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/Package.h"
#include "Engine/Texture2D.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionAdd.h"

static UMaterialInterface* CachedAdditive = nullptr;
static UMaterialInterface* CachedOpaque = nullptr;
static UMaterialInterface* CachedLitEmissive = nullptr;
static UMaterialInterface* CachedLitTextured = nullptr;
static UMaterialInterface* CachedGlass = nullptr;


static UMaterial* BuildRuntimeMaterial(EBlendMode Blend, EMaterialShadingModel Shading,
	const TCHAR* Name, bool bAddBaseColor)
{
	UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), Name);
	Mat->SetShadingModel(Shading);
	Mat->BlendMode = Blend;
	Mat->TwoSided = true;

	// Emissive color parameter
	UMaterialExpressionVectorParameter* EmissiveParam =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	EmissiveParam->ParameterName = TEXT("EmissiveColor");
	EmissiveParam->DefaultValue = FLinearColor(1.f, 1.f, 1.f, 1.f);
	Mat->GetExpressionCollection().Expressions.Add(EmissiveParam);
	Mat->GetEditorOnlyData()->EmissiveColor.Connect(0, EmissiveParam);

	if (bAddBaseColor)
	{
		UMaterialExpressionVectorParameter* BaseParam =
			NewObject<UMaterialExpressionVectorParameter>(Mat);
		BaseParam->ParameterName = TEXT("BaseColor");
		BaseParam->DefaultValue = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
		Mat->GetExpressionCollection().Expressions.Add(BaseParam);
		Mat->GetEditorOnlyData()->BaseColor.Connect(0, BaseParam);

		UMaterialExpressionScalarParameter* MetallicParam =
			NewObject<UMaterialExpressionScalarParameter>(Mat);
		MetallicParam->ParameterName = TEXT("Metallic");
		MetallicParam->DefaultValue = 0.5f;
		Mat->GetExpressionCollection().Expressions.Add(MetallicParam);
		Mat->GetEditorOnlyData()->Metallic.Connect(0, MetallicParam);

		UMaterialExpressionScalarParameter* RoughnessParam =
			NewObject<UMaterialExpressionScalarParameter>(Mat);
		RoughnessParam->ParameterName = TEXT("Roughness");
		RoughnessParam->DefaultValue = 0.5f;
		Mat->GetExpressionCollection().Expressions.Add(RoughnessParam);
		Mat->GetEditorOnlyData()->Roughness.Connect(0, RoughnessParam);

		UMaterialExpressionScalarParameter* SpecularParam =
			NewObject<UMaterialExpressionScalarParameter>(Mat);
		SpecularParam->ParameterName = TEXT("Specular");
		SpecularParam->DefaultValue = 0.5f;
		Mat->GetExpressionCollection().Expressions.Add(SpecularParam);
		Mat->GetEditorOnlyData()->Specular.Connect(0, SpecularParam);
	}

	if (Blend != BLEND_Opaque)
	{
		UMaterialExpressionScalarParameter* OpacityParam =
			NewObject<UMaterialExpressionScalarParameter>(Mat);
		OpacityParam->ParameterName = TEXT("Opacity");
		OpacityParam->DefaultValue = 1.0f;
		Mat->GetExpressionCollection().Expressions.Add(OpacityParam);
		Mat->GetEditorOnlyData()->Opacity.Connect(0, OpacityParam);
	}

	Mat->PreEditChange(nullptr);
	Mat->PostEditChange();
	Mat->AddToRoot();
	return Mat;
}

/**
 * Builds a PBR material with world-position-tiled detail texture.
 * Graph: WorldPos / TilingScale → mask XY → TexSample → × BaseColor → BaseColor out
 * This breaks up flat-colored surfaces with natural stain/wear variation.
 */
static UMaterial* BuildTexturedMaterial()
{
	UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), TEXT("ExoLitTextured"));
	Mat->SetShadingModel(MSM_DefaultLit);
	Mat->BlendMode = BLEND_Opaque;
	Mat->TwoSided = true;

	// --- BaseColor parameter (tinted by detail texture) ---
	UMaterialExpressionVectorParameter* BaseParam =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	BaseParam->ParameterName = TEXT("BaseColor");
	BaseParam->DefaultValue = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	Mat->GetExpressionCollection().Expressions.Add(BaseParam);

	// --- World-position UVs for detail texture tiling ---
	UMaterialExpressionWorldPosition* WorldPos =
		NewObject<UMaterialExpressionWorldPosition>(Mat);
	Mat->GetExpressionCollection().Expressions.Add(WorldPos);

	UMaterialExpressionScalarParameter* TilingParam =
		NewObject<UMaterialExpressionScalarParameter>(Mat);
	TilingParam->ParameterName = TEXT("TilingScale");
	TilingParam->DefaultValue = 500.f;
	Mat->GetExpressionCollection().Expressions.Add(TilingParam);

	// WorldPos / TilingScale → tiled coordinates
	UMaterialExpressionDivide* DivNode =
		NewObject<UMaterialExpressionDivide>(Mat);
	DivNode->A.Connect(0, WorldPos);
	DivNode->B.Connect(0, TilingParam);
	Mat->GetExpressionCollection().Expressions.Add(DivNode);

	// Extract XY only → UV for texture sample
	UMaterialExpressionComponentMask* MaskXY =
		NewObject<UMaterialExpressionComponentMask>(Mat);
	MaskXY->R = 1;
	MaskXY->G = 1;
	MaskXY->B = 0;
	MaskXY->A = 0;
	MaskXY->Input.Connect(0, DivNode);
	Mat->GetExpressionCollection().Expressions.Add(MaskXY);

	// --- Detail texture sample ---
	UMaterialExpressionTextureSampleParameter2D* TexSample =
		NewObject<UMaterialExpressionTextureSampleParameter2D>(Mat);
	TexSample->ParameterName = TEXT("DetailTexture");
	TexSample->Coordinates.Connect(0, MaskXY);
	// Default: white texture (no modulation) — noise set per MID in SpawnStaticMesh
	TexSample->Texture = LoadObject<UTexture2D>(nullptr,
		TEXT("/Engine/EngineResources/WhiteSquareTexture"));
	Mat->GetExpressionCollection().Expressions.Add(TexSample);

	// BaseColor = BaseColorParam × DetailTexture.RGB
	UMaterialExpressionMultiply* ColorMul =
		NewObject<UMaterialExpressionMultiply>(Mat);
	ColorMul->A.Connect(0, BaseParam);
	ColorMul->B.Connect(0, TexSample);
	Mat->GetExpressionCollection().Expressions.Add(ColorMul);
	Mat->GetEditorOnlyData()->BaseColor.Connect(0, ColorMul);

	// --- Standard PBR scalar parameters ---
	UMaterialExpressionScalarParameter* MetallicParam =
		NewObject<UMaterialExpressionScalarParameter>(Mat);
	MetallicParam->ParameterName = TEXT("Metallic");
	MetallicParam->DefaultValue = 0.5f;
	Mat->GetExpressionCollection().Expressions.Add(MetallicParam);
	Mat->GetEditorOnlyData()->Metallic.Connect(0, MetallicParam);

	UMaterialExpressionScalarParameter* RoughnessParam =
		NewObject<UMaterialExpressionScalarParameter>(Mat);
	RoughnessParam->ParameterName = TEXT("Roughness");
	RoughnessParam->DefaultValue = 0.5f;
	Mat->GetExpressionCollection().Expressions.Add(RoughnessParam);

	// Roughness × DetailTexture.R — worn/stained areas become slightly smoother
	UMaterialExpressionMultiply* RoughMul =
		NewObject<UMaterialExpressionMultiply>(Mat);
	RoughMul->A.Connect(0, RoughnessParam);
	RoughMul->B.Connect(1, TexSample); // output 1 = R channel (scalar)
	Mat->GetExpressionCollection().Expressions.Add(RoughMul);
	Mat->GetEditorOnlyData()->Roughness.Connect(0, RoughMul);

	UMaterialExpressionScalarParameter* SpecularParam =
		NewObject<UMaterialExpressionScalarParameter>(Mat);
	SpecularParam->ParameterName = TEXT("Specular");
	SpecularParam->DefaultValue = 0.5f;
	Mat->GetExpressionCollection().Expressions.Add(SpecularParam);
	Mat->GetEditorOnlyData()->Specular.Connect(0, SpecularParam);

	// --- Normal map with world-position tiled UVs ---
	UMaterialExpressionTextureSampleParameter2D* NormalSample =
		NewObject<UMaterialExpressionTextureSampleParameter2D>(Mat);
	NormalSample->ParameterName = TEXT("NormalMap");
	NormalSample->SamplerType = SAMPLERTYPE_Normal;
	NormalSample->Coordinates.Connect(0, MaskXY);
	// Default: engine flat normal (no bump) — overridden per-MID in SpawnStaticMesh
	NormalSample->Texture = LoadObject<UTexture2D>(nullptr,
		TEXT("/Engine/EngineResources/DefaultNormal"));
	Mat->GetExpressionCollection().Expressions.Add(NormalSample);
	Mat->GetEditorOnlyData()->Normal.Connect(0, NormalSample);

	// --- Emissive with Fresnel edge highlight ---
	UMaterialExpressionVectorParameter* EmissiveParam =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	EmissiveParam->ParameterName = TEXT("EmissiveColor");
	EmissiveParam->DefaultValue = FLinearColor(0.f, 0.f, 0.f, 1.f);
	Mat->GetExpressionCollection().Expressions.Add(EmissiveParam);

	// Fresnel rim glow — subtle edge highlight for depth perception
	UMaterialExpressionFresnel* Fresnel =
		NewObject<UMaterialExpressionFresnel>(Mat);
	Fresnel->Exponent = 4.f;
	Fresnel->BaseReflectFraction = 0.01f;
	Mat->GetExpressionCollection().Expressions.Add(Fresnel);

	UMaterialExpressionVectorParameter* FresnelColorParam =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	FresnelColorParam->ParameterName = TEXT("FresnelColor");
	FresnelColorParam->DefaultValue = FLinearColor(0.f, 0.f, 0.f, 1.f);
	Mat->GetExpressionCollection().Expressions.Add(FresnelColorParam);

	UMaterialExpressionMultiply* FresnelMul =
		NewObject<UMaterialExpressionMultiply>(Mat);
	FresnelMul->A.Connect(0, Fresnel);
	FresnelMul->B.Connect(0, FresnelColorParam);
	Mat->GetExpressionCollection().Expressions.Add(FresnelMul);

	UMaterialExpressionAdd* EmissiveSum =
		NewObject<UMaterialExpressionAdd>(Mat);
	EmissiveSum->A.Connect(0, EmissiveParam);
	EmissiveSum->B.Connect(0, FresnelMul);
	Mat->GetExpressionCollection().Expressions.Add(EmissiveSum);
	Mat->GetEditorOnlyData()->EmissiveColor.Connect(0, EmissiveSum);

	Mat->PreEditChange(nullptr);
	Mat->PostEditChange();
	Mat->AddToRoot();
	return Mat;
}

UMaterialInterface* FExoMaterialFactory::GetEmissiveAdditive()
{
	if (CachedAdditive) return CachedAdditive;
	CachedAdditive = BuildRuntimeMaterial(BLEND_Additive, MSM_Unlit,
		TEXT("ExoEmissiveAdditive"), false);
	return CachedAdditive;
}

UMaterialInterface* FExoMaterialFactory::GetEmissiveOpaque()
{
	if (CachedOpaque) return CachedOpaque;
	CachedOpaque = BuildRuntimeMaterial(BLEND_Opaque, MSM_Unlit,
		TEXT("ExoEmissiveOpaque"), false);
	return CachedOpaque;
}

UMaterialInterface* FExoMaterialFactory::GetLitEmissive()
{
	if (CachedLitEmissive) return CachedLitEmissive;
	CachedLitEmissive = BuildRuntimeMaterial(BLEND_Opaque, MSM_DefaultLit,
		TEXT("ExoLitEmissive"), true);
	return CachedLitEmissive;
}

UMaterialInterface* FExoMaterialFactory::GetLitTextured()
{
	if (CachedLitTextured) return CachedLitTextured;
	CachedLitTextured = BuildTexturedMaterial();
	return CachedLitTextured;
}

UMaterialInterface* FExoMaterialFactory::GetGlassTranslucent()
{
	if (CachedGlass) return CachedGlass;
	CachedGlass = BuildRuntimeMaterial(BLEND_Translucent, MSM_DefaultLit,
		TEXT("ExoGlassTranslucent"), true);
	return CachedGlass;
}
