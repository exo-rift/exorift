#include "Player/ExoAbilityComponent.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
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
}

void UExoAbilityComponent::UseAbility(EExoAbilityType Type)
{
	for (FExoAbility& Ability : Abilities)
	{
		if (Ability.Type != Type) continue;
		if (!Ability.bIsReady()) return;

		switch (Type)
		{
		case EExoAbilityType::Dash:        ExecuteDash(); break;
		case EExoAbilityType::AreaScan:    ExecuteAreaScan(); break;
		case EExoAbilityType::ShieldBubble: ExecuteShieldBubble(); break;
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
