#include "Visual/ExoTracerManager.h"
#include "DrawDebugHelpers.h"

void FExoTracerManager::SpawnTracer(UWorld* World, const FVector& Start, const FVector& End, bool bIsHit)
{
	if (!World) return;

	FColor TracerColor = bIsHit ? FColor(255, 80, 80) : FColor(0, 220, 255);
	DrawDebugLine(World, Start, End, TracerColor, false, 0.1f, 0, 1.f);
}

void FExoTracerManager::SpawnMuzzleFlash(UWorld* World, const FVector& Location, const FRotator& Rotation)
{
	if (!World) return;

	// Placeholder: bright yellow point at muzzle position
	DrawDebugPoint(World, Location, 12.f, FColor(255, 220, 80), false, 0.05f);
}
