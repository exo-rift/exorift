// ExoCharacterWallRun.cpp — Wall-running movement mechanic
// Split from ExoCharacter to keep files under 400 lines.

#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Visual/ExoFootstepDust.h"
#include "Visual/ExoScreenShake.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExoRift.h"

void AExoCharacter::TryWallRun()
{
	if (bIsWallRunning || bIsSliding || bIsMantling || bIsDead || bIsDBNO) return;
	if (GetCharacterMovement()->IsMovingOnGround()) return;
	if (GetVelocity().Size2D() < 200.f) return;

	FVector Location = GetActorLocation();
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Get movement direction
	FVector MoveDir = GetVelocity().GetSafeNormal2D();
	FVector RightDir = FVector::CrossProduct(FVector::UpVector, MoveDir);

	// Try right wall first, then left
	for (int32 Side : {1, -1})
	{
		FVector TraceDir = RightDir * Side;
		FVector TraceEnd = Location + TraceDir * WallRunTraceDistance;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Location, TraceEnd,
			ECC_Visibility, Params))
		{
			// Wall must be roughly vertical
			if (Hit.ImpactNormal.Z > 0.3f || Hit.ImpactNormal.Z < -0.3f) continue;

			// Start wall run
			bIsWallRunning = true;
			WallRunTimer = 0.f;
			WallRunNormal = Hit.ImpactNormal;
			WallRunSide = Side;

			// Direction along the wall, perpendicular to normal, maintaining forward momentum
			WallRunDirection = FVector::CrossProduct(WallRunNormal, FVector::UpVector);
			if (FVector::DotProduct(WallRunDirection, MoveDir) < 0.f)
				WallRunDirection = -WallRunDirection;

			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
			GetCharacterMovement()->GravityScale = WallRunGravityScale;
			GetCharacterMovement()->Velocity = WallRunDirection * WallRunSpeed
				+ FVector(0.f, 0.f, 100.f); // Slight upward boost at start

			// Feedback
			FExoScreenShake::AddShake(0.05f, 0.08f);
			if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
				Audio->PlayWeaponFireSound(nullptr, GetActorLocation(), 0.15f);

			WallRunTrailTimer = 0.f;
			UE_LOG(LogExoRift, Log, TEXT("%s started wall-run (side %d)"),
				*GetName(), Side);
			return;
		}
	}
}

void AExoCharacter::TickWallRun(float DeltaTime)
{
	if (!bIsWallRunning) return;

	WallRunTimer += DeltaTime;

	// Check if wall is still there
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	FVector TraceDir = -WallRunNormal;
	FVector TraceEnd = GetActorLocation() + TraceDir * (WallRunTraceDistance + 20.f);
	FHitResult Hit;
	bool bStillOnWall = GetWorld()->LineTraceSingleByChannel(
		Hit, GetActorLocation(), TraceEnd, ECC_Visibility, Params);

	if (!bStillOnWall || WallRunTimer >= WallRunDuration
		|| GetCharacterMovement()->IsMovingOnGround())
	{
		EndWallRun(false);
		return;
	}

	// Maintain wall-run velocity with slight downward drift over time
	float GravityFade = FMath::Lerp(0.f, 200.f, WallRunTimer / WallRunDuration);
	FVector DesiredVel = WallRunDirection * WallRunSpeed
		+ FVector(0.f, 0.f, -GravityFade);

	// Push into wall slightly to maintain contact
	DesiredVel += TraceDir * 50.f;
	GetCharacterMovement()->Velocity = DesiredVel;

	// Camera tilt toward wall (12 degrees)
	float TargetTilt = WallRunSide * 12.f;
	WallRunCameraTilt = FMath::FInterpTo(WallRunCameraTilt, TargetTilt, DeltaTime, 8.f);
	if (FirstPersonCamera)
	{
		FRotator CamRot = FirstPersonCamera->GetRelativeRotation();
		CamRot.Roll = WallRunCameraTilt;
		FirstPersonCamera->SetRelativeRotation(CamRot);
	}

	// VFX trail every 0.12s
	WallRunTrailTimer += DeltaTime;
	if (WallRunTrailTimer >= 0.12f)
	{
		WallRunTrailTimer = 0.f;
		FVector FootPos = GetActorLocation() - FVector(0.f, 0.f,
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		AExoFootstepDust::SpawnLandingDust(GetWorld(),
			FootPos + WallRunNormal * 10.f, 300.f);
	}
}

void AExoCharacter::EndWallRun(bool bJumpOff)
{
	if (!bIsWallRunning) return;
	bIsWallRunning = false;
	WallRunTimer = 0.f;

	GetCharacterMovement()->GravityScale = 1.f;
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);

	if (bJumpOff)
	{
		// Jump away from wall and upward, with some forward momentum
		FVector JumpDir = WallRunNormal * WallJumpForce
			+ FVector(0.f, 0.f, WallJumpForce * 0.7f)
			+ WallRunDirection * WallJumpForce * 0.3f;
		LaunchCharacter(JumpDir, true, true);

		FExoScreenShake::AddShake(0.06f, 0.06f);
		AExoFootstepDust::SpawnLandingDust(GetWorld(), GetActorLocation(), 450.f);
	}

	// Camera tilt will lerp back to 0 in TickCameraBob
	UE_LOG(LogExoRift, Log, TEXT("%s wall-run ended (jump=%d)"),
		*GetName(), bJumpOff);
}

void AExoCharacter::WallJump()
{
	if (bIsWallRunning)
	{
		EndWallRun(true);
	}
}
