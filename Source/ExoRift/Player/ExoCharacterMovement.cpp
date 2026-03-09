// ExoCharacterMovement.cpp — Slide, Mantle, and Footstep tick logic for AExoCharacter
// Split from ExoCharacter.cpp to keep files under 400 lines.

#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// --- Sliding ---

void AExoCharacter::StartSlide()
{
	if (bIsSliding || bIsDead || bIsDBNO || bIsMantling || bIsExecuting) return;
	if (!GetCharacterMovement()->IsMovingOnGround()) return;

	bIsSliding = true;
	SlideTimer = 0.f;

	StopFire();

	// Half capsule height
	GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight * 0.5f);

	// Lower camera
	FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
	CamLoc.Z = DefaultCameraZ + SlideCameraOffset;
	FirstPersonCamera->SetRelativeLocation(CamLoc);

	// Burst speed in the direction of movement
	GetCharacterMovement()->MaxWalkSpeed = SlideStartSpeed;
	GetCharacterMovement()->GroundFriction = 0.f;
}

void AExoCharacter::StopSlide()
{
	if (!bIsSliding) return;
	bIsSliding = false;
	SlideTimer = 0.f;

	GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight);
	FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
	CamLoc.Z = DefaultCameraZ;
	FirstPersonCamera->SetRelativeLocation(CamLoc);

	GetCharacterMovement()->GroundFriction = 8.f;
	float Speed = bIsSprinting ? DefaultWalkSpeed * SprintSpeedMultiplier : DefaultWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AExoCharacter::TickSlide(float DeltaTime)
{
	if (!bIsSliding) return;
	SlideTimer += DeltaTime;

	float Alpha = FMath::Clamp(SlideTimer / SlideDuration, 0.f, 1.f);
	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(SlideStartSpeed, SlideEndSpeed, Alpha);

	if (SlideTimer >= SlideDuration || GetCharacterMovement()->Velocity.Size2D() < DefaultWalkSpeed * 0.5f)
	{
		StopSlide();
	}
}

// --- Mantling ---

void AExoCharacter::TryMantle()
{
	if (bIsMantling || bIsSliding || bIsDead || bIsDBNO || bIsExecuting) return;
	if (GetCharacterMovement()->IsMovingOnGround()) return; // Only while airborne

	FVector EyeLocation;
	FRotator EyeRotation;
	GetActorEyesViewPoint(EyeLocation, EyeRotation);

	// Forward trace from eye level to detect a wall
	FVector ForwardDir = FVector(EyeRotation.Vector().X, EyeRotation.Vector().Y, 0.f).GetSafeNormal();
	FVector TraceStart = EyeLocation;
	FVector TraceEnd = TraceStart + ForwardDir * MantleForwardTrace;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult WallHit;
	bool bHitWall = GetWorld()->LineTraceSingleByChannel(WallHit, TraceStart, TraceEnd, ECC_Visibility, Params);
	if (!bHitWall) return;

	// Trace down from above the wall to find the ledge surface
	FVector LedgeTraceStart = WallHit.ImpactPoint + ForwardDir * 10.f + FVector(0.f, 0.f, MantleReach);
	FVector LedgeTraceEnd = LedgeTraceStart - FVector(0.f, 0.f, MantleReach * 2.f);

	FHitResult LedgeHit;
	bool bFoundLedge = GetWorld()->LineTraceSingleByChannel(LedgeHit, LedgeTraceStart, LedgeTraceEnd, ECC_Visibility, Params);
	if (!bFoundLedge) return;

	// Check if ledge is within MantleReach of the player's head
	float HeadZ = GetActorLocation().Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float LedgeZ = LedgeHit.ImpactPoint.Z;
	if (LedgeZ < GetActorLocation().Z || LedgeZ > HeadZ + MantleReach) return;

	// Ledge must be roughly horizontal (walkable)
	if (LedgeHit.ImpactNormal.Z < 0.7f) return;

	// Start mantle
	bIsMantling = true;
	MantleTimer = 0.f;
	MantleStartLocation = GetActorLocation();
	MantleTarget = FVector(LedgeHit.ImpactPoint.X, LedgeHit.ImpactPoint.Y,
		LedgeZ + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.f);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->StopMovementImmediately();
}

void AExoCharacter::TickMantle(float DeltaTime)
{
	if (!bIsMantling) return;

	MantleTimer += DeltaTime;
	float Alpha = FMath::Clamp(MantleTimer / MantleDuration, 0.f, 1.f);

	// Smooth interpolation to ledge
	FVector NewLoc = FMath::Lerp(MantleStartLocation, MantleTarget, Alpha);
	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		bIsMantling = false;
		MantleTimer = 0.f;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void AExoCharacter::TickFootsteps(float DeltaTime)
{
	if (!GetCharacterMovement()->IsMovingOnGround()) return;
	if (GetVelocity().Size2D() < 50.f) return;

	FootstepInterval = bIsSprinting ? 0.35f : 0.5f;
	FootstepTimer -= DeltaTime;
	if (FootstepTimer <= 0.f)
	{
		FootstepTimer = FootstepInterval;
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayFootstepSound(GetActorLocation(), bIsSprinting);
		}
	}
}
