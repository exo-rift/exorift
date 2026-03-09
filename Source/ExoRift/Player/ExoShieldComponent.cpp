#include "Player/ExoShieldComponent.h"
#include "Net/UnrealNetwork.h"

UExoShieldComponent::UExoShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	CurrentShield = MaxShield;
}

void UExoShieldComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastDamage += DeltaTime;

	// Recharge after delay
	if (CurrentShield < MaxShield && TimeSinceLastDamage >= RechargeDelay)
	{
		float OldShield = CurrentShield;
		CurrentShield = FMath::Min(CurrentShield + RechargeRate * DeltaTime, MaxShield);

		if (CurrentShield != OldShield)
		{
			OnShieldChanged.Broadcast(CurrentShield, MaxShield);
		}

		if (CurrentShield >= MaxShield && bWasBroken)
		{
			bWasBroken = false;
			OnShieldFullyRecharged.Broadcast();
		}
	}
}

float UExoShieldComponent::AbsorbDamage(float IncomingDamage)
{
	if (CurrentShield <= 0.f) return IncomingDamage;

	TimeSinceLastDamage = 0.f;
	float Absorbed = FMath::Min(IncomingDamage, CurrentShield);
	CurrentShield -= Absorbed;
	float Remaining = IncomingDamage - Absorbed;

	OnShieldChanged.Broadcast(CurrentShield, MaxShield);

	if (CurrentShield <= 0.f)
	{
		bWasBroken = true;
		OnShieldBroken.Broadcast();
	}

	return Remaining;
}

void UExoShieldComponent::SetShield(float Amount)
{
	CurrentShield = FMath::Clamp(Amount, 0.f, MaxShield);
	OnShieldChanged.Broadcast(CurrentShield, MaxShield);
}

void UExoShieldComponent::AddShield(float Amount)
{
	SetShield(CurrentShield + Amount);
}

void UExoShieldComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UExoShieldComponent, CurrentShield);
}
