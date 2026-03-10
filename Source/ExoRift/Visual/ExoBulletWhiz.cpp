// ExoBulletWhiz.cpp — Near-miss bullet screen effect
#include "Visual/ExoBulletWhiz.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoScreenShake.h"
#include "Core/ExoAudioManager.h"
#include "Player/ExoCharacter.h"

float FExoBulletWhiz::LastWhizTime = -1.f;

void FExoBulletWhiz::CheckNearMiss(UWorld* World, const FVector& Start, const FVector& End)
{
	if (!World) return;

	// Find local player character
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;
	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	// Don't trigger on our own shots
	AExoCharacter* LocalChar = Cast<AExoCharacter>(Pawn);
	if (!LocalChar || !LocalChar->IsAlive()) return;

	// Point-to-line-segment distance
	FVector PlayerPos = Pawn->GetActorLocation() + FVector(0, 0, 80.f); // Head height
	FVector AB = End - Start;
	float ABLenSq = AB.SizeSquared();
	if (ABLenSq < 1.f) return;

	float T = FVector::DotProduct(PlayerPos - Start, AB) / ABLenSq;
	T = FMath::Clamp(T, 0.f, 1.f);
	FVector ClosestPoint = Start + AB * T;
	float Dist = FVector::Distance(PlayerPos, ClosestPoint);

	if (Dist > WhizRadius) return;
	if (Dist < 50.f) return; // Too close = actual hit, not a near miss

	// Cooldown check
	float Now = World->GetTimeSeconds();
	if (Now - LastWhizTime < WhizCooldown) return;
	LastWhizTime = Now;

	// Intensity based on closeness (closer = stronger)
	float Intensity = 1.f - (Dist / WhizRadius);
	Intensity = FMath::Clamp(Intensity, 0.f, 1.f);

	// Subtle chromatic aberration + camera nudge
	AExoPostProcess* PP = AExoPostProcess::Get(World);
	if (PP)
	{
		PP->TriggerDamageFlash(Intensity * 0.15f);
	}

	// Small directional camera shake
	FExoScreenShake::AddExplosionShake(ClosestPoint, PlayerPos,
		WhizRadius * 2.f, 0.08f * Intensity);

	// Whiz-by sound
	UExoAudioManager* Audio = UExoAudioManager::Get(World);
	if (Audio)
	{
		Audio->PlayImpactSound(ClosestPoint, false);
	}
}
