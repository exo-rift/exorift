#include "Player/ExoInventoryComponent.h"
#include "Player/ExoCharacter.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Weapons/ExoWeaponShotgun.h"
#include "Weapons/ExoWeaponMelee.h"
#include "Weapons/ExoWeaponPickup.h"
#include "UI/ExoPickupNotification.h"
#include "Camera/CameraComponent.h"
#include "ExoRift.h"

UExoInventoryComponent::UExoInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Slots.SetNum(SlotCount);
}

void UExoInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapons();
}

// ---------------------------------------------------------------------------
// Slot type mapping
// ---------------------------------------------------------------------------

EWeaponType UExoInventoryComponent::SlotTypeForIndex(int32 Index)
{
	switch (Index)
	{
	case 0: return EWeaponType::Rifle;
	case 1: return EWeaponType::Pistol;
	case 2: return EWeaponType::Shotgun;
	default: return EWeaponType::Rifle;
	}
}

int32 UExoInventoryComponent::FindSlotForWeapon(AExoWeaponBase* Weapon) const
{
	if (!Weapon) return -1;

	EWeaponType Type = Weapon->GetWeaponType();

	// First pass: find an empty slot that matches the weapon type
	for (int32 i = 0; i < SlotCount; ++i)
	{
		if (!Slots[i] && SlotTypeForIndex(i) == Type)
		{
			return i;
		}
	}

	// Second pass: find any empty slot
	for (int32 i = 0; i < SlotCount; ++i)
	{
		if (!Slots[i])
		{
			return i;
		}
	}

	return -1;
}

// ---------------------------------------------------------------------------
// Add / Drop / Swap
// ---------------------------------------------------------------------------

bool UExoInventoryComponent::AddWeapon(AExoWeaponBase* Weapon)
{
	if (!Weapon) return false;

	int32 TargetSlot = FindSlotForWeapon(Weapon);
	if (TargetSlot < 0) return false;

	Slots[TargetSlot] = Weapon;
	AttachWeaponToOwner(Weapon);

	// If this is the first weapon or the current slot was empty, make it active
	bool bHadActive = GetCurrentWeapon() != nullptr;
	if (!bHadActive || CurrentSlotIndex == TargetSlot)
	{
		CurrentSlotIndex = TargetSlot;
		Weapon->SetActorHiddenInGame(false);
		OnWeaponChanged.Broadcast(Weapon, CurrentSlotIndex);
	}
	else
	{
		Weapon->SetActorHiddenInGame(true);
	}

	// Pickup notification for local player
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwner());
	if (Char && Char->IsLocallyControlled())
	{
		FExoPickupNotification::ShowWeaponPickup(
			Weapon->GetWeaponName(), AExoWeaponBase::GetRarityColor(Weapon->Rarity));
	}

	UE_LOG(LogExoRift, Log, TEXT("Inventory: added %s to slot %d"), *Weapon->GetWeaponName(), TargetSlot);
	return true;
}

bool UExoInventoryComponent::DropWeapon(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= SlotCount) return false;
	AExoWeaponBase* Weapon = Slots[SlotIndex];
	if (!Weapon) return false;

	// Stop firing before drop
	Weapon->StopFire();

	// Spawn a pickup at the owner's feet
	SpawnPickupForWeapon(Weapon);

	// Detach and destroy the weapon actor
	Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Weapon->Destroy();
	Slots[SlotIndex] = nullptr;

	// If we dropped the current weapon, try to switch to another
	if (SlotIndex == CurrentSlotIndex)
	{
		AExoWeaponBase* Next = nullptr;
		int32 NextSlot = CurrentSlotIndex;
		for (int32 i = 1; i <= SlotCount; ++i)
		{
			int32 Check = (CurrentSlotIndex + i) % SlotCount;
			if (Slots[Check])
			{
				Next = Slots[Check];
				NextSlot = Check;
				break;
			}
		}

		if (Next)
		{
			CurrentSlotIndex = NextSlot;
			Next->SetActorHiddenInGame(false);
			OnWeaponChanged.Broadcast(Next, CurrentSlotIndex);
		}
		else
		{
			// All slots empty — fall back to melee
			SwitchToMelee();
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Inventory: dropped weapon from slot %d"), SlotIndex);
	return true;
}

void UExoInventoryComponent::SwapToSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= SlotCount) return;
	if (SlotIndex == CurrentSlotIndex && !bMeleeActive) return;
	if (!Slots[SlotIndex]) return;

	// Hide current weapon (could be melee or a slotted weapon)
	if (AExoWeaponBase* Old = GetCurrentWeapon())
	{
		Old->StopFire();
		Old->SetActorHiddenInGame(true);
	}

	bMeleeActive = false;
	CurrentSlotIndex = SlotIndex;
	AExoWeaponBase* New = Slots[CurrentSlotIndex];
	if (New)
	{
		New->SetActorHiddenInGame(false);
		New->PlayDrawAnimation();
	}

	OnWeaponChanged.Broadcast(New, CurrentSlotIndex);
}

void UExoInventoryComponent::CycleWeapon(int32 Direction)
{
	if (Direction == 0) return;

	int32 Step = (Direction > 0) ? 1 : -1;
	for (int32 i = 1; i <= SlotCount; ++i)
	{
		int32 Check = (CurrentSlotIndex + i * Step + SlotCount) % SlotCount;
		if (Slots[Check])
		{
			SwapToSlot(Check);
			return;
		}
	}
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

AExoWeaponBase* UExoInventoryComponent::GetWeapon(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SlotCount) return nullptr;
	return Slots[SlotIndex];
}

AExoWeaponBase* UExoInventoryComponent::GetCurrentWeapon() const
{
	if (bMeleeActive && MeleeWeapon) return MeleeWeapon;
	return GetWeapon(CurrentSlotIndex);
}

// ---------------------------------------------------------------------------
// Melee weapon
// ---------------------------------------------------------------------------

void UExoInventoryComponent::SwitchToMelee()
{
	if (!MeleeWeapon) return;
	if (bMeleeActive) return;

	// Hide current weapon
	if (AExoWeaponBase* Old = GetCurrentWeapon())
	{
		Old->StopFire();
		Old->SetActorHiddenInGame(true);
	}

	bMeleeActive = true;
	MeleeWeapon->SetActorHiddenInGame(false);
	OnWeaponChanged.Broadcast(MeleeWeapon, -1);
}

void UExoInventoryComponent::CheckAutoSwitchToMelee()
{
	if (bMeleeActive) return;

	for (int32 i = 0; i < SlotCount; ++i)
	{
		if (Slots[i] && Slots[i]->GetCurrentEnergy() > 0.f)
		{
			return; // At least one weapon has energy
		}
	}

	SwitchToMelee();
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

void UExoInventoryComponent::AttachWeaponToOwner(AExoWeaponBase* Weapon)
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwner());
	if (!Char || !Weapon) return;

	Weapon->SetOwner(Char);
	UCameraComponent* Cam = Char->GetFirstPersonCamera();
	if (Cam)
	{
		Weapon->AttachToComponent(Cam,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("GripPoint"));
	}
}

void UExoInventoryComponent::SpawnPickupForWeapon(AExoWeaponBase* Weapon)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Weapon) return;

	FVector SpawnLoc = Owner->GetActorLocation() - FVector(0.f, 0.f, 60.f);
	FActorSpawnParameters Params;
	Params.Owner = nullptr;

	AExoWeaponPickup* Pickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
		AExoWeaponPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
	if (Pickup)
	{
		Pickup->WeaponType = Weapon->GetWeaponType();
		Pickup->Rarity = Weapon->Rarity;
		Pickup->bRespawns = false;
	}
}

void UExoInventoryComponent::SpawnDefaultWeapons()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);

	// Spawn rifle into slot 0 (Primary)
	AExoWeaponRifle* Rifle = GetWorld()->SpawnActor<AExoWeaponRifle>(
		AExoWeaponRifle::StaticClass(), Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
	if (Rifle) AddWeapon(Rifle);

	// Spawn pistol into slot 1 (Secondary)
	AExoWeaponPistol* Pistol = GetWorld()->SpawnActor<AExoWeaponPistol>(
		AExoWeaponPistol::StaticClass(), Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
	if (Pistol) AddWeapon(Pistol);

	// Spawn shotgun into slot 2 (Utility)
	AExoWeaponShotgun* Shotgun = GetWorld()->SpawnActor<AExoWeaponShotgun>(
		AExoWeaponShotgun::StaticClass(), Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
	if (Shotgun) AddWeapon(Shotgun);

	// Spawn melee weapon (Plasma Blade) — always available, not in a slot
	AExoWeaponMelee* Melee = GetWorld()->SpawnActor<AExoWeaponMelee>(
		AExoWeaponMelee::StaticClass(), Owner->GetActorLocation(), Owner->GetActorRotation(), SpawnParams);
	if (Melee)
	{
		MeleeWeapon = Melee;
		AttachWeaponToOwner(Melee);
		Melee->SetActorHiddenInGame(true);
		UE_LOG(LogExoRift, Log, TEXT("Inventory: spawned Plasma Blade (melee fallback)"));
	}
}
