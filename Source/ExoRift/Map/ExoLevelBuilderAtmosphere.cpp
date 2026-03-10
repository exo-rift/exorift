// ExoLevelBuilderAtmosphere.cpp — Holographic displays, spotlights, energy conduits, neon
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoFlickerLight.h"
#include "Visual/ExoHoloBillboard.h"

void AExoLevelBuilder::BuildAtmosphere()
{
	// === HOLOGRAPHIC DISPLAYS — floating data screens near key locations ===
	SpawnHolographicDisplay(FVector(0.f, 0.f, 400.f), 0.f, 1.f);         // Central hub
	SpawnHolographicDisplay(FVector(-2500.f, 80000.f, 300.f), 30.f, 0.8f); // North compound
	SpawnHolographicDisplay(FVector(3000.f, -80000.f, 300.f), -20.f, 0.9f); // South research
	SpawnHolographicDisplay(FVector(80000.f, 1500.f, 350.f), 45.f, 1.1f);  // East power
	SpawnHolographicDisplay(FVector(-80000.f, -2000.f, 300.f), -60.f, 0.7f); // West barracks

	// === SPOTLIGHT BEAMS — vertical search/beacon lights ===
	FLinearColor CoolWhite(0.7f, 0.8f, 1.f);
	FLinearColor WarnOrange(1.f, 0.5f, 0.1f);
	FLinearColor TacBlue(0.2f, 0.4f, 1.f);

	SpawnSpotlightBeam(FVector(5000.f, 5000.f, 10.f), 3000.f, CoolWhite);
	SpawnSpotlightBeam(FVector(-5000.f, -5000.f, 10.f), 2800.f, CoolWhite);
	SpawnSpotlightBeam(FVector(82000.f, 3000.f, 10.f), 3500.f, WarnOrange);    // Power station
	SpawnSpotlightBeam(FVector(-82000.f, 0.f, 10.f), 2500.f, TacBlue);          // Barracks
	SpawnSpotlightBeam(FVector(0.f, 82000.f, 10.f), 3200.f, WarnOrange);        // North
	SpawnSpotlightBeam(FVector(0.f, -82000.f, 10.f), 2800.f, TacBlue);          // South
	// Outer perimeter beacons
	SpawnSpotlightBeam(FVector(150000.f, 0.f, 10.f), 4000.f, WarnOrange);
	SpawnSpotlightBeam(FVector(-150000.f, 0.f, 10.f), 4000.f, WarnOrange);
	SpawnSpotlightBeam(FVector(0.f, 150000.f, 10.f), 4000.f, WarnOrange);
	SpawnSpotlightBeam(FVector(0.f, -150000.f, 10.f), 4000.f, WarnOrange);

	// === ENERGY CONDUITS — glowing pipes connecting buildings ===
	FLinearColor ConduitCyan(0.1f, 0.6f, 0.8f);
	FLinearColor ConduitOrange(0.8f, 0.3f, 0.05f);

	// Hub to north
	SpawnEnergyConduit(FVector(0.f, 5000.f, 150.f), FVector(0.f, 30000.f, 150.f), ConduitCyan);
	// Hub to east
	SpawnEnergyConduit(FVector(5000.f, 0.f, 150.f), FVector(30000.f, 0.f, 150.f), ConduitCyan);
	// Hub to south
	SpawnEnergyConduit(FVector(0.f, -5000.f, 150.f), FVector(0.f, -30000.f, 150.f), ConduitOrange);
	// Hub to west
	SpawnEnergyConduit(FVector(-5000.f, 0.f, 150.f), FVector(-30000.f, 0.f, 150.f), ConduitOrange);

	// === NEON ACCENT TUBES — decorative glowing strips on buildings ===
	FLinearColor NeonCyan(0.05f, 0.8f, 1.f);
	FLinearColor NeonMagenta(1.f, 0.1f, 0.6f);
	FLinearColor NeonGold(1.f, 0.7f, 0.1f);

	// Central hub accents
	SpawnNeonTube(FVector(4500.f, 0.f, 200.f), FVector(0.08f, 5.f, 0.08f), 0.f, NeonCyan);
	SpawnNeonTube(FVector(-4500.f, 0.f, 200.f), FVector(0.08f, 5.f, 0.08f), 0.f, NeonMagenta);
	SpawnNeonTube(FVector(0.f, 4500.f, 200.f), FVector(5.f, 0.08f, 0.08f), 0.f, NeonCyan);
	SpawnNeonTube(FVector(0.f, -4500.f, 200.f), FVector(5.f, 0.08f, 0.08f), 0.f, NeonMagenta);

	// North compound accents
	SpawnNeonTube(FVector(-5000.f, 82000.f, 300.f), FVector(0.06f, 4.f, 0.06f), 0.f, NeonGold);
	SpawnNeonTube(FVector(5000.f, 82000.f, 300.f), FVector(0.06f, 4.f, 0.06f), 0.f, NeonCyan);

	// Power station warning strips
	SpawnNeonTube(FVector(83000.f, -3000.f, 200.f), FVector(0.06f, 0.06f, 3.f), 0.f, NeonGold);
	SpawnNeonTube(FVector(83000.f, 3000.f, 200.f), FVector(0.06f, 0.06f, 3.f), 0.f, NeonGold);

	// Barracks entrance markers
	SpawnNeonTube(FVector(-83000.f, -5000.f, 180.f), FVector(0.06f, 0.06f, 2.5f), 0.f, NeonMagenta);
	SpawnNeonTube(FVector(-83000.f, 5000.f, 180.f), FVector(0.06f, 0.06f, 2.5f), 0.f, NeonMagenta);

	// === FLICKERING LIGHTS — damaged fixtures for atmosphere ===
	FActorSpawnParameters FlickerParams;
	FlickerParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	struct FFlickerDef { FVector Pos; FLinearColor Color; float Intensity; };
	TArray<FFlickerDef> Flickers = {
		// Industrial compound — damaged amber lights
		{{-4000.f, 79000.f, 500.f}, FLinearColor(1.f, 0.6f, 0.15f), 6000.f},
		{{6000.f, 81000.f, 450.f}, FLinearColor(1.f, 0.5f, 0.1f), 5000.f},
		// Barracks corridor — red alert flickers
		{{-79000.f, 3000.f, 400.f}, FLinearColor(1.f, 0.15f, 0.1f), 7000.f},
		{{-81000.f, -4000.f, 400.f}, FLinearColor(1.f, 0.2f, 0.1f), 5500.f},
		// Tunnel entrance — unstable white
		{{40000.f, -40000.f, 300.f}, FLinearColor(0.8f, 0.85f, 1.f), 4000.f},
		// Research labs — flickering green
		{{1000.f, -81000.f, 450.f}, FLinearColor(0.3f, 1.f, 0.4f), 5000.f},
	};

	for (const FFlickerDef& F : Flickers)
	{
		AExoFlickerLight* FL = GetWorld()->SpawnActor<AExoFlickerLight>(
			AExoFlickerLight::StaticClass(), F.Pos, FRotator::ZeroRotator, FlickerParams);
		if (FL) FL->InitLight(F.Color, F.Intensity);
	}

	// === HOLOGRAPHIC BILLBOARDS — large animated screens at compounds ===
	{
		FActorSpawnParameters BP;
		BP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		struct FBoardDef { FVector Pos; float Yaw; FLinearColor Color; float W; float H; };
		TArray<FBoardDef> Boards = {
			// Hub — large central display
			{{0.f, -6000.f, 1200.f}, 0.f, FLinearColor(0.1f, 0.6f, 1.f), 5000.f, 2500.f},
			// North compound — industrial amber
			{{-5000.f, 80000.f, 800.f}, 90.f, FLinearColor(1.f, 0.6f, 0.1f), 3500.f, 1800.f},
			// East power station — electric blue
			{{80000.f, -5000.f, 900.f}, 0.f, FLinearColor(0.15f, 0.4f, 1.f), 3000.f, 1500.f},
			// South labs — green data
			{{4000.f, -80000.f, 700.f}, 180.f, FLinearColor(0.2f, 0.9f, 0.4f), 3500.f, 2000.f},
			// West barracks — red alert
			{{-80000.f, 4000.f, 800.f}, 270.f, FLinearColor(1.f, 0.25f, 0.15f), 3000.f, 1500.f},
		};

		for (const FBoardDef& B : Boards)
		{
			AExoHoloBillboard* BB = GetWorld()->SpawnActor<AExoHoloBillboard>(
				AExoHoloBillboard::StaticClass(), B.Pos,
				FRotator(0.f, B.Yaw, 0.f), BP);
			if (BB) BB->InitBillboard(B.Color, B.W, B.H);
		}
	}
}

void AExoLevelBuilder::SpawnHolographicDisplay(const FVector& Pos, float Yaw, float Scale)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FLinearColor DarkFrame(0.04f, 0.04f, 0.06f);

	// Support pillar
	SpawnStaticMesh(Pos - FVector(0.f, 0.f, Pos.Z * 0.5f),
		FVector(0.15f, 0.15f, Pos.Z / 100.f) * Scale, Rot, CylinderMesh, DarkFrame);

	// Projector base (disk on top of pillar)
	SpawnStaticMesh(Pos, FVector(0.6f, 0.6f, 0.05f) * Scale, Rot, CylinderMesh, DarkFrame);

	// Holographic screen — thin glowing rectangle floating above
	float ScreenH = 180.f * Scale;
	FVector ScreenPos = Pos + FVector(0.f, 0.f, ScreenH * 0.5f + 40.f);
	UStaticMeshComponent* Screen = SpawnStaticMesh(ScreenPos,
		FVector(2.5f, 0.02f, 1.5f) * Scale,
		Rot + FRotator(5.f, 0.f, 0.f),
		CubeMesh, FLinearColor(0.03f, 0.15f, 0.25f));
	if (Screen && BaseMaterial)
	{
		UMaterialInstanceDynamic* SM = Cast<UMaterialInstanceDynamic>(Screen->GetMaterial(0));
		if (SM) SM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.5f, 2.f, 4.f));
	}

	// Scanline bars (3 horizontal lines on screen)
	for (int32 i = 0; i < 3; i++)
	{
		float BarZ = -40.f + i * 40.f;
		FVector BarPos = ScreenPos + Rot.RotateVector(FVector(0.f, 2.f, BarZ * Scale));
		UStaticMeshComponent* Bar = SpawnStaticMesh(BarPos,
			FVector(2.2f, 0.01f, 0.05f) * Scale, Rot,
			CubeMesh, FLinearColor(0.1f, 0.4f, 0.6f));
		if (Bar && BaseMaterial)
		{
			UMaterialInstanceDynamic* BM = Cast<UMaterialInstanceDynamic>(Bar->GetMaterial(0));
			if (BM) BM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(1.f, 3.f, 5.f));
		}
	}

	// Hologram light — cyan glow
	UPointLightComponent* HoloLight = NewObject<UPointLightComponent>(this);
	HoloLight->SetupAttachment(RootComponent);
	HoloLight->SetWorldLocation(ScreenPos);
	HoloLight->SetIntensity(8000.f * Scale);
	HoloLight->SetAttenuationRadius(800.f * Scale);
	HoloLight->SetLightColor(FLinearColor(0.15f, 0.5f, 0.8f));
	HoloLight->CastShadows = false;
	HoloLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnSpotlightBeam(const FVector& Base, float Height,
	const FLinearColor& Color)
{
	FLinearColor DarkMetal(0.06f, 0.06f, 0.08f);

	// Pole
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(0.3f, 0.3f, Height / 100.f), FRotator::ZeroRotator,
		CylinderMesh, DarkMetal);

	// Fixture housing (small box at top)
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height),
		FVector(0.5f, 0.5f, 0.3f), FRotator::ZeroRotator,
		CubeMesh, DarkMetal);

	// Spot light pointing down
	USpotLightComponent* Spot = NewObject<USpotLightComponent>(this);
	Spot->SetupAttachment(RootComponent);
	Spot->SetWorldLocation(Base + FVector(0.f, 0.f, Height));
	Spot->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	Spot->SetIntensity(50000.f);
	Spot->SetAttenuationRadius(Height * 1.5f);
	Spot->SetOuterConeAngle(35.f);
	Spot->SetInnerConeAngle(20.f);
	Spot->SetLightColor(Color);
	Spot->CastShadows = false;
	Spot->RegisterComponent();

	// Ground pool of light (bright circle on ground)
	UPointLightComponent* PoolLight = NewObject<UPointLightComponent>(this);
	PoolLight->SetupAttachment(RootComponent);
	PoolLight->SetWorldLocation(Base + FVector(0.f, 0.f, 50.f));
	PoolLight->SetIntensity(3000.f);
	PoolLight->SetAttenuationRadius(500.f);
	PoolLight->SetLightColor(Color);
	PoolLight->CastShadows = false;
	PoolLight->RegisterComponent();

	// Status LED on fixture
	UStaticMeshComponent* LED = SpawnStaticMesh(
		Base + FVector(20.f, 0.f, Height - 10.f),
		FVector(0.04f, 0.04f, 0.04f), FRotator::ZeroRotator,
		SphereMesh, Color);
	if (LED && BaseMaterial)
	{
		UMaterialInstanceDynamic* LM = Cast<UMaterialInstanceDynamic>(LED->GetMaterial(0));
		if (LM) LM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 8.f, Color.G * 8.f, Color.B * 8.f));
	}
}

void AExoLevelBuilder::SpawnEnergyConduit(const FVector& Start, const FVector& End,
	const FLinearColor& Color)
{
	FVector Dir = End - Start;
	float Len = Dir.Size();
	FVector Mid = (Start + End) * 0.5f;
	FRotator Rot = Dir.Rotation();

	// Outer pipe (dark casing)
	SpawnStaticMesh(Mid, FVector(0.25f, 0.25f, Len / 100.f),
		FRotator(0.f, Rot.Yaw, 90.f), CylinderMesh,
		FLinearColor(0.05f, 0.05f, 0.06f));

	// Inner glowing core
	UStaticMeshComponent* Core = SpawnStaticMesh(Mid,
		FVector(0.12f, 0.12f, Len / 100.f + 0.1f),
		FRotator(0.f, Rot.Yaw, 90.f), CylinderMesh, Color);
	if (Core && BaseMaterial)
	{
		UMaterialInstanceDynamic* CM = Cast<UMaterialInstanceDynamic>(Core->GetMaterial(0));
		if (CM) CM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 6.f, Color.G * 6.f, Color.B * 6.f));
	}

	// Junction nodes at start and end
	for (const FVector& Pt : {Start, End})
	{
		SpawnStaticMesh(Pt, FVector(0.4f, 0.4f, 0.4f),
			FRotator::ZeroRotator, SphereMesh,
			FLinearColor(0.05f, 0.05f, 0.06f));

		UPointLightComponent* JL = NewObject<UPointLightComponent>(this);
		JL->SetupAttachment(RootComponent);
		JL->SetWorldLocation(Pt);
		JL->SetIntensity(2000.f);
		JL->SetAttenuationRadius(400.f);
		JL->SetLightColor(Color);
		JL->CastShadows = false;
		JL->RegisterComponent();
	}

	// Mid-span glow light
	UPointLightComponent* MidLight = NewObject<UPointLightComponent>(this);
	MidLight->SetupAttachment(RootComponent);
	MidLight->SetWorldLocation(Mid);
	MidLight->SetIntensity(1500.f);
	MidLight->SetAttenuationRadius(300.f);
	MidLight->SetLightColor(Color);
	MidLight->CastShadows = false;
	MidLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnNeonTube(const FVector& Pos, const FVector& Scale,
	float Yaw, const FLinearColor& Color)
{
	FRotator Rot(0.f, Yaw, 0.f);

	UStaticMeshComponent* Tube = SpawnStaticMesh(Pos, Scale, Rot, CylinderMesh, Color);
	if (Tube && BaseMaterial)
	{
		UMaterialInstanceDynamic* TM = Cast<UMaterialInstanceDynamic>(Tube->GetMaterial(0));
		if (TM) TM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 10.f, Color.G * 10.f, Color.B * 10.f));
	}

	// Neon glow light
	UPointLightComponent* NL = NewObject<UPointLightComponent>(this);
	NL->SetupAttachment(RootComponent);
	NL->SetWorldLocation(Pos);
	float MaxDim = FMath::Max3(Scale.X, Scale.Y, Scale.Z) * 100.f;
	NL->SetIntensity(4000.f);
	NL->SetAttenuationRadius(FMath::Max(MaxDim * 2.f, 300.f));
	NL->SetLightColor(Color);
	NL->CastShadows = false;
	NL->RegisterComponent();
}
