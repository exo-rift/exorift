#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/ExoTypes.h"
#include "ExoInventoryComponent.generated.h"

class AExoWeaponBase;
class AExoWeaponPickup;
class AExoCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChanged, AExoWeaponBase*, NewWeapon, int32, SlotIndex);

/**
 * Manages weapon inventory with 3 fixed slots: Primary, Secondary, Utility.
 * Replaces the simple WeaponInventory array that was in ExoCharacter.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoInventoryComponent();

	/** Total number of weapon slots. */
	static constexpr int32 SlotCount = 3;
	int32 GetSlotCount() const { return SlotCount; }

	// --- Slot management ---

	/** Adds a weapon to the first empty slot that matches its type. Returns false if no room. */
	bool AddWeapon(AExoWeaponBase* Weapon);

	/** Drops the weapon in the given slot, spawning a pickup at the owner's feet. */
	bool DropWeapon(int32 SlotIndex);

	/** Switch to a specific slot (does nothing if slot is empty). */
	void SwapToSlot(int32 SlotIndex);

	/** Cycle to next/prev weapon. Direction > 0 = next, < 0 = prev. */
	void CycleWeapon(int32 Direction);

	/** Switch to the always-available melee weapon. */
	void SwitchToMelee();

	// --- Accessors ---

	AExoWeaponBase* GetWeapon(int32 SlotIndex) const;
	AExoWeaponBase* GetCurrentWeapon() const;
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	UPROPERTY(BlueprintAssignable)
	FOnWeaponChanged OnWeaponChanged;

protected:
	virtual void BeginPlay() override;

private:
	/** Maps slot index to expected weapon type. */
	static EWeaponType SlotTypeForIndex(int32 Index);

	/** Find the first empty slot whose type matches the weapon. Returns -1 if none. */
	int32 FindSlotForWeapon(AExoWeaponBase* Weapon) const;

	/** Attaches a weapon actor to the owner character's camera. */
	void AttachWeaponToOwner(AExoWeaponBase* Weapon);

	/** Spawn a pickup at the owner's feet representing the given weapon. */
	void SpawnPickupForWeapon(AExoWeaponBase* Weapon);

	/** Spawn the default loadout weapons. Called from BeginPlay on authority. */
	void SpawnDefaultWeapons();

	/** Check if all slotted weapons are out of energy. If so, auto-switch to melee. */
	void CheckAutoSwitchToMelee();

	UPROPERTY()
	TArray<AExoWeaponBase*> Slots; // Always size SlotCount

	/** Dedicated melee weapon — always available, does not occupy a slot. */
	UPROPERTY()
	AExoWeaponBase* MeleeWeapon = nullptr;

	/** True when the melee weapon is currently active (overrides slot-based selection). */
	bool bMeleeActive = false;

	int32 CurrentSlotIndex = 0;
};
