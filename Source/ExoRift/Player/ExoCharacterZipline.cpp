// ExoCharacterZipline.cpp — Zipline mount, ride, and dismount logic
#include "Player/ExoCharacter.h"
#include "Map/ExoZipline.h"
#include "Core/ExoAudioManager.h"
#include "EngineUtils.h"
#include "Visual/ExoPostProcess.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

void AExoCharacter::TryMountZipline()
{
	if (bIsOnZipline || bIsDBNO || bIsDead || bIsExecuting) return;

	// Find nearest zipline endpoint within interaction range
	const float MaxDist = 300.f;
	float BestDist = MaxDist;
	AExoZipline* BestZip = nullptr;
	bool bStartEnd = true; // True = mount from start, false = from end

	for (TActorIterator<AExoZipline> It(GetWorld()); It; ++It)
	{
		AExoZipline* Zip = *It;
		if (!Zip) continue;

		float DistStart = FVector::Distance(GetActorLocation(), Zip->GetStartPoint());
		float DistEnd = FVector::Distance(GetActorLocation(), Zip->GetEndPoint());

		if (DistStart < BestDist)
		{
			BestDist = DistStart;
			BestZip = Zip;
			bStartEnd = true;
		}
		if (DistEnd < BestDist)
		{
			BestDist = DistEnd;
			BestZip = Zip;
			bStartEnd = false;
		}
	}

	if (!BestZip) return;

	StartZipline(BestZip, bStartEnd ? 0.f : 1.f, bStartEnd ? 1 : -1);
}

void AExoCharacter::StartZipline(AExoZipline* Zipline, float StartT, int32 Direction)
{
	if (!Zipline) return;

	bIsOnZipline = true;
	CurrentZipline = Zipline;
	ZiplineT = StartT;
	ZiplineDirection = Direction;

	// Stop any current movement actions
	StopFire();
	StopSprint();
	if (bIsSliding) StopSlide();
	if (bIsWallRunning) EndWallRun(false);

	// Switch to flying mode for smooth cable follow
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->StopMovementImmediately();

	// Initial position snap
	FVector Pos = Zipline->GetPositionAtT(ZiplineT);
	Pos.Z -= 80.f; // Hang below cable
	SetActorLocation(Pos);

	// Speed boost feel
	if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
	{
		PP->ApplySpeedBoostEffect(0.3f);
	}

	UE_LOG(LogExoRift, Log, TEXT("%s mounted zipline"), *GetName());
}

void AExoCharacter::TickZipline(float DeltaTime)
{
	if (!bIsOnZipline || !CurrentZipline) return;

	float Speed = CurrentZipline->RideSpeed;
	float Length = CurrentZipline->GetCableLength();
	if (Length < 1.f) { EndZipline(false); return; }

	// Advance along cable (gravity assists downhill, resists uphill)
	FVector Dir = CurrentZipline->GetDirection() * ZiplineDirection;
	float GravityBoost = -Dir.Z * 800.f; // Downhill = faster, uphill = slower
	float EffectiveSpeed = FMath::Max(Speed + GravityBoost, 400.f);

	float DeltaT = (EffectiveSpeed * DeltaTime) / Length;
	ZiplineT += DeltaT * ZiplineDirection;

	// Reached the end
	if (ZiplineT <= 0.f || ZiplineT >= 1.f)
	{
		ZiplineT = FMath::Clamp(ZiplineT, 0.f, 1.f);
		EndZipline(false);
		return;
	}

	// Move character along cable
	FVector Pos = CurrentZipline->GetPositionAtT(ZiplineT);
	Pos.Z -= 80.f; // Hang below cable
	SetActorLocation(Pos);

	// Face travel direction
	FVector TravelDir = CurrentZipline->GetDirection() * ZiplineDirection;
	FRotator LookRot = FRotator(0.f, TravelDir.Rotation().Yaw, 0.f);
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookRot, DeltaTime, 8.f));

	// Wind audio tick
	ZiplineAudioTimer -= DeltaTime;
	if (ZiplineAudioTimer <= 0.f)
	{
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayWindSound(GetActorLocation(), EffectiveSpeed / Speed);
		}
		ZiplineAudioTimer = 0.3f;
	}
}

void AExoCharacter::EndZipline(bool bJumpOff)
{
	if (!bIsOnZipline) return;

	bIsOnZipline = false;

	// Restore walking
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);

	if (bJumpOff && CurrentZipline)
	{
		// Jump off with momentum in travel direction
		FVector LaunchDir = CurrentZipline->GetDirection() * ZiplineDirection;
		LaunchDir.Z = 0.4f;
		LaunchDir.Normalize();
		LaunchCharacter(LaunchDir * 800.f, true, true);
	}

	CurrentZipline = nullptr;
	ZiplineT = 0.f;
	ZiplineDirection = 1;
	ZiplineAudioTimer = 0.f;

	// Clear speed effect
	if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
	{
		PP->ApplySpeedBoostEffect(0.f);
	}

	UE_LOG(LogExoRift, Log, TEXT("%s dismounted zipline"), *GetName());
}
