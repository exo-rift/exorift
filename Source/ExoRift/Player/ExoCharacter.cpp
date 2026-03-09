#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Core/ExoGameMode.h"
#include "Visual/ExoPostProcess.h"
#include "UI/ExoHitMarker.h"
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

	// Interaction
	InteractionComp = CreateDefaultSubobject<UExoInteractionComponent>(TEXT("InteractionComp"));

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
	SpawnDefaultWeapons();
}

void AExoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickSlide(DeltaTime);
	TickMantle(DeltaTime);

	// Update post-process low health effect
	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP)
		{
			PP->SetLowHealthEffect(Health / MaxHealth);
		}

		FExoHitMarker::Tick(DeltaTime);
	}
}

float AExoCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Shield absorbs first
	if (ShieldComp && ShieldComp->HasShield())
	{
		ActualDamage = ShieldComp->AbsorbDamage(ActualDamage);
	}

	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);

	// Visual feedback for the damaged player
	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP)
		{
			PP->TriggerDamageFlash(FMath::Clamp(DamageAmount / 30.f, 0.2f, 1.f));
		}

		// Damage direction indicator
		if (DamageCauser && DamageCauser != this)
		{
			FVector DmgDir = DamageCauser->GetActorLocation() - GetActorLocation();
			FRotator DmgRot = DmgDir.Rotation();
			float RelativeAngle = (DmgRot.Yaw - GetControlRotation().Yaw);
			FExoHitMarker::AddDamageIndicator(RelativeAngle);
		}
	}

	if (Health <= 0.f)
	{
		FString WeaponName = TEXT("Unknown");
		if (AExoWeaponBase* Weapon = Cast<AExoWeaponBase>(DamageCauser))
		{
			WeaponName = Weapon->GetWeaponName();
		}
		else if (!Cast<AExoWeaponBase>(DamageCauser))
		{
			WeaponName = TEXT("Zone");
		}
		Die(EventInstigator, WeaponName);
	}

	return ActualDamage;
}

void AExoCharacter::StartFire()
{
	if (CurrentWeapon && !bIsDead && !bIsSliding && !bIsMantling)
	{
		CurrentWeapon->StartFire();
	}
}

void AExoCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void AExoCharacter::SwapWeapon()
{
	if (WeaponInventory.Num() <= 1 || bIsDead) return;

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	CurrentWeaponIndex = (CurrentWeaponIndex + 1) % WeaponInventory.Num();
	CurrentWeapon = WeaponInventory[CurrentWeaponIndex];
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(false);
	}
}

void AExoCharacter::EquipWeapon(AExoWeaponBase* Weapon)
{
	if (!Weapon) return;

	Weapon->SetOwner(this);
	Weapon->AttachToComponent(FirstPersonCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("GripPoint"));
	WeaponInventory.Add(Weapon);

	if (!CurrentWeapon)
	{
		CurrentWeapon = Weapon;
		CurrentWeaponIndex = 0;
	}
	else
	{
		Weapon->SetActorHiddenInGame(true);
	}
}

void AExoCharacter::StartSprint()
{
	if (!bIsDead)
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

void AExoCharacter::SpawnDefaultWeapons()
{
	if (!HasAuthority()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	// Spawn rifle
	AExoWeaponRifle* Rifle = GetWorld()->SpawnActor<AExoWeaponRifle>(AExoWeaponRifle::StaticClass(),
		GetActorLocation(), GetActorRotation(), SpawnParams);
	if (Rifle) EquipWeapon(Rifle);

	// Spawn pistol
	AExoWeaponPistol* Pistol = GetWorld()->SpawnActor<AExoWeaponPistol>(AExoWeaponPistol::StaticClass(),
		GetActorLocation(), GetActorRotation(), SpawnParams);
	if (Pistol) EquipWeapon(Pistol);
}

// --- Sliding ---

void AExoCharacter::StartSlide()
{
	if (bIsSliding || bIsDead || bIsMantling) return;
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
	if (bIsMantling || bIsSliding || bIsDead) return;
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

void AExoCharacter::OnRep_Health()
{
	// Client-side: trigger post-process on health change
	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP)
		{
			PP->SetLowHealthEffect(Health / MaxHealth);
		}
	}
}

void AExoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExoCharacter, Health);
	DOREPLIFETIME(AExoCharacter, CurrentWeapon);
}
