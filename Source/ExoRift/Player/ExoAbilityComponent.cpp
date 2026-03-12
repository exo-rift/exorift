#include "Player/ExoAbilityComponent.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoDecoyActor.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoScanPulseActor.h"
#include "Visual/ExoShieldBubbleActor.h"
#include "Visual/ExoScreenShake.h"
#include "Core/ExoAudioManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ExoRift.h"

UExoAbilityComponent::UExoAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UExoAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize default abilities
	FExoAbility Dash;
	Dash.Type = EExoAbilityType::Dash;
	Dash.Cooldown = 8.f;
	Dash.CooldownRemaining = 0.f;
	Dash.AbilityName = TEXT("Dash");
	Abilities.Add(Dash);

	FExoAbility Scan;
	Scan.Type = EExoAbilityType::AreaScan;
	Scan.Cooldown = 20.f;
	Scan.CooldownRemaining = 0.f;
	Scan.AbilityName = TEXT("Area Scan");
	Abilities.Add(Scan);

	FExoAbility Shield;
	Shield.Type = EExoAbilityType::ShieldBubble;
	Shield.Cooldown = 30.f;
	Shield.CooldownRemaining = 0.f;
	Shield.AbilityName = TEXT("Shield Bubble");
	Abilities.Add(Shield);

	FExoAbility Grapple;
	Grapple.Type = EExoAbilityType::GrappleHook;
	Grapple.Cooldown = 15.f;
	Grapple.CooldownRemaining = 0.f;
	Grapple.AbilityName = TEXT("Grapple Hook");
	Abilities.Add(Grapple);

	FExoAbility DecoyAbility;
	DecoyAbility.Type = EExoAbilityType::Decoy;
	DecoyAbility.Cooldown = 25.f;
	DecoyAbility.CooldownRemaining = 0.f;
	DecoyAbility.AbilityName = TEXT("Decoy");
	Abilities.Add(DecoyAbility);
}

void UExoAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Reduce cooldown timers
	for (FExoAbility& Ability : Abilities)
	{
		if (Ability.CooldownRemaining > 0.f)
		{
			float PrevCooldown = Ability.CooldownRemaining;
			Ability.CooldownRemaining = FMath::Max(Ability.CooldownRemaining - DeltaTime, 0.f);

			if (PrevCooldown > 0.f && Ability.CooldownRemaining <= 0.f)
			{
				OnAbilityCooldownComplete.Broadcast(Ability.Type);
			}
		}
	}

	TickAreaScan(DeltaTime);
	TickGrapple(DeltaTime);
}

void UExoAbilityComponent::UseAbility(EExoAbilityType Type)
{
	// Grapple blocks other abilities while active
	if (bIsGrappling && Type != EExoAbilityType::GrappleHook) return;

	for (FExoAbility& Ability : Abilities)
	{
		if (Ability.Type != Type) continue;
		if (!Ability.bIsReady()) return;

		switch (Type)
		{
		case EExoAbilityType::Dash:         ExecuteDash(); break;
		case EExoAbilityType::AreaScan:     ExecuteAreaScan(); break;
		case EExoAbilityType::ShieldBubble: ExecuteShieldBubble(); break;
		case EExoAbilityType::GrappleHook:  ExecuteGrapple(); break;
		case EExoAbilityType::Decoy:        ExecuteDecoy(); break;
		}

		Ability.CooldownRemaining = Ability.Cooldown;
		OnAbilityUsed.Broadcast(Type);

		UE_LOG(LogExoRift, Log, TEXT("Ability used: %s"), *Ability.AbilityName);
		return;
	}
}

void UExoAbilityComponent::ExecuteDash()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	FVector LookDir = Char->GetControlRotation().Vector();
	LookDir.Z = 0.f;
	LookDir.Normalize();

	Char->LaunchCharacter(LookDir * DashImpulse, true, false);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayAbilityActivateSound(Char->GetActorLocation(), 1.2f);
	}

	if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
	{
		PP->TriggerDashEffect();
	}

	FExoScreenShake::AddShake(0.25f, 0.12f);
}

void UExoAbilityComponent::ExecuteAreaScan()
{
	ScannedEnemies.Empty();
	ScanTimeRemaining = ScanDuration;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Origin = Owner->GetActorLocation();

	// Find all characters within scan radius
	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Other = *It;
		if (Other == Owner) continue;
		if (!Other->IsAlive()) continue;

		float Dist = FVector::Dist(Origin, Other->GetActorLocation());
		if (Dist <= ScanRadius)
		{
			ScannedEnemies.Add(Other);
		}
	}

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayAbilityActivateSound(Origin, 0.8f);
	}

	if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
	{
		PP->TriggerScanPulse();
	}

	// World-space expanding ring
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoScanPulseActor* Pulse = GetWorld()->SpawnActor<AExoScanPulseActor>(
			AExoScanPulseActor::StaticClass(), Origin, FRotator::ZeroRotator, SpawnParams);
		if (Pulse) Pulse->Init(ScanRadius);
	}

	FExoScreenShake::AddShake(0.2f, 0.15f);

	UE_LOG(LogExoRift, Log, TEXT("AreaScan revealed %d enemies"), ScannedEnemies.Num());
}

void UExoAbilityComponent::ExecuteShieldBubble()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwner());
	if (!Char) return;

	UExoShieldComponent* Shield = Char->GetShieldComponent();
	if (Shield)
	{
		Shield->AddShield(ShieldRestoreAmount);
	}

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayAbilityActivateSound(Char->GetActorLocation(), 0.6f);
	}

	if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
	{
		PP->TriggerShieldFlash();
	}

	// World-space shield dome
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoShieldBubbleActor* Bubble = GetWorld()->SpawnActor<AExoShieldBubbleActor>(
			AExoShieldBubbleActor::StaticClass(),
			Char->GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Bubble) Bubble->Init();
	}

	FExoScreenShake::AddShake(0.15f, 0.1f);
}

void UExoAbilityComponent::ExecuteGrapple()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	FVector Start = Char->GetActorLocation();
	FVector Forward = Char->GetControlRotation().Vector();
	FVector End = Start + Forward * GrappleRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_WorldStatic, Params);

	if (bHit)
	{
		bIsGrappling = true;
		GrappleTarget = Hit.ImpactPoint;
		GrappleStartLocation = Start;
		GrappleTimer = 0.f;

		// Disable gravity while grappling for smooth traversal
		UCharacterMovementComponent* Movement = Char->GetCharacterMovement();
		if (Movement)
		{
			Movement->GravityScale = 0.f;
		}

		CreateGrappleBeam();

		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayAbilityActivateSound(Start, 1.0f);
		}

		if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
		{
			PP->ApplyGrappleEffect(1.f);
		}

		UE_LOG(LogExoRift, Log, TEXT("Grapple attached at distance %.0f"),
			FVector::Dist(Start, GrappleTarget));
	}
}

void UExoAbilityComponent::ExecuteDecoy()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SpawnLoc = Owner->GetActorLocation();
	FRotator SpawnRot = Owner->GetActorRotation();

	AExoDecoyActor* Decoy = World->SpawnActor<AExoDecoyActor>(
		AExoDecoyActor::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);

	if (Decoy)
	{
		Decoy->SetLifeSpan(DecoyLifetime);

		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayAbilityActivateSound(SpawnLoc, 1.4f);
		}

		UE_LOG(LogExoRift, Log, TEXT("Decoy deployed, auto-destroy in %.0fs"), DecoyLifetime);
	}
}

void UExoAbilityComponent::TickAreaScan(float DeltaTime)
{
	if (ScanTimeRemaining <= 0.f) return;

	ScanTimeRemaining -= DeltaTime;
	if (ScanTimeRemaining <= 0.f)
	{
		ScannedEnemies.Empty();
		ScanTimeRemaining = 0.f;
	}
}

void UExoAbilityComponent::TickGrapple(float DeltaTime)
{
	if (!bIsGrappling) return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char)
	{
		bIsGrappling = false;
		DestroyGrappleBeam();
		return;
	}

	GrappleTimer += DeltaTime;
	float Alpha = FMath::Clamp(GrappleTimer / GrappleDuration, 0.f, 1.f);

	float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);
	FVector NewLocation = FMath::Lerp(GrappleStartLocation, GrappleTarget, EasedAlpha);
	Char->SetActorLocation(NewLocation);

	UpdateGrappleBeam();

	if (Alpha >= 1.f)
	{
		bIsGrappling = false;
		DestroyGrappleBeam();

		UCharacterMovementComponent* Movement = Char->GetCharacterMovement();
		if (Movement)
		{
			Movement->GravityScale = 1.f;
		}

		FVector ArrivalDir = (GrappleTarget - GrappleStartLocation).GetSafeNormal();
		Char->LaunchCharacter(ArrivalDir * 400.f, false, false);

		if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
		{
			PP->ApplyGrappleEffect(0.f);
		}
	}
}

// Grapple beam VFX: CreateGrappleBeam, UpdateGrappleBeam, DestroyGrappleBeam
// are in ExoAbilityGrappleVFX.cpp
