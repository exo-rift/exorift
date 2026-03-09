#pragma once

#include "CoreMinimal.h"

/**
 * Static emote data table — plain C++ (not a UObject).
 * Defines all available emotes in the game.
 */
struct FEmoteData
{
	FString Name;
	float Duration;  // 0 = toggle emote
	int32 AnimIndex;
};

class EXORIFT_API FExoEmoteSystem
{
public:
	static const FEmoteData& GetEmote(int32 Index);
	static int32 GetEmoteCount();

private:
	static const FEmoteData EmoteTable[];
	static const int32 EmoteCount;
};
