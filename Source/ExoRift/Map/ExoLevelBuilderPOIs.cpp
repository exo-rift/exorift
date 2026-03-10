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
			{1500.f, -2000.f, 50.f}, {-1800.f, 3000.f, 50.f},
			{3500.f, 79500.f, 50.f}, {-4500.f, 81500.f, 50.f},
			{2000.f, -80500.f, 50.f}, {-3500.f, -82000.f, 50.f},
			{81500.f, 1500.f, 50.f}, {79000.f, -2500.f, 50.f},
			{-80500.f, 2500.f, 50.f}, {-82000.f, -1500.f, 50.f},
			{45000.f, 45000.f, 50.f}, {-55000.f, -55000.f, 50.f},
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
			{{5000.f, 5000.f, 10.f}, FLinearColor(1.f, 0.3f, 0.1f)},
			{{-5000.f, 5500.f, 10.f}, FLinearColor(0.1f, 0.8f, 1.f)},
			{{5500.f, -5000.f, 10.f}, FLinearColor(0.2f, 1.f, 0.3f)},
			{{-5500.f, -5500.f, 10.f}, FLinearColor(1.f, 0.7f, 0.1f)},
			{{7000.f, 0.f, 10.f}, FLinearColor(0.8f, 0.2f, 1.f)},
			{{-7000.f, 0.f, 10.f}, FLinearColor(1.f, 0.1f, 0.4f)},
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
			FVector(30000.f, 40000.f, 0.f),
			FRotator(0.f, -25.f, 0.f), ShipP);
		if (Ship) Ship->BuildShip();
		UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Placed crashed capital ship landmark"));
	}

	// Energy reactor centerpiece at hub
	{
		FActorSpawnParameters ReactorP;
		ReactorP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoReactorCore* Reactor = GetWorld()->SpawnActor<AExoReactorCore>(
			AExoReactorCore::StaticClass(), FVector(0.f, -3000.f, 10.f),
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
			{{60000.f, -60000.f, 0.f}, 15.f},
			{{-70000.f, 70000.f, 0.f}, -30.f},
			{{100000.f, 80000.f, 0.f}, 45.f},
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
			{{-50000.f, 30000.f, 0.f}, 20.f},
			{{40000.f, -50000.f, 0.f}, -35.f},
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
			FVector(-40000.f, -60000.f, 0.f),
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
			{{10000.f, 10000.f, 0.f}, 45.f},
			{{-10000.f, -10000.f, 0.f}, 225.f},
			{{5000.f, 78000.f, 0.f}, 180.f},
			{{78000.f, 5000.f, 0.f}, 270.f},
			{{-78000.f, -5000.f, 0.f}, 90.f},
			{{-5000.f, -78000.f, 0.f}, 0.f},
			{{50000.f, 50000.f, 0.f}, -45.f},
			{{-60000.f, -50000.f, 0.f}, 135.f},
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
			{{2500.f, -1000.f, 10.f}, EPowerUpType::SpeedBoost},
			{{-2500.f, 1000.f, 10.f}, EPowerUpType::DamageBoost},
			{{4000.f, 80000.f, 10.f}, EPowerUpType::ShieldRecharge},
			{{-4000.f, -81000.f, 10.f}, EPowerUpType::OverheatReset},
			{{80000.f, 2000.f, 10.f}, EPowerUpType::SpeedBoost},
			{{-81000.f, -2000.f, 10.f}, EPowerUpType::DamageBoost},
			{{45000.f, 46000.f, 10.f}, EPowerUpType::ShieldRecharge},
			{{-55000.f, -54000.f, 10.f}, EPowerUpType::OverheatReset},
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
