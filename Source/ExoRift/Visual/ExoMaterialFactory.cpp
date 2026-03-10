// ExoMaterialFactory.cpp — Runtime emissive material creation
#include "Visual/ExoMaterialFactory.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/Package.h"

#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#endif

static UMaterialInterface* CachedAdditive = nullptr;
static UMaterialInterface* CachedOpaque = nullptr;
static UMaterialInterface* CachedLitEmissive = nullptr;

static UMaterialInterface* GetFallbackMaterial()
{
	static UMaterialInterface* Fallback = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	return Fallback;
}

#if WITH_EDITOR
static UMaterial* BuildRuntimeMaterial(EBlendMode Blend, EMaterialShadingModel Shading,
	const TCHAR* Name, bool bAddBaseColor)
{
	UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), Name);
	Mat->SetShadingModel(Shading);
	Mat->BlendMode = Blend;
	Mat->TwoSided = true;

	// Emissive color parameter — THE key feature missing from BasicShapeMaterial
	UMaterialExpressionVectorParameter* EmissiveParam =
		NewObject<UMaterialExpressionVectorParameter>(Mat);
	EmissiveParam->ParameterName = TEXT("EmissiveColor");
	EmissiveParam->DefaultValue = FLinearColor(1.f, 1.f, 1.f, 1.f);
	Mat->GetExpressionCollection().Expressions.Add(EmissiveParam);
	Mat->GetEditorOnlyData()->EmissiveColor.Connect(0, EmissiveParam);

	// BaseColor for lit materials (PBR diffuse + emissive glow)
	if (bAddBaseColor)
	{
		UMaterialExpressionVectorParameter* BaseParam =
			NewObject<UMaterialExpressionVectorParameter>(Mat);
		BaseParam->ParameterName = TEXT("BaseColor");
		BaseParam->DefaultValue = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
		Mat->GetExpressionCollection().Expressions.Add(BaseParam);
		Mat->GetEditorOnlyData()->BaseColor.Connect(0, BaseParam);

		// Metallic + Roughness for PBR surfaces
		UMaterialExpressionScalarParameter* MetallicParam =
			NewObject<UMaterialExpressionScalarParameter>(Mat);
		MetallicParam->ParameterName = TEXT("Metallic");
		MetallicParam->DefaultValue = 0.85f;
		Mat->GetExpressionCollection().Expressions.Add(MetallicParam);
		Mat->GetEditorOnlyData()->Metallic.Connect(0, MetallicParam);

		UMaterialExpressionScalarParameter* RoughnessParam =
			NewObject<UMaterialExpressionScalarParameter>(Mat);
		RoughnessParam->ParameterName = TEXT("Roughness");
		RoughnessParam->DefaultValue = 0.25f;
		Mat->GetExpressionCollection().Expressions.Add(RoughnessParam);
		Mat->GetEditorOnlyData()->Roughness.Connect(0, RoughnessParam);
	}

	// Opacity for non-opaque blends (fading support)
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
	Mat->AddToRoot(); // Prevent GC
	return Mat;
}
#endif

UMaterialInterface* FExoMaterialFactory::GetEmissiveAdditive()
{
	if (CachedAdditive) return CachedAdditive;
#if WITH_EDITOR
	CachedAdditive = BuildRuntimeMaterial(BLEND_Additive, MSM_Unlit,
		TEXT("ExoEmissiveAdditive"), false);
	if (CachedAdditive) return CachedAdditive;
#endif
	CachedAdditive = GetFallbackMaterial();
	return CachedAdditive;
}

UMaterialInterface* FExoMaterialFactory::GetEmissiveOpaque()
{
	if (CachedOpaque) return CachedOpaque;
#if WITH_EDITOR
	CachedOpaque = BuildRuntimeMaterial(BLEND_Opaque, MSM_Unlit,
		TEXT("ExoEmissiveOpaque"), false);
	if (CachedOpaque) return CachedOpaque;
#endif
	CachedOpaque = GetFallbackMaterial();
	return CachedOpaque;
}

UMaterialInterface* FExoMaterialFactory::GetLitEmissive()
{
	if (CachedLitEmissive) return CachedLitEmissive;
#if WITH_EDITOR
	CachedLitEmissive = BuildRuntimeMaterial(BLEND_Opaque, MSM_DefaultLit,
		TEXT("ExoLitEmissive"), true);
	if (CachedLitEmissive) return CachedLitEmissive;
#endif
	CachedLitEmissive = GetFallbackMaterial();
	return CachedLitEmissive;
}
