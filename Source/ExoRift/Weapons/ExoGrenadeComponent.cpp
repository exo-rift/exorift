#include "Weapons/ExoGrenadeComponent.h"
#include "Weapons/ExoGrenade.h"
#include "Visual/ExoGrenadeTrail.h"
#include "ExoRift.h"

UExoGrenadeComponent::UExoGrenadeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExoGrenadeComponent::ThrowGrenade(FVector Origin, FRotator Direction)
{
	if (CurrentGrenades <= 0) return;

	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastThrowTime < ThrowCooldown) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	AExoGrenade* Grenade = GetWorld()->SpawnActor<AExoGrenade>(
		AExoGrenade::StaticClass(), Origin, Direction, SpawnParams);

	if (!Grenade) return;

	Grenade->GrenadeType = GrenadeType;
	Grenade->Ignite(Direction.Vector());

	// Attach a trail effect to the grenade
	FActorSpawnParameters TrailParams;
	TrailParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoGrenadeTrail* Trail = GetWorld()->SpawnActor<AExoGrenadeTrail>(
		AExoGrenadeTrail::StaticClass(), Origin, FRotator::ZeroRotator, TrailParams);
	if (Trail) Trail->InitTrail(Grenade, Grenade->GrenadeType);

	--CurrentGrenades;
	LastThrowTime = Now;

	UE_LOG(LogExoRift, Log, TEXT("Grenade thrown. Remaining: %d"), CurrentGrenades);
}

void UExoGrenadeComponent::AddGrenades(int32 Count)
{
	CurrentGrenades = FMath::Clamp(CurrentGrenades + Count, 0, MaxGrenades);
}
