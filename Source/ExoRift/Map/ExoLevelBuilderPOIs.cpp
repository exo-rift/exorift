// ExoLevelBuilderPOIs.cpp — Landmark/POI placement (towers, depots, ships, etc.)
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoLootCrate.h"
#include "Map/ExoTargetDummy.h"
#include "Map/ExoPowerUpTerminal.h"
#include "Map/ExoReactorCore.h"
#include "Map/ExoCrashedCapitalShip.h"
#include "Map/ExoRelayTower.h"
#include "Map/ExoFuelDepot.h"
#include "Map/ExoMiningExcavation.h"
#include "Map/ExoGuardTower.h"
#include "ExoRift.h"

void AExoLevelBuilder::PlacePOIs()
{
	// Loot crates scattered at key locations
	{
		FActorSpawnParameters Crate;
		Crate.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FVector CratePositions[] = {
			{300.f, -400.f, 50.f}, {-360.f, 600.f, 50.f},
			{700.f, 15900.f, 50.f}, {-900.f, 16300.f, 50.f},
			{400.f, -16100.f, 50.f}, {-700.f, -16400.f, 50.f},
			{16300.f, 300.f, 50.f}, {15800.f, -500.f, 50.f},
			{-16100.f, 500.f, 50.f}, {-16400.f, -300.f, 50.f},
			{9000.f, 9000.f, 50.f}, {-11000.f, -11000.f, 50.f},
		};
		for (const FVector& P : CratePositions)
		{
			AExoLootCrate* LC = GetWorld()->SpawnActor<AExoLootCrate>(
				AExoLootCrate::StaticClass(), P,
				FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f), Crate);
			if (LC) LC->ItemCount = FMath::RandRange(1, 3);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 12 loot crates"));
	}

	// Target dummies near hub for warmup practice
	{
		FActorSpawnParameters Dum;
		Dum.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FDummyDef { FVector Pos; FLinearColor Color; };
		FDummyDef Dummies[] = {
			{{1000.f, 1000.f, 10.f}, FLinearColor(1.f, 0.3f, 0.1f)},
			{{-1000.f, 1100.f, 10.f}, FLinearColor(0.1f, 0.8f, 1.f)},
			{{1100.f, -1000.f, 10.f}, FLinearColor(0.2f, 1.f, 0.3f)},
			{{-1100.f, -1100.f, 10.f}, FLinearColor(1.f, 0.7f, 0.1f)},
			{{1400.f, 0.f, 10.f}, FLinearColor(0.8f, 0.2f, 1.f)},
			{{-1400.f, 0.f, 10.f}, FLinearColor(1.f, 0.1f, 0.4f)},
		};
		for (const FDummyDef& D : Dummies)
		{
			AExoTargetDummy* TD = GetWorld()->SpawnActor<AExoTargetDummy>(
				AExoTargetDummy::StaticClass(), D.Pos, FRotator::ZeroRotator, Dum);
			if (TD) TD->InitDummy(D.Color, 200.f);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 6 target dummies"));
	}

	// Crashed capital ship — major landmark between hub and north compound
	{
		FActorSpawnParameters ShipP;
		ShipP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoCrashedCapitalShip* Ship = GetWorld()->SpawnActor<AExoCrashedCapitalShip>(
			AExoCrashedCapitalShip::StaticClass(),
			FVector(6000.f, 8000.f, 0.f),
			FRotator(0.f, -25.f, 0.f), ShipP);
		if (Ship) Ship->BuildShip();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed crashed capital ship landmark"));
	}

	// Energy reactor centerpiece at hub
	{
		FActorSpawnParameters ReactorP;
		ReactorP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoReactorCore* Reactor = GetWorld()->SpawnActor<AExoReactorCore>(
			AExoReactorCore::StaticClass(), FVector(0.f, -600.f, 10.f),
			FRotator::ZeroRotator, ReactorP);
		if (Reactor) Reactor->InitReactor();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed energy reactor at hub center"));
	}

	// Relay towers — tall landmarks for orientation
	{
		FActorSpawnParameters TowerP;
		TowerP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FTowerDef { FVector Pos; float Yaw; };
		FTowerDef Towers[] = {
			{{12000.f, -12000.f, 0.f}, 15.f},
			{{-14000.f, 14000.f, 0.f}, -30.f},
			{{20000.f, 16000.f, 0.f}, 45.f},
			{{-18000.f, -14000.f, 0.f}, 120.f},
			{{16000.f, -18000.f, 0.f}, -60.f},
		};
		for (const FTowerDef& T : Towers)
		{
			AExoRelayTower* Tower = GetWorld()->SpawnActor<AExoRelayTower>(
				AExoRelayTower::StaticClass(), T.Pos,
				FRotator(0.f, T.Yaw, 0.f), TowerP);
			if (Tower) Tower->BuildTower();
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 3 relay towers"));
	}

	// Fuel depots — industrial storage at key routes
	{
		FActorSpawnParameters DepotP;
		DepotP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FDepotDef { FVector Pos; float Yaw; };
		FDepotDef Depots[] = {
			{{-10000.f, 6000.f, 0.f}, 20.f},
			{{8000.f, -10000.f, 0.f}, -35.f},
			{{-4000.f, -6000.f, 0.f}, 60.f},
			{{14000.f, 10000.f, 0.f}, -10.f},
		};
		for (const FDepotDef& D : Depots)
		{
			AExoFuelDepot* Depot = GetWorld()->SpawnActor<AExoFuelDepot>(
				AExoFuelDepot::StaticClass(), D.Pos,
				FRotator(0.f, D.Yaw, 0.f), DepotP);
			if (Depot) Depot->BuildDepot();
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 2 fuel depots"));
	}

	// Mining excavation — quarry with mineral veins
	{
		FActorSpawnParameters MineP;
		MineP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoMiningExcavation* Mine = GetWorld()->SpawnActor<AExoMiningExcavation>(
			AExoMiningExcavation::StaticClass(),
			FVector(-8000.f, -12000.f, 0.f),
			FRotator(0.f, 15.f, 0.f), MineP);
		if (Mine) Mine->BuildSite();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed mining excavation site"));
	}

	// Guard towers — sniper vantage points at strategic locations
	{
		FActorSpawnParameters TowerP;
		TowerP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FGTDef { FVector Pos; float Yaw; };
		FGTDef GuardTowers[] = {
			{{2000.f, 2000.f, 0.f}, 45.f},
			{{-2000.f, -2000.f, 0.f}, 225.f},
			{{1000.f, 15600.f, 0.f}, 180.f},
			{{15600.f, 1000.f, 0.f}, 270.f},
			{{-15600.f, -1000.f, 0.f}, 90.f},
			{{-1000.f, -15600.f, 0.f}, 0.f},
			{{10000.f, 10000.f, 0.f}, -45.f},
			{{-12000.f, -10000.f, 0.f}, 135.f},
			// Additional open-field guard towers for map density
			{{7000.f, -7000.f, 0.f}, 315.f},
			{{-7000.f, 7000.f, 0.f}, 135.f},
			{{14000.f, -14000.f, 0.f}, 315.f},
			{{-14000.f, 14000.f, 0.f}, 135.f},
		};
		for (const FGTDef& GT : GuardTowers)
		{
			AExoGuardTower* Tower = GetWorld()->SpawnActor<AExoGuardTower>(
				AExoGuardTower::StaticClass(), GT.Pos,
				FRotator(0.f, GT.Yaw, 0.f), TowerP);
			if (Tower) Tower->BuildTower();
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 8 guard towers"));
	}

	// Power-up terminals at strategic compound locations
	{
		FActorSpawnParameters TermP;
		TermP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		struct FTermDef { FVector Pos; EPowerUpType Type; };
		FTermDef Terminals[] = {
			{{500.f, -200.f, 10.f}, EPowerUpType::SpeedBoost},
			{{-500.f, 200.f, 10.f}, EPowerUpType::DamageBoost},
			{{800.f, NorthY, 10.f}, EPowerUpType::ShieldRecharge},
			{{-800.f, SouthY - 200.f, 10.f}, EPowerUpType::OverheatReset},
			{{EastX, 400.f, 10.f}, EPowerUpType::SpeedBoost},
			{{WestX - 200.f, -400.f, 10.f}, EPowerUpType::DamageBoost},
			{{9000.f, 9200.f, 10.f}, EPowerUpType::ShieldRecharge},
			{{-11000.f, -10800.f, 10.f}, EPowerUpType::OverheatReset},
		};
		for (const FTermDef& T : Terminals)
		{
			AExoPowerUpTerminal* Term = GetWorld()->SpawnActor<AExoPowerUpTerminal>(
				AExoPowerUpTerminal::StaticClass(), T.Pos,
				FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f), TermP);
			if (Term) Term->InitTerminal(T.Type);
		}
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed 8 power-up terminals"));
	}
}
