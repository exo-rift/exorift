#include "Map/ExoSupplyDropManager.h"
#include "Map/ExoSupplyDrop.h"
#include "Map/ExoZoneSystem.h"
#include "Core/ExoGameState.h"
#include "Core/ExoTypes.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoSupplyDropManager::AExoSupplyDropManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AExoSupplyDropManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only spawn drops during active gameplay phases
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS) return;

	bool bPlayPhase = (GS->MatchPhase == EBRMatchPhase::Playing ||
	                   GS->MatchPhase == EBRMatchPhase::ZoneShrinking);
	if (!bPlayPhase)
	{
		bIsActive = false;
		return;
	}

	if (!bIsActive)
	{
		bIsActive = true;
		DropTimer = DropInterval * 0.5f; // First drop comes sooner
	}

	CleanupDepletedDrops();

	DropTimer += DeltaTime;
	if (DropTimer >= DropInterval && ActiveDrops.Num() < MaxActiveDrops)
	{
		SpawnDrop();
		DropTimer = 0.f;
	}
}

void AExoSupplyDropManager::SpawnDrop()
{
	// Find zone system to get current zone bounds
	AExoZoneSystem* Zone = nullptr;
	for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
	{
		Zone = *It;
		break;
	}

	FVector SpawnLocation;
	if (Zone)
	{
		// Spawn within the current zone radius
		FVector2D Center = Zone->GetCurrentCenter();
		float Radius = Zone->GetCurrentRadius() * 0.8f; // Slightly inside zone edge
		float Angle = FMath::FRandRange(0.f, 2.f * PI);
		float Dist = FMath::FRandRange(0.f, Radius);
		SpawnLocation = FVector(
			Center.X + FMath::Cos(Angle) * Dist,
			Center.Y + FMath::Sin(Angle) * Dist,
			DropAltitude);
	}
	else
	{
		// Fallback: random position in a 4km area
		SpawnLocation = FVector(
			FMath::FRandRange(-200000.f, 200000.f),
			FMath::FRandRange(-200000.f, 200000.f),
			DropAltitude);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AExoSupplyDrop* Drop = GetWorld()->SpawnActor<AExoSupplyDrop>(
		AExoSupplyDrop::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (Drop)
	{
		ActiveDrops.Add(Drop);

		// Announce the supply drop via game state
		AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
		if (GS)
		{
			GS->SupplyDropAnnouncement = TEXT("SUPPLY DROP INCOMING");
			GS->AnnouncementTimer = 5.f;
		}

		UE_LOG(LogExoRift, Log, TEXT("Supply drop spawned at %s"), *SpawnLocation.ToString());
	}
}

void AExoSupplyDropManager::CleanupDepletedDrops()
{
	for (int32 i = ActiveDrops.Num() - 1; i >= 0; --i)
	{
		if (!ActiveDrops[i] || ActiveDrops[i]->GetState() == ESupplyDropState::Depleted)
		{
			ActiveDrops.RemoveAt(i);
		}
	}
}
