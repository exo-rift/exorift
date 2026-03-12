// ExoLevelBuilder.cpp — Terrain, lighting, skybox, and mesh helper
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Visual/ExoAmbientParticles.h"
#include "Visual/ExoEnvironmentAnimator.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoProceduralTextures.h"
#include "Engine/Texture2D.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

AExoLevelBuilder::AExoLevelBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Cache engine meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphFind.Succeeded()) SphereMesh = SphFind.Object;

}

void AExoLevelBuilder::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Generating level..."));

	// Destroy pre-existing lighting actors from the template level to prevent
	// competing lights (ExoLevelBuilder creates its own full lighting setup)
	{
		TArray<AActor*> ToDestroy;
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(this, ADirectionalLight::StaticClass(), Found);
		ToDestroy.Append(Found);
		Found.Reset();
		UGameplayStatics::GetAllActorsOfClass(this, ASkyLight::StaticClass(), Found);
		ToDestroy.Append(Found);
		Found.Reset();
		UGameplayStatics::GetAllActorsOfClass(this, AExponentialHeightFog::StaticClass(), Found);
		ToDestroy.Append(Found);
		for (AActor* A : ToDestroy)
		{
			if (A && A != this) A->Destroy();
		}
		UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Cleaned up %d template lighting actors"), ToDestroy.Num());
	}

	ScanMarketplaceAssets();
	BuildTerrain();
	BuildLighting();
	BuildSkybox();
	BuildStructures();
	PlaceSpawnPoints();
	PlaceLootSpawners();
	PlaceHazardZones();
	PlaceExplodingBarrels();
	PlaceCoverElements();
	BuildProps();
	BuildEnvironmentalDebris();
	BuildRoads();
	BuildWaterFeatures();
	BuildFoliage();
	BuildGroundDetail();
	BuildInteriors();
	BuildSignage();
	BuildAtmosphere();
	BuildCatwalks();
	BuildTunnels();
	BuildCompoundLighting();
	BuildFieldCover();
	PlaceJumpPads();
	PlaceDrones();
	PlaceSteamVents();

	// Ambient floating particles (dust motes / energy wisps)
	AExoAmbientParticles* Particles = AExoAmbientParticles::Get(GetWorld());
	if (Particles) Particles->SetStyle(true); // Energy wisps for sci-fi

	BuildStorytelling();
	PlacePOIs();

	// Spawn the environment animator for pylon rotation, console flicker, etc.
	{
		FActorSpawnParameters AnimParams;
		AnimParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AExoEnvironmentAnimator>(
			AExoEnvironmentAnimator::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, AnimParams);
	}

	UE_LOG(LogExoRift, Log, TEXT("ExoLevelBuilder: Level complete — %d mesh components, ready."),
		LevelMeshes.Num());

	// Auto-screenshot sequence: multi-angle captures for visual review
	// Use -ExoScreenshot on command line to enable
	if (FParse::Param(FCommandLine::Get(), TEXT("ExoScreenshot")))
	{
		struct FCameraShot { FVector Pos; FRotator Rot; float Time; const TCHAR* Name; };
		TArray<FCameraShot> Shots = {
			{{3000.f, 0.f, 1500.f},   {-8.f, -170.f, 0.f},  4.f, TEXT("01_StreetLevel")},
			{{0.f, 8000.f, 3000.f},    {-20.f, -90.f, 0.f},  7.f, TEXT("02_CompoundView")},
			{{-5000.f, -5000.f, 800.f},{-5.f, 45.f, 0.f},   10.f, TEXT("03_RoadLevel")},
			{{0.f, 0.f, 15000.f},      {-60.f, 0.f, 0.f},   13.f, TEXT("04_Aerial")},
			{{12000.f, 12000.f, 600.f},{-3.f, -135.f, 0.f},  16.f, TEXT("05_Outskirts")},
		};

		for (const FCameraShot& Shot : Shots)
		{
			// Move camera 1s before capture
			FTimerHandle CamHandle;
			FVector ShotPos = Shot.Pos;
			FRotator ShotRot = Shot.Rot;
			GetWorldTimerManager().SetTimer(CamHandle, [this, ShotPos, ShotRot]()
			{
				APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
				APawn* P = PC ? PC->GetPawn() : nullptr;
				if (P)
				{
					P->SetActorLocation(ShotPos);
					P->SetActorEnableCollision(false);
					PC->SetControlRotation(ShotRot);
				}
			}, Shot.Time, false);

			// Capture 1.5s after camera move
			FTimerHandle CapHandle;
			FString ShotName = Shot.Name;
			GetWorldTimerManager().SetTimer(CapHandle, [this, ShotName]()
			{
				FString Dir = FPaths::ProjectSavedDir() / TEXT("Screenshots");
				IFileManager::Get().MakeDirectory(*Dir, true);
				FString Path = Dir / FString::Printf(TEXT("ExoRift_%s.png"), *ShotName);
				FScreenshotRequest::RequestScreenshot(Path, false, false);
				UE_LOG(LogExoRift, Log, TEXT("Screenshot: %s"), *Path);
			}, Shot.Time + 1.5f, false);
		}
	}
}

// BuildTerrain is in ExoLevelBuilderTerrain.cpp

void AExoLevelBuilder::BuildLighting()
{
	// === SKY ATMOSPHERE — realistic atmospheric scattering ===
	USkyAtmosphereComponent* Atmos = NewObject<USkyAtmosphereComponent>(this);
	Atmos->SetupAttachment(RootComponent);
	Atmos->RegisterComponent();
	// Alien world: slightly thicker atmosphere, orange-tinted sunset horizon
	Atmos->BottomRadius = 6360.f;
	Atmos->AtmosphereHeight = 80.f;
	Atmos->MultiScatteringFactor = 1.f;
	// Rayleigh: balanced sky — not oversaturated blue
	Atmos->RayleighScattering = FLinearColor(0.058f, 0.1f, 0.22f);
	Atmos->RayleighExponentialDistribution = 8.f;
	// Mie: slight warm haze near horizon
	Atmos->MieScattering = FLinearColor(0.004f, 0.004f, 0.004f);
	Atmos->MieAbsorption = FLinearColor(0.004f, 0.004f, 0.004f);
	Atmos->MieAnisotropy = 0.8f;
	Atmos->MieExponentialDistribution = 1.2f;
	// Ground albedo — warm rocky surface (brighter = more bounce light)
	Atmos->GroundAlbedo = FColor(55, 50, 42);

	// === PRIMARY SUN — low angle for dramatic long shadows ===
	UDirectionalLightComponent* Sun = NewObject<UDirectionalLightComponent>(this);
	Sun->SetupAttachment(RootComponent);
	Sun->RegisterComponent();
	Sun->SetWorldRotation(FRotator(-18.f, 220.f, 0.f));
	Sun->SetIntensity(12.f); // Bright sun for proper PBR response + shadow definition
	Sun->SetLightColor(FLinearColor(1.0f, 0.9f, 0.75f));
	Sun->CastShadows = true;
	Sun->SetAtmosphereSunLight(true); // Link sun to atmosphere for scattering
	Sun->SetAtmosphereSunLightIndex(0);

	// Fill light — opposite direction, cool blue, softer
	UDirectionalLightComponent* Fill = NewObject<UDirectionalLightComponent>(this);
	Fill->SetupAttachment(RootComponent);
	Fill->RegisterComponent();
	Fill->SetWorldRotation(FRotator(-55.f, 40.f, 0.f));
	Fill->SetIntensity(1.5f);
	Fill->SetLightColor(FLinearColor(0.45f, 0.55f, 0.85f));
	Fill->CastShadows = false;

	// === SKY LIGHT — ambient from atmosphere + cubemap capture ===
	USkyLightComponent* Sky = NewObject<USkyLightComponent>(this);
	Sky->SetupAttachment(RootComponent);
	Sky->SetRealTimeCapture(true); // Captures atmosphere for ambient lighting
	Sky->RegisterComponent();
	Sky->SetIntensity(4.5f);
	Sky->SetLightColor(FLinearColor(0.75f, 0.77f, 0.85f));

	// === VOLUMETRIC CLOUDS — realistic cloud layer ===
	UVolumetricCloudComponent* Clouds = NewObject<UVolumetricCloudComponent>(this);
	Clouds->SetupAttachment(RootComponent);
	Clouds->RegisterComponent();
	Clouds->LayerBottomAltitude = 5.f;
	Clouds->LayerHeight = 8.f;
	Clouds->TracingStartMaxDistance = 350.f;
	Clouds->TracingMaxDistance = 50.f;

	// === EXPONENTIAL HEIGHT FOG — atmospheric depth ===
	UExponentialHeightFogComponent* Fog = NewObject<UExponentialHeightFogComponent>(this);
	Fog->SetupAttachment(RootComponent);
	Fog->RegisterComponent();
	Fog->SetFogDensity(0.0004f);
	Fog->SetFogHeightFalloff(0.1f);
	Fog->SetFogInscatteringColor(FLinearColor(0.04f, 0.05f, 0.08f));
	Fog->SetDirectionalInscatteringColor(FLinearColor(0.2f, 0.15f, 0.08f));
	Fog->SetFogMaxOpacity(0.5f);
	Fog->SetStartDistance(8000.f);
	Fog->SetVolumetricFog(true);
	Fog->SetVolumetricFogScatteringDistribution(0.3f);
	Fog->SetVolumetricFogExtinctionScale(0.5f);

	// Second fog layer — thin ground haze for depth
	UExponentialHeightFogComponent* GroundFog = NewObject<UExponentialHeightFogComponent>(this);
	GroundFog->SetupAttachment(RootComponent);
	GroundFog->RegisterComponent();
	GroundFog->SetFogDensity(0.001f);
	GroundFog->SetFogHeightFalloff(0.5f);
	GroundFog->SetFogInscatteringColor(FLinearColor(0.02f, 0.03f, 0.05f));
	GroundFog->SetFogMaxOpacity(0.2f);
	GroundFog->SetRelativeLocation(FVector(0.f, 0.f, -200.f));

	// === POST-PROCESS — realistic cinematic look ===
	UPostProcessComponent* PP = NewObject<UPostProcessComponent>(this);
	PP->SetupAttachment(RootComponent);
	PP->bUnbound = true;
	PP->Priority = 0.f;

	// Auto-exposure: slow adaptation for dramatic lighting changes
	PP->Settings.bOverride_AutoExposureBias = true;
	PP->Settings.AutoExposureBias = 1.2f;
	PP->Settings.bOverride_AutoExposureMinBrightness = true;
	PP->Settings.AutoExposureMinBrightness = 0.1f;
	PP->Settings.bOverride_AutoExposureMaxBrightness = true;
	PP->Settings.AutoExposureMaxBrightness = 6.f;
	PP->Settings.bOverride_AutoExposureSpeedUp = true;
	PP->Settings.AutoExposureSpeedUp = 2.f;
	PP->Settings.bOverride_AutoExposureSpeedDown = true;
	PP->Settings.AutoExposureSpeedDown = 1.f;

	// Bloom: moderate for realistic glow, not the washed-out look
	PP->Settings.bOverride_BloomIntensity = true;
	PP->Settings.BloomIntensity = 0.8f;
	PP->Settings.bOverride_BloomThreshold = true;
	PP->Settings.BloomThreshold = 1.0f; // Only bright things bloom
	PP->Settings.bOverride_BloomMethod = true;
	PP->Settings.BloomMethod = EBloomMethod::BM_SOG;

	// Vignette — subtle cinematic framing
	PP->Settings.bOverride_VignetteIntensity = true;
	PP->Settings.VignetteIntensity = 0.25f;

	// Film grain — subtle
	PP->Settings.bOverride_FilmGrainIntensity = true;
	PP->Settings.FilmGrainIntensity = 0.015f;

	// Color grading — cooler shadows, warm highlights for sci-fi
	PP->Settings.bOverride_ColorGamma = true;
	PP->Settings.ColorGamma = FVector4(0.97f, 0.98f, 1.04f, 1.f);
	PP->Settings.bOverride_ColorGain = true;
	PP->Settings.ColorGain = FVector4(0.96f, 0.98f, 1.05f, 1.f);
	PP->Settings.bOverride_ColorSaturation = true;
	PP->Settings.ColorSaturation = FVector4(1.08f, 1.08f, 1.08f, 1.f);
	PP->Settings.bOverride_ColorContrastShadows = true;
	PP->Settings.ColorContrastShadows = FVector4(1.05f, 1.05f, 1.1f, 1.f);
	PP->Settings.bOverride_ColorGainHighlights = true;
	PP->Settings.ColorGainHighlights = FVector4(1.03f, 1.0f, 0.95f, 1.f);

	// Ambient occlusion — strong for depth
	PP->Settings.bOverride_AmbientOcclusionIntensity = true;
	PP->Settings.AmbientOcclusionIntensity = 0.8f;
	PP->Settings.bOverride_AmbientOcclusionRadius = true;
	PP->Settings.AmbientOcclusionRadius = 300.f;

	// Screen-space reflections — visible on metallic surfaces
	PP->Settings.bOverride_ScreenSpaceReflectionIntensity = true;
	PP->Settings.ScreenSpaceReflectionIntensity = 80.f;
	PP->Settings.bOverride_ScreenSpaceReflectionQuality = true;
	PP->Settings.ScreenSpaceReflectionQuality = 80.f;
	PP->Settings.bOverride_ScreenSpaceReflectionMaxRoughness = true;
	PP->Settings.ScreenSpaceReflectionMaxRoughness = 0.6f;

	// Lens flare — subtle on bright lights
	PP->Settings.bOverride_LensFlareIntensity = true;
	PP->Settings.LensFlareIntensity = 0.2f;
	PP->Settings.bOverride_LensFlareTint = true;
	PP->Settings.LensFlareTint = FLinearColor(0.8f, 0.85f, 1.f);
	PP->Settings.bOverride_LensFlareThreshold = true;
	PP->Settings.LensFlareThreshold = 6.f;

	// Chromatic aberration — subtle lens imperfection
	PP->Settings.bOverride_SceneFringeIntensity = true;
	PP->Settings.SceneFringeIntensity = 0.15f;

	// Motion blur for cinematic feel
	PP->Settings.bOverride_MotionBlurAmount = true;
	PP->Settings.MotionBlurAmount = 0.3f;
	PP->Settings.bOverride_MotionBlurMax = true;
	PP->Settings.MotionBlurMax = 3.f;

	PP->RegisterComponent();

	// Nav mesh bounds for bot navigation
	FActorSpawnParameters NavParams;
	NavParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ANavMeshBoundsVolume* NavBounds = GetWorld()->SpawnActor<ANavMeshBoundsVolume>(
		ANavMeshBoundsVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, NavParams);
	if (NavBounds)
	{
		NavBounds->SetActorScale3D(FVector(MapHalfSize / 100.f, MapHalfSize / 100.f, 500.f));
	}
}

UStaticMeshComponent* AExoLevelBuilder::SpawnStaticMesh(const FVector& Location,
	const FVector& Scale, const FRotator& Rotation, UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	Comp->SetupAttachment(RootComponent);
	Comp->SetStaticMesh(Mesh);
	Comp->SetWorldLocation(Location);
	Comp->SetWorldScale3D(Scale);
	Comp->SetWorldRotation(Rotation);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->CastShadow = true;
	Comp->RegisterComponent();

	// Classify surface by authored color and geometry
	float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
	bool bIsLargeGround = (Scale.X > 100.f || Scale.Y > 100.f) && Scale.Z < 5.f;
	bool bIsTerrain = (Lum < 0.06f) || bIsLargeGround;
	bool bIsStructural = !bIsTerrain && Lum < 0.15f;

	float Metallic, Roughness, Specular;
	FLinearColor AdjustedColor = Color;

	// Position-based hash for deterministic per-instance variation
	float PosHash = FMath::Abs(FMath::Sin(Location.X * 0.0001f + Location.Y * 0.00013f
		+ Location.Z * 0.00017f));
	float PosHash2 = FMath::Abs(FMath::Sin(Location.X * 0.00023f + Location.Z * 0.00031f));

	// Boost authored colors to realistic PBR albedo range
	// Terrain: 3.5x → concrete/asphalt albedo 0.2-0.35
	// Structural: 3.0x → weathered metal 0.25-0.45
	// Accent: 2.5x → polished metal/paint 0.3-0.6+
	if (bIsTerrain)
	{
		float Boost = 3.5f;
		Metallic = 0.02f;
		Roughness = 0.78f + PosHash * 0.15f;
		Specular = 0.35f;
		// Warm earth tones — shift towards sandy/brown, not cold grey
		float WarmShift = PosHash * 0.04f;
		float EarthTint = PosHash2 * 0.03f;
		AdjustedColor = FLinearColor(
			FMath::Clamp(Color.R * Boost + WarmShift + EarthTint, 0.15f, 0.45f),
			FMath::Clamp(Color.G * Boost + WarmShift * 0.6f, 0.14f, 0.4f),
			FMath::Clamp(Color.B * Boost - WarmShift * 0.3f, 0.1f, 0.35f));
	}
	else if (bIsStructural)
	{
		float Boost = 3.0f;
		Metallic = 0.35f + PosHash * 0.3f;
		Roughness = 0.35f + PosHash * 0.25f;
		Specular = 0.5f;
		// Weathered industrial look — slight color variation per piece
		float Weather = PosHash * 0.03f;
		float Rust = PosHash2 * 0.02f;
		AdjustedColor = FLinearColor(
			FMath::Clamp(Color.R * Boost - Weather + Rust, 0.15f, 0.55f),
			FMath::Clamp(Color.G * Boost - Weather, 0.14f, 0.5f),
			FMath::Clamp(Color.B * Boost - Weather * 0.5f, 0.13f, 0.5f));
	}
	else
	{
		float Boost = 2.5f;
		Metallic = 0.7f + PosHash * 0.15f;
		Roughness = 0.15f + PosHash * 0.12f;
		Specular = 0.6f;
		AdjustedColor = FLinearColor(
			FMath::Clamp(Color.R * Boost, 0.1f, 0.9f),
			FMath::Clamp(Color.G * Boost, 0.1f, 0.9f),
			FMath::Clamp(Color.B * Boost, 0.1f, 0.9f));
	}

	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitTextured();
	if (LitMat)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!DynMat) { LevelMeshes.Add(Comp); return Comp; }
		DynMat->SetVectorParameterValue(TEXT("BaseColor"), AdjustedColor);
		DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		DynMat->SetScalarParameterValue(TEXT("Metallic"), Metallic);
		DynMat->SetScalarParameterValue(TEXT("Roughness"), Roughness);
		DynMat->SetScalarParameterValue(TEXT("Specular"), Specular);

		// Detail + normal textures: terrain=broad ground, structural=fine metal
		UTexture2D* DetailTex = bIsTerrain
			? FExoProceduralTextures::GetGroundNoise()
			: FExoProceduralTextures::GetMetalNoise();
		if (DetailTex)
			DynMat->SetTextureParameterValue(TEXT("DetailTexture"), DetailTex);

		UTexture2D* NormalTex = bIsTerrain
			? FExoProceduralTextures::GetGroundNormal()
			: FExoProceduralTextures::GetMetalNormal();
		if (NormalTex)
			DynMat->SetTextureParameterValue(TEXT("NormalMap"), NormalTex);

		DynMat->SetScalarParameterValue(TEXT("TilingScale"), bIsTerrain ? 600.f : 300.f);

		// Fresnel edge highlight for structural/accent surfaces
		FLinearColor FresnelCol = FLinearColor::Black;
		if (bIsStructural)
			FresnelCol = FLinearColor(0.015f, 0.025f, 0.04f);
		else if (!bIsTerrain)
			FresnelCol = FLinearColor(0.03f, 0.05f, 0.08f);
		DynMat->SetVectorParameterValue(TEXT("FresnelColor"), FresnelCol);
		Comp->SetMaterial(0, DynMat);
	}

	LevelMeshes.Add(Comp);
	return Comp;
}

UStaticMeshComponent* AExoLevelBuilder::SpawnRawMesh(const FVector& Location,
	const FVector& Scale, const FRotator& Rotation, UStaticMesh* Mesh)
{
	if (!Mesh) return nullptr;

	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	Comp->SetupAttachment(RootComponent);
	Comp->SetStaticMesh(Mesh);
	Comp->SetWorldLocation(Location);
	Comp->SetWorldScale3D(Scale);
	Comp->SetWorldRotation(Rotation);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	Comp->CastShadow = true;
	Comp->RegisterComponent();
	LevelMeshes.Add(Comp);
	return Comp;
}

void AExoLevelBuilder::ScanMarketplaceAssets()
{
	// Try loading assets from known Fab/Megascans/Paragon paths.
	// If a pack isn't installed, LoadObject silently returns nullptr — no crash.
	// When found, the asset is bound for use by the level generator.

	struct FAssetProbe { const TCHAR* Path; const TCHAR* Name; };

	// --- Materials ---
	TArray<FAssetProbe> GroundMats = {
		{TEXT("/Game/Megascans/Surfaces/Concrete_Rough/MI_Concrete_Rough_inst"), TEXT("Megascans Concrete")},
		{TEXT("/Game/Megascans/Surfaces/Asphalt_Wet/MI_Asphalt_Wet_inst"), TEXT("Megascans Asphalt")},
		{TEXT("/Game/StarterContent/Materials/M_Ground_Gravel"), TEXT("StarterContent Gravel")},
	};
	for (const auto& P : GroundMats)
	{
		if (auto* Mat = LoadObject<UMaterialInterface>(nullptr, P.Path))
		{
			GroundMaterial = Mat;
			UE_LOG(LogExoRift, Log, TEXT("MarketplaceAssets: Found ground material — %s"), P.Name);
			break;
		}
	}

	TArray<FAssetProbe> WallMats = {
		{TEXT("/Game/Megascans/Surfaces/Concrete_Wall/MI_Concrete_Wall_inst"), TEXT("Megascans Wall")},
		{TEXT("/Game/StarterContent/Materials/M_Brick_Clay_New"), TEXT("StarterContent Brick")},
	};
	for (const auto& P : WallMats)
	{
		if (auto* Mat = LoadObject<UMaterialInterface>(nullptr, P.Path))
		{
			ConcreteWallMaterial = Mat;
			UE_LOG(LogExoRift, Log, TEXT("MarketplaceAssets: Found wall material — %s"), P.Name);
			break;
		}
	}

	TArray<FAssetProbe> MetalMats = {
		{TEXT("/Game/Megascans/Surfaces/Metal_Panel/MI_Metal_Panel_inst"), TEXT("Megascans Metal")},
		{TEXT("/Game/StarterContent/Materials/M_Metal_Burnished_Steel"), TEXT("StarterContent Steel")},
	};
	for (const auto& P : MetalMats)
	{
		if (auto* Mat = LoadObject<UMaterialInterface>(nullptr, P.Path))
		{
			MetalPanelMaterial = Mat;
			UE_LOG(LogExoRift, Log, TEXT("MarketplaceAssets: Found metal material — %s"), P.Name);
			break;
		}
	}

	// --- Meshes ---
	TArray<FAssetProbe> Rocks = {
		{TEXT("/Game/Megascans/3D_Assets/Rock_01/SM_Rock_01"), TEXT("Megascans Rock")},
		{TEXT("/Game/StarterContent/Props/SM_Rock"), TEXT("StarterContent Rock")},
	};
	for (const auto& P : Rocks)
	{
		if (auto* Mesh = LoadObject<UStaticMesh>(nullptr, P.Path))
		{
			RockMesh = Mesh;
			UE_LOG(LogExoRift, Log, TEXT("MarketplaceAssets: Found rock mesh — %s"), P.Name);
			break;
		}
	}

	int32 Found = (GroundMaterial.Get() ? 1 : 0) + (ConcreteWallMaterial.Get() ? 1 : 0)
		+ (MetalPanelMaterial.Get() ? 1 : 0) + (RockMesh.Get() ? 1 : 0);
	UE_LOG(LogExoRift, Log, TEXT("MarketplaceAssets: %d/%d asset slots filled."), Found, 6);

	// --- KayKit Space Base meshes ---
	struct FKKProbe { UStaticMesh** Target; const TCHAR* Path; const TCHAR* Name; };
	TArray<FKKProbe> KKProbes = {
		{&KK_BaseModule,      TEXT("/Game/KayKit/SpaceBase/basemodule_A"),       TEXT("BaseModule")},
		{&KK_BaseGarage,      TEXT("/Game/KayKit/SpaceBase/basemodule_garage"),  TEXT("BaseGarage")},
		{&KK_CargoDepot,      TEXT("/Game/KayKit/SpaceBase/cargodepot_A"),       TEXT("CargoDepot")},
		{&KK_Cargo,           TEXT("/Game/KayKit/SpaceBase/cargo_A"),            TEXT("Cargo")},
		{&KK_Container,       TEXT("/Game/KayKit/SpaceBase/containers_A"),       TEXT("Container")},
		{&KK_StructureTall,   TEXT("/Game/KayKit/SpaceBase/structure_tall"),      TEXT("StructureTall")},
		{&KK_StructureLow,    TEXT("/Game/KayKit/SpaceBase/structure_low"),       TEXT("StructureLow")},
		{&KK_RoofModule,      TEXT("/Game/KayKit/SpaceBase/roofmodule_base"),     TEXT("RoofModule")},
		{&KK_SolarPanel,      TEXT("/Game/KayKit/SpaceBase/solarpanel"),          TEXT("SolarPanel")},
		{&KK_DrillStructure,  TEXT("/Game/KayKit/SpaceBase/drill_structure"),     TEXT("DrillStructure")},
		{&KK_LandingPadLarge, TEXT("/Game/KayKit/SpaceBase/landingpad_large"),    TEXT("LandingPad")},
		{&KK_Tunnel,          TEXT("/Game/KayKit/SpaceBase/tunnel_straight_A"),   TEXT("Tunnel")},
		{&KK_Rock,            TEXT("/Game/KayKit/SpaceBase/rock_A"),              TEXT("Rock")},
		{&KK_TerrainLow,      TEXT("/Game/KayKit/SpaceBase/terrain_low"),         TEXT("TerrainLow")},
		{&KK_TerrainTall,     TEXT("/Game/KayKit/SpaceBase/terrain_tall"),        TEXT("TerrainTall")},
		{&KK_WindTurbine,     TEXT("/Game/KayKit/SpaceBase/windturbine_tall"),    TEXT("WindTurbine")},
		{&KK_Lander,          TEXT("/Game/KayKit/SpaceBase/lander_A"),            TEXT("Lander")},
		{&KK_SpaceTruck,      TEXT("/Game/KayKit/SpaceBase/spacetruck"),          TEXT("SpaceTruck")},
		{&KK_Lights,          TEXT("/Game/KayKit/SpaceBase/lights"),              TEXT("Lights")},
	};
	int32 KKFound = 0;
	for (const auto& P : KKProbes)
	{
		if (auto* Mesh = LoadObject<UStaticMesh>(nullptr, P.Path))
		{
			*P.Target = Mesh;
			KKFound++;
		}
	}
	bHasKayKitAssets = (KKFound > 0);
	UE_LOG(LogExoRift, Log, TEXT("KayKit SpaceBase: %d/%d meshes loaded.%s"),
		KKFound, KKProbes.Num(),
		bHasKayKitAssets ? TEXT(" Using real meshes!") : TEXT(" Not found — using primitives."));

	// --- Kenney Space Kit meshes ---
	struct FKNProbe { UStaticMesh** Target; const TCHAR* Path; };
	const TCHAR* KNBase = TEXT("/Game/Meshes/Kenney_SpaceKit/");
	TArray<FKNProbe> KNProbes = {
		{&KN_Corridor,             TEXT("/Game/Meshes/Kenney_SpaceKit/corridor")},
		{&KN_CorridorCorner,       TEXT("/Game/Meshes/Kenney_SpaceKit/corridor-corner")},
		{&KN_CorridorWide,         TEXT("/Game/Meshes/Kenney_SpaceKit/corridor-wide")},
		{&KN_CorridorWideCorner,   TEXT("/Game/Meshes/Kenney_SpaceKit/corridor-wide-corner")},
		{&KN_CorridorIntersection, TEXT("/Game/Meshes/Kenney_SpaceKit/corridor-intersection")},
		{&KN_RoomSmall,            TEXT("/Game/Meshes/Kenney_SpaceKit/room-small")},
		{&KN_RoomLarge,            TEXT("/Game/Meshes/Kenney_SpaceKit/room-large")},
		{&KN_RoomWide,             TEXT("/Game/Meshes/Kenney_SpaceKit/room-wide")},
		{&KN_Gate,                 TEXT("/Game/Meshes/Kenney_SpaceKit/gate")},
		{&KN_GateDoor,             TEXT("/Game/Meshes/Kenney_SpaceKit/gate-door")},
		{&KN_GateLasers,           TEXT("/Game/Meshes/Kenney_SpaceKit/gate-lasers")},
		{&KN_Stairs,               TEXT("/Game/Meshes/Kenney_SpaceKit/stairs")},
		{&KN_StairsWide,           TEXT("/Game/Meshes/Kenney_SpaceKit/stairs-wide")},
		{&KN_Door,                 TEXT("/Game/Meshes/Kenney_SpaceKit/door")},
		{&KN_Cables,               TEXT("/Game/Meshes/Kenney_SpaceKit/cables")},
	};
	int32 KNFound = 0;
	for (const auto& P : KNProbes)
	{
		if (auto* Mesh = LoadObject<UStaticMesh>(nullptr, P.Path))
		{
			*P.Target = Mesh;
			KNFound++;
		}
	}
	bHasKenneyAssets = (KNFound > 0);
	UE_LOG(LogExoRift, Log, TEXT("Kenney SpaceKit: %d/%d meshes loaded.%s"),
		KNFound, KNProbes.Num(),
		bHasKenneyAssets ? TEXT(" Using real meshes!") : TEXT(" Not found."));

	// --- Quaternius Sci-Fi meshes ---
	struct FQTProbe { UStaticMesh** Target; const TCHAR* Path; };
	TArray<FQTProbe> QTProbes = {
		{&QT_Column1,           TEXT("/Game/Meshes/Quaternius_SciFi/Column_1")},
		{&QT_Column2,           TEXT("/Game/Meshes/Quaternius_SciFi/Column_2")},
		{&QT_ColumnSlim,        TEXT("/Game/Meshes/Quaternius_SciFi/Column_Slim")},
		{&QT_DoorSingle,        TEXT("/Game/Meshes/Quaternius_SciFi/Door_Single")},
		{&QT_DoorDoubleL,       TEXT("/Game/Meshes/Quaternius_SciFi/Door_Double_L")},
		{&QT_FloorBasic,        TEXT("/Game/Meshes/Quaternius_SciFi/FloorTile_Basic")},
		{&QT_FloorCorner,       TEXT("/Game/Meshes/Quaternius_SciFi/FloorTile_Corner")},
		{&QT_FloorSide,         TEXT("/Game/Meshes/Quaternius_SciFi/FloorTile_Side")},
		{&QT_Wall1,             TEXT("/Game/Meshes/Quaternius_SciFi/Walls/Wall_1")},
		{&QT_Wall3,             TEXT("/Game/Meshes/Quaternius_SciFi/Walls/Wall_3")},
		{&QT_WindowWall,        TEXT("/Game/Meshes/Quaternius_SciFi/Walls/Window_Wall_SideA")},
		{&QT_PropsCrate,        TEXT("/Game/Meshes/Quaternius_SciFi/Props_Crate")},
		{&QT_PropsComputer,     TEXT("/Game/Meshes/Quaternius_SciFi/Props_Computer")},
		{&QT_PropsShelf,        TEXT("/Game/Meshes/Quaternius_SciFi/Props_Shelf")},
		{&QT_PropsContainerFull,TEXT("/Game/Meshes/Quaternius_SciFi/Props_ContainerFull")},
		{&QT_RoofPipes,         TEXT("/Game/Meshes/Quaternius_SciFi/RoofTile_Pipes1")},
		{&QT_Staircase,         TEXT("/Game/Meshes/Quaternius_SciFi/Staircase")},
		{&QT_DetailVent1,       TEXT("/Game/Meshes/Quaternius_SciFi/Details/Details_Vent_1")},
		{&QT_DetailPipesLong,   TEXT("/Game/Meshes/Quaternius_SciFi/Details/Details_Pipes_Long")},
	};
	int32 QTFound = 0;
	for (const auto& P : QTProbes)
	{
		if (auto* Mesh = LoadObject<UStaticMesh>(nullptr, P.Path))
		{
			*P.Target = Mesh;
			QTFound++;
		}
	}
	bHasQuaterniusAssets = (QTFound > 0);
	UE_LOG(LogExoRift, Log, TEXT("Quaternius SciFi: %d/%d meshes loaded.%s"),
		QTFound, QTProbes.Num(),
		bHasQuaterniusAssets ? TEXT(" Using real meshes!") : TEXT(" Not found."));

	// --- SciFi Door mesh ---
	if (auto* DoorMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/SciFiDoor/Sci-fi-door")))
	{
		SF_Door = DoorMesh;
		bHasSciFiDoorAsset = true;
		UE_LOG(LogExoRift, Log, TEXT("SciFi Door: Loaded."));
	}
	else
	{
		UE_LOG(LogExoRift, Log, TEXT("SciFi Door: Not found."));
	}
}
