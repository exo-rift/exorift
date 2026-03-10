#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoArmorComponent.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoAbilityComponent.h"
#include "Player/ExoKillStreakComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Player/ExoEmoteComponent.h"
#include "Player/ExoPlayerController.h"
#include "Weapons/ExoGrenadeComponent.h"
#include "Weapons/ExoTrapComponent.h"
#include "Core/ExoAudioManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Core/ExoGameMode.h"
#include "Core/ExoPlayerState.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoCharacterModel.h"
#include "Visual/ExoScreenShake.h"
#include "Map/ExoZoneSystem.h"
#include "EngineUtils.h"
#include "UI/ExoHitMarker.h"
#include "UI/ExoHitDirectionIndicator.h"
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

	// Emotes
	EmoteComp = CreateDefaultSubobject<UExoEmoteComponent>(TEXT("EmoteComp"));

	// Traps
	TrapComp = CreateDefaultSubobject<UExoTrapComponent>(TEXT("TrapComp"));

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

	// Build procedural character model (visible to other players)
	UExoCharacterModel* CharModel = NewObject<UExoCharacterModel>(this);
	CharModel->SetupAttachment(GetMesh());
	CharModel->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CharModel->RegisterComponent();
	bool bIsBot = !IsPlayerControlled();
	CharModel->BuildModel(bIsBot);
}

void AExoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickDBNO(DeltaTime);
	TickExecution(DeltaTime);
	TickSlide(DeltaTime);
	TickMantle(DeltaTime);
	TickFootsteps(DeltaTime);
	TickADS(DeltaTime);
	if (EmoteComp) EmoteComp->TickEmote(DeltaTime);

	// Update post-process effects and HUD subsystems (local player only)
	if (IsLocallyControlled())
	{
		AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
		if (PP)
		{
			float HealthPct = Health / MaxHealth;
			PP->SetLowHealthEffect(HealthPct);
			PP->ApplyLowHealthPulse(HealthPct);
			PP->ApplySpeedBoostEffect((bIsSprinting || bIsSliding) ? 1.f : 0.f);

			// Zone damage visual/audio feedback
			bool bOutsideZone = false;
			for (TActorIterator<AExoZoneSystem> It(GetWorld()); It; ++It)
			{
				bOutsideZone = !(*It)->IsInsideZone(GetActorLocation());
				break;
			}
			if (bOutsideZone)
			{
				PP->ApplyZoneDamageEffect(0.7f);
				ZoneDamageAudioTimer -= DeltaTime;
				if (ZoneDamageAudioTimer <= 0.f)
				{
					ZoneDamageAudioTimer = 3.f;
					if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
						Audio->PlayZoneWarningSound();
				}
			}
			else
			{
				PP->ApplyZoneDamageEffect(0.f);
				ZoneDamageAudioTimer = 0.f;
			}
		}

		FExoHitMarker::Tick(DeltaTime);
		FExoPingSystem::Tick(DeltaTime);
		FExoHitDirectionIndicator::Tick(DeltaTime);
		FExoScreenShake::Tick(DeltaTime);

		// Apply screen shake to camera
		if (FExoScreenShake::IsShaking())
		{
			AddControllerPitchInput(FExoScreenShake::GetShakeOffset().Pitch * DeltaTime * 10.f);
			AddControllerYawInput(FExoScreenShake::GetShakeOffset().Yaw * DeltaTime * 10.f);
		}

		TickCameraBob(DeltaTime);
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
		// Screen shake proportional to damage
		FExoScreenShake::AddShake(FMath::Clamp(DamageAmount / 50.f, 0.1f, 0.8f), 0.25f);
		if (DamageCauser && DamageCauser != this)
		{
			float RelativeAngle = (DamageCauser->GetActorLocation() - GetActorLocation()).Rotation().Yaw
				- GetControlRotation().Yaw;
			FExoHitMarker::AddDamageIndicator(RelativeAngle);
			FExoHitDirectionIndicator::AddHit(DamageCauser->GetActorLocation());
		}
	}

	// If executor takes damage, cancel the execution
	if (bIsExecuting) CancelExecution();

	// Cancel any active emote on damage
	if (EmoteComp && EmoteComp->IsEmoting()) EmoteComp->CancelEmote();

	// Track damage taken stat
	if (AController* MyController = GetController())
	{
		if (AExoPlayerState* PS = MyController->GetPlayerState<AExoPlayerState>())
			PS->DamageTaken += ActualDamage;
	}

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

// --- ADS ---

void AExoCharacter::StartADS()
{
	if (bIsDead || bIsDBNO || bIsSprinting || bIsSliding || bIsExecuting) return;
	bIsADS = true;
	AExoWeaponBase* W = GetCurrentWeapon();
	if (W)
	{
		W->StartADS();
		TargetFOV = W->GetADSFOV();
	}
	else
	{
		TargetFOV = 75.f;
	}
	// Slow movement while ADS
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed * 0.6f;
}

void AExoCharacter::StopADS()
{
	bIsADS = false;
	TargetFOV = DefaultFOV;
	if (AExoWeaponBase* W = GetCurrentWeapon()) W->StopADS();
	if (!bIsSprinting)
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void AExoCharacter::ToggleFireMode()
{
	if (AExoWeaponBase* W = GetCurrentWeapon()) W->ToggleFireMode();
}

void AExoCharacter::TickADS(float DeltaTime)
{
	// Smooth FOV interpolation
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 12.f);
	if (FirstPersonCamera && FMath::Abs(FirstPersonCamera->FieldOfView - CurrentFOV) > 0.1f)
	{
		FirstPersonCamera->SetFieldOfView(CurrentFOV);
	}
}

// DBNO, Revive, Execution, and Die implementations are in ExoCharacterCombat.cpp
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
