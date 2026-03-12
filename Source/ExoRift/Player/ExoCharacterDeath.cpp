// ExoCharacterDeath.cpp — Death, death box spawning, and elimination logic
#include "Player/ExoCharacter.h"
#include "Player/ExoPlayerController.h"
#include "Player/ExoInventoryComponent.h"
#include "Player/ExoKillStreakComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoDeathBox.h"
#include "Core/ExoGameMode.h"
#include "Core/ExoPlayerState.h"
#include "Visual/ExoDeathEffect.h"
#include "Visual/ExoDBNOBeacon.h"
#include "Visual/ExoKillScorch.h"
#include "Visual/ExoScreenShake.h"
#include "UI/ExoPickupNotification.h"
#include "UI/ExoKillAnnouncer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

void AExoCharacter::Die(AController* Killer, const FString& WeaponName)
{
	if (bIsDead) return;
	bIsDead = true;

	if (DBNOBeacon) { DBNOBeacon->DestroyBeacon(); DBNOBeacon = nullptr; }
	DestroyReviveRingEffect();
	StopFire();

	// Ragdoll
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	// Collect weapon info, then spawn a death box containing all loot
	if (InventoryComp)
	{
		TArray<TPair<EWeaponType, EWeaponRarity>> WeaponList;
		for (int32 i = 0; i < InventoryComp->GetSlotCount(); i++)
		{
			AExoWeaponBase* W = InventoryComp->GetWeapon(i);
			if (W)
			{
				WeaponList.Add(TPair<EWeaponType, EWeaponRarity>(W->GetWeaponType(), W->Rarity));
				W->StopFire();
				W->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				W->Destroy();
			}
		}

		if (WeaponList.Num() > 0)
		{
			FActorSpawnParameters BoxParams;
			BoxParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AExoDeathBox* Box = GetWorld()->SpawnActor<AExoDeathBox>(
				AExoDeathBox::StaticClass(),
				GetActorLocation() + FVector(0.f, 0.f, 30.f),
				FRotator::ZeroRotator, BoxParams);
			if (Box)
			{
				FString Name = GetPlayerState()
					? GetPlayerState()->GetPlayerName() : GetName();
				Box->InitFromPlayer(Name, WeaponList);
			}
		}
	}

	// Death energy burst effect
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoDeathEffect* FX = GetWorld()->SpawnActor<AExoDeathEffect>(
			AExoDeathEffect::StaticClass(),
			GetActorLocation() + FVector(0.f, 0.f, 50.f),
			FRotator::ZeroRotator, SpawnParams);
		if (FX)
		{
			FLinearColor AccentCol(0.2f, 0.5f, 1.f);
			FX->Init(AccentCol);
		}
	}

	// Persistent kill scorch mark on the ground
	AExoKillScorch::SpawnScorch(GetWorld(), GetActorLocation(), false);

	// Death screen shake for nearby players
	FExoScreenShake::AddExplosionShake(GetActorLocation(),
		GetActorLocation(), 3000.f, 0.3f);

	// Register kill streak for the killer
	if (Killer)
	{
		AExoCharacter* KillerChar = Cast<AExoCharacter>(Killer->GetPawn());
		if (KillerChar && KillerChar->KillStreakComp)
		{
			KillerChar->KillStreakComp->RegisterKill();
			int32 Streak = KillerChar->KillStreakComp->GetCurrentStreak();

			if (KillerChar->IsLocallyControlled())
			{
				FExoKillAnnouncer::RegisterKill();
			}

			if (KillerChar->IsLocallyControlled() && Streak >= 2)
			{
				FString StreakText;
				if (Streak == 2) StreakText = TEXT("Double Kill!");
				else if (Streak == 3) StreakText = TEXT("Triple Kill!");
				else if (Streak == 4) StreakText = TEXT("Quad Kill!");
				else if (Streak >= 5) StreakText = KillerChar->KillStreakComp->GetStreakName();

				if (!StreakText.IsEmpty())
				{
					FExoPickupNotification::ShowElimination(StreakText, false);
				}
			}
		}
	}

	// Notify game mode
	if (AExoGameMode* GM = GetWorld()->GetAuthGameMode<AExoGameMode>())
	{
		GM->OnPlayerEliminated(GetController(), Killer, WeaponName);
	}

	// Transition the owning player controller to spectator mode
	if (AExoPlayerController* ExoPC = Cast<AExoPlayerController>(GetController()))
	{
		ExoPC->OnCharacterDied(Killer);
	}

	UE_LOG(LogExoRift, Log, TEXT("%s eliminated"), *GetName());
}
