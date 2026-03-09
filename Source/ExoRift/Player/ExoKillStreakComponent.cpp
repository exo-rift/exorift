#include "Player/ExoKillStreakComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

UExoKillStreakComponent::UExoKillStreakComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UExoKillStreakComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bStreakActive) return;

	StreakTimer -= DeltaTime;
	if (StreakTimer <= 0.f)
	{
		// Streak expired
		ResetStreak();
	}
}

void UExoKillStreakComponent::RegisterKill()
{
	CurrentStreak++;
	StreakTimer = StreakTimeWindow;
	bStreakActive = true;

	UpdateStreakBonuses();

	FString Name = GetStreakName();
	OnStreakChanged.Broadcast(CurrentStreak, Name);

	if (!Name.IsEmpty())
	{
		UE_LOG(LogExoRift, Log, TEXT("Kill streak: %d — %s"), CurrentStreak, *Name);
	}
}

FString UExoKillStreakComponent::GetStreakName() const
{
	if (CurrentStreak >= 8) return TEXT("Godlike");
	if (CurrentStreak >= 5) return TEXT("Unstoppable");
	if (CurrentStreak >= 3) return TEXT("On Fire");
	return FString();
}

void UExoKillStreakComponent::UpdateStreakBonuses()
{
	float OldSpeed = SpeedMultiplier;
	float OldDamage = DamageMultiplier;

	// Reset to defaults, then apply tier
	SpeedMultiplier = 1.f;
	DamageMultiplier = 1.f;

	if (CurrentStreak >= 8)
	{
		// Godlike: 10% speed + 15% damage
		SpeedMultiplier = 1.10f;
		DamageMultiplier = 1.15f;
	}
	else if (CurrentStreak >= 5)
	{
		// Unstoppable: 15% damage
		DamageMultiplier = 1.15f;
	}
	else if (CurrentStreak >= 3)
	{
		// On Fire: 10% speed
		SpeedMultiplier = 1.10f;
	}

	// Apply or revert speed changes
	if (FMath::Abs(SpeedMultiplier - OldSpeed) > KINDA_SMALL_NUMBER)
	{
		if (bSpeedApplied)
		{
			RevertSpeedBonus();
		}
		if (SpeedMultiplier > 1.f)
		{
			ApplySpeedBonus();
		}
	}
}

void UExoKillStreakComponent::ResetStreak()
{
	int32 OldStreak = CurrentStreak;
	CurrentStreak = 0;
	bStreakActive = false;
	StreakTimer = 0.f;

	RevertSpeedBonus();
	DamageMultiplier = 1.f;
	SpeedMultiplier = 1.f;

	if (OldStreak >= 3)
	{
		OnStreakChanged.Broadcast(0, FString());
		UE_LOG(LogExoRift, Log, TEXT("Kill streak ended at %d"), OldStreak);
	}
}

void UExoKillStreakComponent::ApplySpeedBonus()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	UCharacterMovementComponent* CMC = Owner->GetCharacterMovement();
	if (!CMC) return;

	BaseWalkSpeed = CMC->MaxWalkSpeed;
	CMC->MaxWalkSpeed = BaseWalkSpeed * SpeedMultiplier;
	bSpeedApplied = true;
}

void UExoKillStreakComponent::RevertSpeedBonus()
{
	if (!bSpeedApplied) return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	UCharacterMovementComponent* CMC = Owner->GetCharacterMovement();
	if (!CMC) return;

	CMC->MaxWalkSpeed = BaseWalkSpeed;
	bSpeedApplied = false;
}
