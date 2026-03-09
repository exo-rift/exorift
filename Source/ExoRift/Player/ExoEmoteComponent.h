#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoEmoteComponent.generated.h"

/**
 * Component attached to ExoCharacter that handles emote playback.
 * Locks movement for the emote duration; interrupted by damage or input.
 */
UCLASS()
class EXORIFT_API UExoEmoteComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoEmoteComponent();

	/** Start playing an emote by global table index. */
	void PlayEmote(int32 EmoteIndex);

	/** Cancel the current emote (e.g. took damage, player moved). */
	void CancelEmote();

	/** Called each frame to tick down emote duration. */
	void TickEmote(float DeltaTime);

	bool IsEmoting() const { return bIsEmoting; }
	int32 GetCurrentEmoteIndex() const { return CurrentEmoteIndex; }

private:
	bool bIsEmoting = false;
	int32 CurrentEmoteIndex = -1;
	float EmoteTimer = 0.f;

	void EndEmote();
};
