#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoArmorComponent.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoAbilityComponent.h"
#include "Player/ExoKillStreakComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Player/ExoPlayerController.h"
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

	if (Health <= 0.f) EnterDBNO();

	return ActualDamage;
}

void AExoCharacter::StartFire()
{
	AExoWeaponBase* Weapon = GetCurrentWeapon();
	if (Weapon && !bIsDead && !bIsDBNO && !bIsSliding && !bIsMantling) Weapon->StartFire();
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
	if (bIsDead || bIsDBNO || !InventoryComp) return;
	InventoryComp->CycleWeapon(1);
}

void AExoCharacter::EquipWeapon(AExoWeaponBase* Weapon)
{
	if (!Weapon || !InventoryComp) return;
	InventoryComp->AddWeapon(Weapon);
}

void AExoCharacter::StartSprint()
{
	if (!bIsDead && !bIsDBNO)
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
	StopFire();
	StopSprint();
	if (bIsSliding) StopSlide();
	GetCharacterMovement()->MaxWalkSpeed = DBNOWalkSpeed;
	UE_LOG(LogExoRift, Log, TEXT("%s is DBNO"), *GetName());
}

void AExoCharacter::TickDBNO(float DeltaTime)
{
	if (!bIsDBNO || bIsDead) return;
	DBNOHealthRemaining -= DBNOBleedRate * DeltaTime;
	if (DBNOHealthRemaining <= 0.f)
	{
		DBNOHealthRemaining = 0.f;
		Die(LastDamageInstigator, LastDamageWeaponName);
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

// --- Sliding ---

void AExoCharacter::StartSlide()
{
	if (bIsSliding || bIsDead || bIsDBNO || bIsMantling) return;
	if (!GetCharacterMovement()->IsMovingOnGround()) return;

	bIsSliding = true;
	SlideTimer = 0.f;

	StopFire();

	// Half capsule height
	GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight * 0.5f);

	// Lower camera
	FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
	CamLoc.Z = DefaultCameraZ + SlideCameraOffset;
	FirstPersonCamera->SetRelativeLocation(CamLoc);

	// Burst speed in the direction of movement
	GetCharacterMovement()->MaxWalkSpeed = SlideStartSpeed;
	GetCharacterMovement()->GroundFriction = 0.f;
}

void AExoCharacter::StopSlide()
{
	if (!bIsSliding) return;
	bIsSliding = false;
	SlideTimer = 0.f;

	GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight);
	FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
	CamLoc.Z = DefaultCameraZ;
	FirstPersonCamera->SetRelativeLocation(CamLoc);

	GetCharacterMovement()->GroundFriction = 8.f;
	float Speed = bIsSprinting ? DefaultWalkSpeed * SprintSpeedMultiplier : DefaultWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AExoCharacter::TickSlide(float DeltaTime)
{
	if (!bIsSliding) return;
	SlideTimer += DeltaTime;

	float Alpha = FMath::Clamp(SlideTimer / SlideDuration, 0.f, 1.f);
	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(SlideStartSpeed, SlideEndSpeed, Alpha);

	if (SlideTimer >= SlideDuration || GetCharacterMovement()->Velocity.Size2D() < DefaultWalkSpeed * 0.5f)
	{
		StopSlide();
	}
}

// --- Mantling ---

void AExoCharacter::TryMantle()
{
	if (bIsMantling || bIsSliding || bIsDead || bIsDBNO) return;
	if (GetCharacterMovement()->IsMovingOnGround()) return; // Only while airborne

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// Forward trace from eye level to detect a wall
	FVector ForwardDir = FVector(EyeRotation.Vector().X, EyeRotation.Vector().Y, 0.f).GetSafeNormal();
	FVector TraceStart = EyeLocation;
	FVector TraceEnd = TraceStart + ForwardDir * MantleForwardTrace;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult WallHit;
	bool bHitWall = GetWorld()->LineTraceSingleByChannel(WallHit, TraceStart, TraceEnd, ECC_Visibility, Params);
	if (!bHitWall) return;

	// Trace down from above the wall to find the ledge surface
	FVector LedgeTraceStart = WallHit.ImpactPoint + ForwardDir * 10.f + FVector(0.f, 0.f, MantleReach);
	FVector LedgeTraceEnd = LedgeTraceStart - FVector(0.f, 0.f, MantleReach * 2.f);

	FHitResult LedgeHit;
	bool bFoundLedge = GetWorld()->LineTraceSingleByChannel(LedgeHit, LedgeTraceStart, LedgeTraceEnd, ECC_Visibility, Params);
	if (!bFoundLedge) return;

	// Check if ledge is within MantleReach of the player's head
	float HeadZ = GetActorLocation().Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float LedgeZ = LedgeHit.ImpactPoint.Z;
	if (LedgeZ < GetActorLocation().Z || LedgeZ > HeadZ + MantleReach) return;

	// Ledge must be roughly horizontal (walkable)
	if (LedgeHit.ImpactNormal.Z < 0.7f) return;

	// Start mantle
	bIsMantling = true;
	MantleTimer = 0.f;
	MantleStartLocation = GetActorLocation();
	MantleTarget = FVector(LedgeHit.ImpactPoint.X, LedgeHit.ImpactPoint.Y,
		LedgeZ + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.f);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->StopMovementImmediately();
}

void AExoCharacter::TickMantle(float DeltaTime)
{
	if (!bIsMantling) return;

	MantleTimer += DeltaTime;
	float Alpha = FMath::Clamp(MantleTimer / MantleDuration, 0.f, 1.f);

	// Smooth interpolation to ledge
	FVector NewLoc = FMath::Lerp(MantleStartLocation, MantleTarget, Alpha);
	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		bIsMantling = false;
		MantleTimer = 0.f;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void AExoCharacter::TickFootsteps(float DeltaTime)
{
	if (!GetCharacterMovement()->IsMovingOnGround()) return;
	if (GetVelocity().Size2D() < 50.f) return;

	FootstepInterval = bIsSprinting ? 0.35f : 0.5f;
	FootstepTimer -= DeltaTime;
	if (FootstepTimer <= 0.f)
	{
		FootstepTimer = FootstepInterval;
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayFootstepSound(GetActorLocation(), bIsSprinting);
		}
	}
}

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
