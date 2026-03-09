#include "Player/ExoAbilityComponent.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoDecoyActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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
		return;
	}

	GrappleTimer += DeltaTime;
	float Alpha = FMath::Clamp(GrappleTimer / GrappleDuration, 0.f, 1.f);

	// Smooth ease-out curve for natural feeling pull
	float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);
	FVector NewLocation = FMath::Lerp(GrappleStartLocation, GrappleTarget, EasedAlpha);
	Char->SetActorLocation(NewLocation);

	if (Alpha >= 1.f)
	{
		bIsGrappling = false;

		// Restore gravity
		UCharacterMovementComponent* Movement = Char->GetCharacterMovement();
		if (Movement)
		{
			Movement->GravityScale = 1.f;
		}

		// Give a small forward momentum on arrival
		FVector ArrivalDir = (GrappleTarget - GrappleStartLocation).GetSafeNormal();
		Char->LaunchCharacter(ArrivalDir * 400.f, false, false);
	}
}
