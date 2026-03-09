#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoArmorComponent.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoAbilityComponent.h"
#include "Player/ExoKillStreakComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Player/ExoPlayerController.h"
#include "Weapons/ExoGrenadeComponent.h"
#include "Core/ExoAudioManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Core/ExoGameMode.h"
#include "Visual/ExoPostProcess.h"
#include "UI/ExoHitMarker.h"
#include "UI/ExoPingSystem.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "ExoRift.h"

AExoCharacter::AExoCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	// First person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// FP arms mesh (owner only)
	FPArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArms"));
	FPArms->SetupAttachment(FirstPersonCamera);
	FPArms->SetOnlyOwnerSee(true);
	FPArms->bCastDynamicShadow = false;
	FPArms->CastShadow = false;

	// Shield
	ShieldComp = CreateDefaultSubobject<UExoShieldComponent>(TEXT("ShieldComp"));

	// Armor
	ArmorComp = CreateDefaultSubobject<UExoArmorComponent>(TEXT("ArmorComp"));

	// Interaction
	InteractionComp = CreateDefaultSubobject<UExoInteractionComponent>(TEXT("InteractionComp"));

	// Abilities
	AbilityComp = CreateDefaultSubobject<UExoAbilityComponent>(TEXT("AbilityComp"));

	// Kill Streak
	KillStreakComp = CreateDefaultSubobject<UExoKillStreakComponent>(TEXT("KillStreakComp"));

	// Inventory
	InventoryComp = CreateDefaultSubobject<UExoInventoryComponent>(TEXT("InventoryComp"));

	// Grenades
	GrenadeComp = CreateDefaultSubobject<UExoGrenadeComponent>(TEXT("GrenadeComp"));

	// Third person mesh (hidden from owner)
	GetMesh()->SetOwnerNoSee(true);

	// Movement defaults
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Cache default capsule height for slide restoration
	DefaultCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	DefaultCameraZ = FirstPersonCamera->GetRelativeLocation().Z;
}

void AExoCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	// Default weapons are now spawned by UExoInventoryComponent::BeginPlay
}

void AExoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickDBNO(DeltaTime);
	TickExecution(DeltaTime);
	TickSlide(DeltaTime);
	TickMantle(DeltaTime);
	TickFootsteps(DeltaTime);

	// Update post-process low health effect
	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP)
		{
			PP->SetLowHealthEffect(Health / MaxHealth);
		}

		FExoHitMarker::Tick(DeltaTime);
		FExoPingSystem::Tick(DeltaTime);
	}
}

float AExoCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Track last damage source for DBNO bleed-out attribution
	LastDamageInstigator = EventInstigator;
	if (AExoWeaponBase* W = Cast<AExoWeaponBase>(DamageCauser)) LastDamageWeaponName = W->GetWeaponName();
	else LastDamageWeaponName = TEXT("Zone");

	// If already DBNO, damage goes to DBNO health
	if (bIsDBNO)
	{
		DBNOHealthRemaining = FMath::Max(DBNOHealthRemaining - ActualDamage, 0.f);
		if (DBNOHealthRemaining <= 0.f) Die(EventInstigator, LastDamageWeaponName);
		return ActualDamage;
	}

	// Armor vest absorbs body damage, then shield absorbs remainder
	if (ArmorComp) ActualDamage = ArmorComp->AbsorbBodyDamage(ActualDamage);
	if (ShieldComp && ShieldComp->HasShield()) ActualDamage = ShieldComp->AbsorbDamage(ActualDamage);
	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);

	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP) PP->TriggerDamageFlash(FMath::Clamp(DamageAmount / 30.f, 0.2f, 1.f));
		if (DamageCauser && DamageCauser != this)
		{
			float RelativeAngle = (DamageCauser->GetActorLocation() - GetActorLocation()).Rotation().Yaw
				- GetControlRotation().Yaw;
			FExoHitMarker::AddDamageIndicator(RelativeAngle);
		}
	}

	// If executor takes damage, cancel the execution
	if (bIsExecuting) CancelExecution();

	if (Health <= 0.f) EnterDBNO();

	return ActualDamage;
}

void AExoCharacter::StartFire()
{
	AExoWeaponBase* Weapon = GetCurrentWeapon();
	if (Weapon && !bIsDead && !bIsDBNO && !bIsSliding && !bIsMantling && !bIsExecuting) Weapon->StartFire();
}

void AExoCharacter::StopFire()
{
	AExoWeaponBase* Weapon = GetCurrentWeapon();
	if (Weapon) Weapon->StopFire();
}

AExoWeaponBase* AExoCharacter::GetCurrentWeapon() const
{
	return InventoryComp ? InventoryComp->GetCurrentWeapon() : nullptr;
}

void AExoCharacter::SwapWeapon()
{
	if (bIsDead || bIsDBNO || bIsExecuting || !InventoryComp) return;
	InventoryComp->CycleWeapon(1);
}

void AExoCharacter::EquipWeapon(AExoWeaponBase* Weapon)
{
	if (!Weapon || !InventoryComp) return;
	InventoryComp->AddWeapon(Weapon);
}

void AExoCharacter::ThrowGrenade()
{
	if (!GrenadeComp || !CanPerformActions()) return;
	if (!FirstPersonCamera) return;

	FVector Origin = FirstPersonCamera->GetComponentLocation();
	FRotator Direction = FirstPersonCamera->GetComponentRotation();
	GrenadeComp->ThrowGrenade(Origin, Direction);
}

void AExoCharacter::StartSprint()
{
	if (!bIsDead && !bIsDBNO && !bIsExecuting)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * SprintSpeedMultiplier;
		StopFire();
	}
}

void AExoCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

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

// Slide, Mantle, and Footstep implementations are in ExoCharacterMovement.cpp

void AExoCharacter::OnRep_Health()
{
	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP) PP->SetLowHealthEffect(Health / MaxHealth);
	}
}

void AExoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExoCharacter, Health);
}
