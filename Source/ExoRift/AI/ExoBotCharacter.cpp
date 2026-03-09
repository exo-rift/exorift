#include "AI/ExoBotCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

AExoBotCharacter::AExoBotCharacter()
{
	// Bots use third-person mesh visible to all
	GetMesh()->SetOwnerNoSee(false);
	// Disable FP arms for bots
	if (FPArms)
	{
		FPArms->SetVisibility(false);
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AExoBotCharacter::Tick(float DeltaTime)
{
	// LOD-based tick rate throttling
	float Interval = 0.f;
	switch (CurrentLODLevel)
	{
	case EAILODLevel::Full: Interval = FullTickInterval; break;
	case EAILODLevel::Simplified: Interval = SimplifiedTickInterval; break;
	case EAILODLevel::Basic: Interval = BasicTickInterval; break;
	}

	if (Interval > 0.f)
	{
		TickAccumulator += DeltaTime;
		if (TickAccumulator < Interval) return;
		TickAccumulator = 0.f;
	}

	Super::Tick(DeltaTime);
}

void AExoBotCharacter::SetAILODLevel(EAILODLevel NewLevel)
{
	if (CurrentLODLevel == NewLevel) return;

	EAILODLevel OldLevel = CurrentLODLevel;
	CurrentLODLevel = NewLevel;

	// Adjust movement at lower LODs
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (CMC)
	{
		switch (NewLevel)
		{
		case EAILODLevel::Full:
			CMC->bUseRVOAvoidance = true;
			break;
		case EAILODLevel::Simplified:
			CMC->bUseRVOAvoidance = false;
			break;
		case EAILODLevel::Basic:
			CMC->bUseRVOAvoidance = false;
			break;
		}
	}

	UE_LOG(LogExoRift, Verbose, TEXT("%s LOD: %d -> %d"), *GetName(),
		static_cast<int32>(OldLevel), static_cast<int32>(NewLevel));
}
