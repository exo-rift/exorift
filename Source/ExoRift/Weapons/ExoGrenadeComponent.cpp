#include "Weapons/ExoGrenadeComponent.h"
#include "Weapons/ExoGrenade.h"
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

	Grenade->Ignite(Direction.Vector());

	--CurrentGrenades;
	LastThrowTime = Now;

	UE_LOG(LogExoRift, Log, TEXT("Grenade thrown. Remaining: %d"), CurrentGrenades);
}

void UExoGrenadeComponent::AddGrenades(int32 Count)
{
	CurrentGrenades = FMath::Clamp(CurrentGrenades + Count, 0, MaxGrenades);
}
