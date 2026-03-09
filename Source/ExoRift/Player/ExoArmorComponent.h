#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoArmorComponent.generated.h"

UENUM(BlueprintType)
enum class EArmorTier : uint8
{
	None,
	Light,
	Medium,
	Heavy
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnArmorChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmorBroken, bool, bWasHelmet);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoArmorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoArmorComponent();

	// Equip armor at full durability for the given tier
	void EquipHelmet(EArmorTier Tier);
	void EquipVest(EArmorTier Tier);

	/**
	 * Vest absorbs body damage: reduces incoming by tier %, depletes durability.
	 * @return Remaining damage that passes through to health.
	 */
	float AbsorbBodyDamage(float Damage);

	/** Headshot multiplier: 1.0 = no helmet, lower = more reduction. */
	float GetHeadshotReduction() const;

	// Percent accessors (0..1)
	float GetVestPercent() const;
	float GetHelmetPercent() const;

	// Raw accessors
	EArmorTier GetHelmetTier() const { return HelmetTier; }
	EArmorTier GetVestTier() const { return VestTier; }
	float GetVestDurability() const { return VestDurability; }
	float GetHelmetDurability() const { return HelmetDurability; }

	UPROPERTY(BlueprintAssignable)
	FOnArmorChanged OnArmorChanged;

	UPROPERTY(BlueprintAssignable)
	FOnArmorBroken OnArmorBroken;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Armor")
	EArmorTier HelmetTier = EArmorTier::None;

	UPROPERTY(VisibleAnywhere, Category = "Armor")
	EArmorTier VestTier = EArmorTier::None;

	UPROPERTY(VisibleAnywhere, Category = "Armor")
	float VestDurability = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Armor")
	float HelmetDurability = 0.f;

private:
	static float GetMaxDurability(EArmorTier Tier);
	static float GetReductionPercent(EArmorTier Tier);
};
