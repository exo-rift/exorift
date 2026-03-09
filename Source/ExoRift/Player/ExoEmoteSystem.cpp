#include "Player/ExoEmoteSystem.h"

const FEmoteData FExoEmoteSystem::EmoteTable[] =
{
	{ TEXT("Salute"),       1.5f, 0 },
	{ TEXT("Victory Pose"), 2.0f, 1 },
	{ TEXT("Taunt"),        1.0f, 2 },
	{ TEXT("Wave"),         1.5f, 3 },
	{ TEXT("Dance"),        3.0f, 4 },
	{ TEXT("Sit"),          0.0f, 5 }, // Toggle
	{ TEXT("Point"),        1.0f, 6 },
	{ TEXT("Clap"),         1.5f, 7 },
};

const int32 FExoEmoteSystem::EmoteCount = UE_ARRAY_COUNT(EmoteTable);

const FEmoteData& FExoEmoteSystem::GetEmote(int32 Index)
{
	static const FEmoteData Invalid = { TEXT("None"), 0.f, -1 };
	if (Index < 0 || Index >= EmoteCount) return Invalid;
	return EmoteTable[Index];
}

int32 FExoEmoteSystem::GetEmoteCount()
{
	return EmoteCount;
}
