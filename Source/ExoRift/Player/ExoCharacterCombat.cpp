// ExoCharacterCombat.cpp — DBNO, revive, and execution systems
// Die() is in ExoCharacterDeath.cpp
#include "Player/ExoCharacter.h"
#include "Player/ExoPlayerController.h"
#include "Player/ExoKillStreakComponent.h"
#include "Core/ExoPlayerState.h"
#include "Visual/ExoExecutionEffect.h"
#include "Visual/ExoKillScorch.h"
#include "Visual/ExoScreenShake.h"
#include "Visual/ExoDBNOTrail.h"
#include "Visual/ExoDBNOBeacon.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	LastDBNOTrailPos = GetActorLocation();
	DBNOTrailTimer = 0.f;
	DBNOBeacon = AExoDBNOBeacon::SpawnBeacon(GetWorld(), this);
	UE_LOG(LogExoRift, Log, TEXT("%s is DBNO"), *GetName());
}

void AExoCharacter::TickDBNO(float DeltaTime)
{
	if (!bIsDBNO || bIsDead) return;

	// Bleed damage over time
	DBNOHealthRemaining -= DBNOBleedRate * DeltaTime;

	// Duration-based bleed-out timer (auto-eliminate after DBNODuration seconds)
	DBNOTimer += DeltaTime;

	// DBNO crawl trail — energy bleed marks on ground
	DBNOTrailTimer -= DeltaTime;
	if (DBNOTrailTimer <= 0.f)
	{
		float DistMoved = FVector::Dist2D(GetActorLocation(), LastDBNOTrailPos);
		if (DistMoved > 30.f)
		{
			AExoDBNOTrail::SpawnMark(GetWorld(), GetActorLocation());
			LastDBNOTrailPos = GetActorLocation();
		}
		DBNOTrailTimer = 0.25f;
	}

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
			return;
		}
	}

	// Revive progress visual ring
	if (bIsBeingRevived && ReviveProgress > 0.f)
	{
		if (!ReviveRing) SpawnReviveRingEffect();

		if (ReviveRing)
		{
			float RingScale = 0.5f + ReviveProgress * 2.f;
			ReviveRing->SetRelativeScale3D(FVector(RingScale, RingScale, 0.02f));

			// Pulse the emissive brightness with progress
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(
				ReviveRing->GetMaterial(0)))
			{
				float Brightness = 2.f + ReviveProgress * 8.f;
				MID->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.5f * Brightness, 8.f * Brightness, 2.f * Brightness));
			}
		}
		if (ReviveLight)
		{
			ReviveLight->SetIntensity(20000.f * ReviveProgress);
		}
	}
	else if (ReviveRing)
	{
		DestroyReviveRingEffect();
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
	DestroyReviveRingEffect();
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
	DestroyReviveRingEffect();
	if (DBNOBeacon) { DBNOBeacon->DestroyBeacon(); DBNOBeacon = nullptr; }

	Health = ReviveHealthRestore;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	UE_LOG(LogExoRift, Log, TEXT("%s has been revived with %.0f HP"), *GetName(), Health);
}

// --- Revive Ring Effect ---

void AExoCharacter::SpawnReviveRingEffect()
{
	UStaticMesh* CylMesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylMesh)
	{
		ReviveRing = NewObject<UStaticMeshComponent>(this);
		ReviveRing->SetupAttachment(GetRootComponent());
		ReviveRing->SetStaticMesh(CylMesh);
		ReviveRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ReviveRing->CastShadow = false;
		ReviveRing->SetRelativeLocation(FVector(0.f, 0.f, 5.f));
		ReviveRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.02f));
		ReviveRing->RegisterComponent();

		UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
		if (AddMat)
		{
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(AddMat, this);
			if (!MID) { return; }
			MID->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.5f, 8.f, 2.f)); // Green revive glow
			ReviveRing->SetMaterial(0, MID);
		}
	}

	ReviveLight = NewObject<UPointLightComponent>(this);
	ReviveLight->SetupAttachment(GetRootComponent());
	ReviveLight->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	ReviveLight->SetIntensity(0.f);
	ReviveLight->SetAttenuationRadius(500.f);
	ReviveLight->SetLightColor(FLinearColor(0.2f, 1.f, 0.3f));
	ReviveLight->CastShadows = false;
	ReviveLight->RegisterComponent();
}

void AExoCharacter::DestroyReviveRingEffect()
{
	if (ReviveRing)
	{
		ReviveRing->DestroyComponent();
		ReviveRing = nullptr;
	}
	if (ReviveLight)
	{
		ReviveLight->DestroyComponent();
		ReviveLight = nullptr;
	}
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

		// Spawn execution-specific VFX before killing the target
		{
			FActorSpawnParameters FXParams;
			FXParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AExoExecutionEffect* ExecFX = GetWorld()->SpawnActor<AExoExecutionEffect>(
				AExoExecutionEffect::StaticClass(),
				Target->GetActorLocation() + FVector(0.f, 0.f, 50.f),
				FRotator::ZeroRotator, FXParams);
			if (ExecFX)
			{
				ExecFX->Init(FLinearColor(1.f, 0.3f, 0.1f), GetActorLocation());
			}
		}
		FExoScreenShake::AddExplosionShake(Target->GetActorLocation(),
			Target->GetActorLocation(), 4000.f, 0.5f);

		// Execution scorch (large, headshot-sized)
		AExoKillScorch::SpawnScorch(GetWorld(), Target->GetActorLocation(), true);

		// Eliminate the target — executor gets kill credit
		Target->Die(GetController(), TEXT("Execution"));
		UE_LOG(LogExoRift, Log, TEXT("%s finished executing %s"), *GetName(), *Target->GetName());
	}
}

// Die() moved to ExoCharacterDeath.cpp
