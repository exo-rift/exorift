#include "Map/ExoHazardZone.h"
#include "Player/ExoCharacter.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoHazardZone::AExoHazardZone()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AExoHazardZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bEnabled)
	{
		ApplyHazardDamage(DeltaTime);
	}
}

void AExoHazardZone::SetHazardEnabled(bool bEnable)
{
	bEnabled = bEnable;
	UE_LOG(LogExoRift, Log, TEXT("Hazard '%s' %s"), *HazardName,
		bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AExoHazardZone::ApplyHazardDamage(float DeltaTime)
{
	if (!HasAuthority()) return;

	const FVector Origin = GetActorLocation();
	const float RadiusSq = HazardRadius * HazardRadius;
	const float FrameDamage = DamagePerSecond * DeltaTime;

	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (!Char || !Char->IsAlive()) continue;

		float DistSq = FVector::DistSquared(Origin, Char->GetActorLocation());
		if (DistSq > RadiusSq) continue;

		// Build a damage event tagged with the hazard type for future VFX
		FDamageEvent DamageEvent;
		Char->TakeDamage(FrameDamage, DamageEvent, nullptr, this);
	}
}
