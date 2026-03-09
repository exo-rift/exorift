#include "Player/ExoArmorComponent.h"
#include "ExoRift.h"

UExoArmorComponent::UExoArmorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ---------------------------------------------------------------------------
// Tier stat tables
// ---------------------------------------------------------------------------

float UExoArmorComponent::GetMaxDurability(EArmorTier Tier)
{
	switch (Tier)
	{
	case EArmorTier::Light:  return 50.f;
	case EArmorTier::Medium: return 100.f;
	case EArmorTier::Heavy:  return 150.f;
	default:                 return 0.f;
	}
}

float UExoArmorComponent::GetReductionPercent(EArmorTier Tier)
{
	switch (Tier)
	{
	case EArmorTier::Light:  return 0.30f;
	case EArmorTier::Medium: return 0.50f;
	case EArmorTier::Heavy:  return 0.65f;
	default:                 return 0.f;
	}
}

// ---------------------------------------------------------------------------
// Equip
// ---------------------------------------------------------------------------

void UExoArmorComponent::EquipHelmet(EArmorTier Tier)
{
	HelmetTier = Tier;
	HelmetDurability = GetMaxDurability(Tier);
	OnArmorChanged.Broadcast();
	UE_LOG(LogExoRift, Log, TEXT("Helmet equipped: Tier %d (%.0f HP)"),
		static_cast<uint8>(Tier), HelmetDurability);
}

void UExoArmorComponent::EquipVest(EArmorTier Tier)
{
	VestTier = Tier;
	VestDurability = GetMaxDurability(Tier);
	OnArmorChanged.Broadcast();
	UE_LOG(LogExoRift, Log, TEXT("Vest equipped: Tier %d (%.0f HP)"),
		static_cast<uint8>(Tier), VestDurability);
}

// ---------------------------------------------------------------------------
// Damage absorption
// ---------------------------------------------------------------------------

float UExoArmorComponent::AbsorbBodyDamage(float Damage)
{
	if (VestTier == EArmorTier::None || VestDurability <= 0.f)
	{
		return Damage;
	}

	// Reduce incoming damage by the vest's reduction percentage
	float Reduction = GetReductionPercent(VestTier);
	float ReducedDamage = Damage * (1.f - Reduction);
	float AbsorbedByVest = Damage - ReducedDamage;

	// Vest durability absorbs the mitigated portion
	if (AbsorbedByVest > VestDurability)
	{
		// Vest breaks — the overflow returns as extra damage
		float Overflow = AbsorbedByVest - VestDurability;
		VestDurability = 0.f;
		VestTier = EArmorTier::None;
		OnArmorBroken.Broadcast(false);
		OnArmorChanged.Broadcast();
		return ReducedDamage + Overflow;
	}

	VestDurability -= AbsorbedByVest;
	OnArmorChanged.Broadcast();
	return ReducedDamage;
}

float UExoArmorComponent::GetHeadshotReduction() const
{
	switch (HelmetTier)
	{
	case EArmorTier::Light:  return 0.70f;
	case EArmorTier::Medium: return 0.50f;
	case EArmorTier::Heavy:  return 0.35f;
	default:                 return 1.0f; // No helmet — full headshot damage
	}
}

// ---------------------------------------------------------------------------
// Percent accessors
// ---------------------------------------------------------------------------

float UExoArmorComponent::GetVestPercent() const
{
	float Max = GetMaxDurability(VestTier);
	return Max > 0.f ? VestDurability / Max : 0.f;
}

float UExoArmorComponent::GetHelmetPercent() const
{
	float Max = GetMaxDurability(HelmetTier);
	return Max > 0.f ? HelmetDurability / Max : 0.f;
}
