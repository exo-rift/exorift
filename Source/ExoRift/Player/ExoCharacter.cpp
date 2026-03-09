#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Core/ExoGameMode.h"
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

	// Third person mesh (hidden from owner)
	GetMesh()->SetOwnerNoSee(true);

	// Movement defaults
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.2f;
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
}

float AExoCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - ActualDamage, 0.f, MaxHealth);

	if (Health <= 0.f)
	{
		FString WeaponName = TEXT("Unknown");
		if (AExoWeaponBase* Weapon = Cast<AExoWeaponBase>(DamageCauser))
		{
			WeaponName = Weapon->GetWeaponName();
		}
		else if (DamageCauser && DamageCauser->IsA<AExoCharacter>())
		{
			WeaponName = TEXT("Zone");
		}
		Die(EventInstigator, WeaponName);
	}

	return ActualDamage;
}

void AExoCharacter::StartFire()
{
	if (CurrentWeapon && !bIsDead)
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

void AExoCharacter::OnRep_Health()
{
	// Client-side health change feedback (flash, sound, etc.)
}

void AExoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AExoCharacter, Health);
	DOREPLIFETIME(AExoCharacter, CurrentWeapon);
}
