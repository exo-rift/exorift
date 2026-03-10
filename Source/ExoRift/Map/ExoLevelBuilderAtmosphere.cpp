// ExoLevelBuilderAtmosphere.cpp — Holographic displays, spotlights, energy conduits, neon
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoForceFieldGate.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoFlickerLight.h"
#include "Visual/ExoHoloBillboard.h"
#include "Visual/ExoSparkEmitter.h"
#include "Visual/ExoRotatingProp.h"
#include "ExoRift.h"

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
	// Wilderness beacons — distant towers visible from open fields
	SpawnSpotlightBeam(FVector(100000.f, 100000.f, 10.f), 3500.f, TacBlue);
	SpawnSpotlightBeam(FVector(-100000.f, -100000.f, 10.f), 3500.f, TacBlue);
	SpawnSpotlightBeam(FVector(100000.f, -100000.f, 10.f), 3200.f, CoolWhite);
	SpawnSpotlightBeam(FVector(-100000.f, 100000.f, 10.f), 3200.f, CoolWhite);
	// Mid-field navigation beacons
	SpawnSpotlightBeam(FVector(40000.f, 40000.f, 10.f), 2500.f, TacBlue);
	SpawnSpotlightBeam(FVector(-40000.f, -40000.f, 10.f), 2500.f, CoolWhite);
	SpawnSpotlightBeam(FVector(40000.f, -40000.f, 10.f), 2200.f, WarnOrange);
	SpawnSpotlightBeam(FVector(-40000.f, 40000.f, 10.f), 2200.f, TacBlue);

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
	// Inter-compound conduits — linking adjacent bases
	// North to East (diagonal)
	SpawnEnergyConduit(FVector(15000.f, 65000.f, 200.f), FVector(65000.f, 15000.f, 200.f), ConduitCyan);
	// East to South (diagonal)
	SpawnEnergyConduit(FVector(65000.f, -15000.f, 200.f), FVector(15000.f, -65000.f, 200.f), ConduitOrange);
	// South to West (diagonal)
	SpawnEnergyConduit(FVector(-15000.f, -65000.f, 200.f), FVector(-65000.f, -15000.f, 200.f), ConduitCyan);
	// West to North (diagonal)
	SpawnEnergyConduit(FVector(-65000.f, 15000.f, 200.f), FVector(-15000.f, 65000.f, 200.f), ConduitOrange);

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

	// === SPARK EMITTERS — damaged electrical panels at compounds ===
	{
		FActorSpawnParameters SP;
		SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		struct FSparkDef { FVector Pos; FRotator Dir; FLinearColor Color; float Interval; };
		TArray<FSparkDef> SparkDefs = {
			// Hub — damaged junction box
			{{3500.f, 2000.f, 300.f}, FRotator(0.f, -90.f, 0.f),
				FLinearColor(0.3f, 0.7f, 1.f), 4.f},
			{{-2000.f, -3000.f, 250.f}, FRotator(0.f, 45.f, 0.f),
				FLinearColor(1.f, 0.6f, 0.1f), 3.5f},
			// North industrial — exposed wiring
			{{5000.f, 79000.f, 400.f}, FRotator(0.f, 0.f, 0.f),
				FLinearColor(1.f, 0.5f, 0.1f), 2.5f},
			{{-3000.f, 82000.f, 200.f}, FRotator(0.f, 180.f, 0.f),
				FLinearColor(1.f, 0.7f, 0.2f), 5.f},
			// East power — overloaded conduit
			{{81000.f, -2000.f, 350.f}, FRotator(0.f, -90.f, 0.f),
				FLinearColor(0.2f, 0.5f, 1.f), 2.f},
			{{79000.f, 3000.f, 500.f}, FRotator(-30.f, 0.f, 0.f),
				FLinearColor(0.4f, 0.6f, 1.f), 3.f},
			// South labs — containment breach sparks
			{{2000.f, -81000.f, 300.f}, FRotator(0.f, 90.f, 0.f),
				FLinearColor(0.3f, 1.f, 0.4f), 4.5f},
			// West barracks — battle damage
			{{-81000.f, 5000.f, 250.f}, FRotator(0.f, 0.f, 0.f),
				FLinearColor(1.f, 0.3f, 0.1f), 3.f},
			{{-79000.f, -3000.f, 400.f}, FRotator(0.f, 270.f, 0.f),
				FLinearColor(1.f, 0.4f, 0.15f), 6.f},
		};

		for (const FSparkDef& S : SparkDefs)
		{
			AExoSparkEmitter* SE = GetWorld()->SpawnActor<AExoSparkEmitter>(
				AExoSparkEmitter::StaticClass(), S.Pos, S.Dir, SP);
			if (SE) SE->InitSparks(S.Color, S.Interval);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d spark emitters"), SparkDefs.Num());
	}

	// === ROTATING PROPS — radar dishes, fans, energy coils ===
	{
		FActorSpawnParameters RP;
		RP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		struct FPropDef { FVector Pos; int32 Type; FLinearColor Color; float Speed; float Scale; };
		TArray<FPropDef> PropDefs = {
			// Hub — radar dish on command center
			{{0.f, 0.f, 2600.f}, 1, FLinearColor(0.2f, 0.6f, 1.f), 15.f, 1.5f},
			// Hub — energy coil at center
			{{0.f, 3000.f, 200.f}, 2, FLinearColor(0.2f, 0.8f, 1.f), 40.f, 1.2f},
			// North — industrial exhaust fan
			{{-4000.f, 81000.f, 600.f}, 0, FLinearColor(0.5f, 0.5f, 0.5f), 120.f, 1.0f},
			{{4000.f, 79000.f, 500.f}, 0, FLinearColor(0.5f, 0.5f, 0.5f), 90.f, 0.8f},
			// East — power coils
			{{82000.f, 0.f, 400.f}, 2, FLinearColor(0.4f, 0.6f, 1.f), 60.f, 1.0f},
			{{78000.f, -4000.f, 350.f}, 2, FLinearColor(0.3f, 0.5f, 1.f), 45.f, 0.7f},
			// South — research scanner dish
			{{0.f, -82000.f, 500.f}, 1, FLinearColor(0.3f, 1.f, 0.4f), 8.f, 1.3f},
			// West — barracks antenna
			{{-82000.f, 0.f, 600.f}, 1, FLinearColor(1.f, 0.3f, 0.1f), 12.f, 1.0f},
			// Corner outposts — small dishes
			{{120000.f, 120000.f, 400.f}, 1, FLinearColor(0.5f, 0.5f, 0.6f), 20.f, 0.6f},
			{{-120000.f, -120000.f, 400.f}, 1, FLinearColor(0.5f, 0.5f, 0.6f), 20.f, 0.6f},
		};

		for (const FPropDef& P : PropDefs)
		{
			AExoRotatingProp* Prop = GetWorld()->SpawnActor<AExoRotatingProp>(
				AExoRotatingProp::StaticClass(), P.Pos, FRotator::ZeroRotator, RP);
			if (Prop) Prop->InitProp(P.Type, P.Color, P.Speed, P.Scale);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed %d rotating props"), PropDefs.Num());
	}

	// === FORCE FIELD GATES at compound entrances ===
	{
		FActorSpawnParameters GateP;
		GateP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		struct FGateDef { FVector Pos; float Yaw; float Width; float Height; FLinearColor Color; };
		FGateDef Gates[] = {
			// Hub entrance (south-facing)
			{{4000.f, -3000.f, 10.f}, 0.f, 600.f, 350.f, FLinearColor(0.2f, 0.5f, 1.f)},
			// North compound main entry
			{{-10000.f, 76000.f, 10.f}, 0.f, 800.f, 400.f, FLinearColor(1.f, 0.5f, 0.1f)},
			// South research lab entry
			{{-9000.f, -80000.f, 10.f}, 90.f, 600.f, 350.f, FLinearColor(0.3f, 1.f, 0.4f)},
			// East power station entry
			{{80000.f, -4000.f, 10.f}, 90.f, 700.f, 400.f, FLinearColor(1.f, 0.3f, 0.1f)},
			// West barracks entry (between guard posts)
			{{-80000.f, 6000.f, 10.f}, 0.f, 800.f, 350.f, FLinearColor(0.2f, 0.4f, 1.f)},
		};

		for (const FGateDef& G : Gates)
		{
			AExoForceFieldGate* Gate = GetWorld()->SpawnActor<AExoForceFieldGate>(
				AExoForceFieldGate::StaticClass(), G.Pos,
				FRotator(0.f, G.Yaw, 0.f), GateP);
			if (Gate) Gate->InitGate(G.Width, G.Height, G.Color);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 5 force field gates"));
	}
}

