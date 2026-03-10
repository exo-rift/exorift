#pragma once

#include "CoreMinimal.h"

class AExoCharacter;
class UWorld;

/**
 * Tracks nearby bullet traces and triggers screen effects
 * when a bullet passes close to the local player.
 * Pure static helper — no actor needed.
 */
struct FExoBulletWhiz
{
	/** Call after a hitscan trace to check for near-miss on local player. */
	static void CheckNearMiss(UWorld* World, const FVector& Start, const FVector& End);

	/** Distance threshold for a near-miss. */
	static constexpr float WhizRadius = 300.f;

	/** Cooldown between whiz effects to avoid spam. */
	static float LastWhizTime;
	static constexpr float WhizCooldown = 0.25f;
};
