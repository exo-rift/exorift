// ExoCharacterCombat.cpp — DBNO, revive, execution, and death systems
#include "Player/ExoCharacter.h"
#include "Player/ExoPlayerController.h"
#include "Core/ExoGameMode.h"
#include "Core/ExoPlayerState.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

void AExoCharacter::EnterDBNO()
{
	if (bIsDBNO || bIsDead) return;
	bIsDBNO = true;
	DBNOHealthRemaining = DBNOHealth;
	DBNOTimer = 0.f;
	bIsBeingRevived = false;
	ReviveProgress = 0.f;
	CurrentReviver = nullptr;
	StopFire();
	StopSprint();
	if (bIsSliding) StopSlide();
	GetCharacterMovement()->MaxWalkSpeed = DBNOCrawlSpeed;
	UE_LOG(LogExoRift, Log, TEXT("%s is DBNO"), *GetName());
}

void AExoCharacter::TickDBNO(float DeltaTime)
{
	if (!bIsDBNO || bIsDead) return;

	// Bleed damage over time
	DBNOHealthRemaining -= DBNOBleedRate * DeltaTime;

	// Duration-based bleed-out timer (auto-eliminate after DBNODuration seconds)
	DBNOTimer += DeltaTime;

	if (DBNOHealthRemaining <= 0.f || DBNOTimer >= DBNODuration)
	{
		DBNOHealthRemaining = 0.f;
		// Kill credit goes to whoever downed this player (LastDamageInstigator)
		Die(LastDamageInstigator, LastDamageWeaponName);
		return;
	}

	// Tick revive progress if a teammate is reviving us
	if (bIsBeingRevived && CurrentReviver)
	{
		ReviveProgress += DeltaTime / ReviveTime;
		if (ReviveProgress >= 1.f)
		{
			CompleteRevive();
		}
	}
}

// --- Revive ---

void AExoCharacter::StartRevive(AExoCharacter* Reviver)
{
	if (!bIsDBNO || bIsDead || !Reviver || bIsBeingRevived) return;
	bIsBeingRevived = true;
	ReviveProgress = 0.f;
	CurrentReviver = Reviver;
	UE_LOG(LogExoRift, Log, TEXT("%s is being revived by %s"), *GetName(), *Reviver->GetName());
}

void AExoCharacter::StopRevive()
{
	if (!bIsBeingRevived) return;
	bIsBeingRevived = false;
	ReviveProgress = 0.f;
	CurrentReviver = nullptr;
}

void AExoCharacter::CompleteRevive()
{
	if (!bIsDBNO || bIsDead) return;

	// Credit the reviver before clearing the reference
	if (CurrentReviver && CurrentReviver->GetController())
	{
		if (AExoPlayerState* RPS = CurrentReviver->GetController()->GetPlayerState<AExoPlayerState>())
			RPS->Revives++;
	}

	bIsDBNO = false;
	bIsBeingRevived = false;
	ReviveProgress = 0.f;
	DBNOTimer = 0.f;
	DBNOHealthRemaining = 0.f;
	CurrentReviver = nullptr;

	Health = ReviveHealthRestore;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	UE_LOG(LogExoRift, Log, TEXT("%s has been revived with %.0f HP"), *GetName(), Health);
}

// --- Execution (Finisher) ---

void AExoCharacter::StartExecution(AExoCharacter* Target)
{
	if (!Target || !Target->IsDBNO() || Target->bIsDead) return;
	if (bIsDead || bIsDBNO || bIsExecuting) return;

	bIsExecuting = true;
	ExecutionTarget = Target;
	ExecutionProgress = 0.f;

	// Lock both players — stop movement and firing
	StopFire();
	StopSprint();
	GetCharacterMovement()->DisableMovement();

	// Stop the target from being revived or crawling
	Target->StopRevive();
	Target->GetCharacterMovement()->DisableMovement();

	UE_LOG(LogExoRift, Log, TEXT("%s executing %s"), *GetName(), *Target->GetName());
}

void AExoCharacter::CancelExecution()
{
	if (!bIsExecuting) return;

	// Restore executor movement
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;

	// Restore target crawl movement if still alive
	if (ExecutionTarget && ExecutionTarget->IsDBNO() && !ExecutionTarget->bIsDead)
	{
		ExecutionTarget->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		ExecutionTarget->GetCharacterMovement()->MaxWalkSpeed = ExecutionTarget->DBNOCrawlSpeed;
	}

	bIsExecuting = false;
	ExecutionTarget = nullptr;
	ExecutionProgress = 0.f;
}

void AExoCharacter::TickExecution(float DeltaTime)
{
	if (!bIsExecuting || !ExecutionTarget) return;

	// If target died during execution (e.g. from bleed), cancel
	if (ExecutionTarget->bIsDead)
	{
		CancelExecution();
		return;
	}

	ExecutionProgress += DeltaTime / ExecutionDuration;
	if (ExecutionProgress >= 1.f)
	{
		ExecutionProgress = 1.f;
		AExoCharacter* Target = ExecutionTarget;

		// Restore executor state before target dies (Die may trigger game logic)
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
		bIsExecuting = false;
		ExecutionTarget = nullptr;

		// Eliminate the target — executor gets kill credit
		Target->Die(GetController(), TEXT("Execution"));
		UE_LOG(LogExoRift, Log, TEXT("%s finished executing %s"), *GetName(), *Target->GetName());
	}
}

void AExoCharacter::Die(AController* Killer, const FString& WeaponName)
{
	if (bIsDead) return;
	bIsDead = true;

	StopFire();

	// Ragdoll
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

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
