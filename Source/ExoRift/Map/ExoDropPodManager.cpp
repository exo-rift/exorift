#include "Map/ExoDropPodManager.h"
#include "Map/ExoDropPod.h"
#include "ExoRift.h"

AExoDropPodManager::AExoDropPodManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AExoDropPodManager::StartDropPhase(const TArray<AController*>& Players)
{
	TotalPods = Players.Num();
	LandedCount = 0;

	for (AController* PC : Players)
	{
		if (!PC) continue;

		// Random drop position within spread radius
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Distance = FMath::RandRange(0.f, DropSpreadRadius);
		FVector DropLoc(
			FMath::Cos(Angle) * Distance,
			FMath::Sin(Angle) * Distance,
			DropAltitude
		);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		AExoDropPod* Pod = GetWorld()->SpawnActor<AExoDropPod>(
			AExoDropPod::StaticClass(), DropLoc, FRotator::ZeroRotator, SpawnParams);
		if (Pod)
		{
			Pod->InitPod(PC, this);
			ActivePods.Add(Pod);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Drop phase started: %d pods deployed"), TotalPods);
}

void AExoDropPodManager::OnPodLanded(AExoDropPod* Pod)
{
	LandedCount++;
	ActivePods.Remove(Pod);

	UE_LOG(LogExoRift, Log, TEXT("Pod landed (%d/%d)"), LandedCount, TotalPods);
}
