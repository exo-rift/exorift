#include "Player/ExoEmoteComponent.h"
#include "Player/ExoEmoteSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

UExoEmoteComponent::UExoEmoteComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Ticked manually by character
}

void UExoEmoteComponent::PlayEmote(int32 EmoteIndex)
{
	if (bIsEmoting) CancelEmote();

	const FEmoteData& Emote = FExoEmoteSystem::GetEmote(EmoteIndex);
	if (Emote.AnimIndex < 0) return; // Invalid emote

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	bIsEmoting = true;
	CurrentEmoteIndex = EmoteIndex;
	EmoteTimer = Emote.Duration;

	// Lock movement during emote (toggle emotes like Sit stay until cancelled)
	Owner->GetCharacterMovement()->DisableMovement();

	UE_LOG(LogExoRift, Log, TEXT("Emote started: %s (%.1fs)"), *Emote.Name, Emote.Duration);
}

void UExoEmoteComponent::CancelEmote()
{
	if (!bIsEmoting) return;
	EndEmote();
	UE_LOG(LogExoRift, Log, TEXT("Emote cancelled"));
}

void UExoEmoteComponent::TickEmote(float DeltaTime)
{
	if (!bIsEmoting) return;

	// Toggle emotes (Duration == 0) stay active until manually cancelled
	if (EmoteTimer <= 0.f) return;

	EmoteTimer -= DeltaTime;
	if (EmoteTimer <= 0.f)
	{
		EndEmote();
	}
}

void UExoEmoteComponent::EndEmote()
{
	bIsEmoting = false;
	CurrentEmoteIndex = -1;
	EmoteTimer = 0.f;

	// Restore movement
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (Owner)
	{
		Owner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}
