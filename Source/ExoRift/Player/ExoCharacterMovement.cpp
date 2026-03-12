// ExoCharacterMovement.cpp — Slide, Mantle, and Footstep tick logic for AExoCharacter
// Split from ExoCharacter.cpp to keep files under 400 lines.

#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Visual/ExoFootstepDust.h"
#include "Visual/ExoSlideTrail.h"
#include "Visual/ExoScreenShake.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ExoRift.h"

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

	// Slide feedback: screen shake on entry
	FExoScreenShake::AddShake(0.06f, 0.05f);

	// Slide feedback: audio cue
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlaySlideStartSound(GetActorLocation());
	}

	// Slide feedback: initial dust burst at feet
	AExoFootstepDust::SpawnLandingDust(GetWorld(), GetActorLocation(), 400.f);

	// Initialize slide VFX state
	SlideTrailTimer = 0.f;

	// Camera tilt direction: lean based on lateral velocity at slide start
	FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
	FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	float Lateral = FVector::DotProduct(GetVelocity(), RightDir);
	SlideTiltTarget = (Lateral > 0.f) ? -3.5f : 3.5f;
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

	// Slide stop feedback: dust burst at stop point
	AExoFootstepDust::SpawnLandingDust(GetWorld(), GetActorLocation(), 350.f);

	// Slide stop feedback: braking audio
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlaySlideStopSound(GetActorLocation());
	}

	// Reset tilt target so TickCameraBob lerps back to neutral
	SlideTiltTarget = 0.f;
}

void AExoCharacter::TickSlide(float DeltaTime)
{
	if (!bIsSliding) return;
	SlideTimer += DeltaTime;

	float Alpha = FMath::Clamp(SlideTimer / SlideDuration, 0.f, 1.f);
	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(SlideStartSpeed, SlideEndSpeed, Alpha);

	// Spawn dust/spark trail periodically while sliding
	SlideTrailTimer += DeltaTime;
	if (SlideTrailTimer >= 0.15f)
	{
		SlideTrailTimer = 0.f;

		// Spark trail — orange-tinted dust at feet, intensity fades as slide slows
		float TrailIntensity = FMath::Lerp(0.8f, 0.3f, Alpha);
		FLinearColor SparkColor(0.8f, 0.5f, 0.2f); // Orange sparks for sci-fi suit scrape
		FVector FootPos = GetActorLocation() - FVector(0.f, 0.f,
			GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AExoFootstepDust* Trail = GetWorld()->SpawnActor<AExoFootstepDust>(
			AExoFootstepDust::StaticClass(), FTransform(FootPos), SpawnParams);
		if (Trail) Trail->InitDust(false, TrailIntensity, SparkColor);

		// Cyan energy streak on the ground
		AExoSlideTrail::SpawnMark(GetWorld(), FootPos);

		// Scrape audio loop tick
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlaySlideLoopSound(GetActorLocation());
		}
	}

	// Interpolate camera tilt toward target (applied in TickCameraBob)
	SlideTiltCurrent = FMath::FInterpTo(SlideTiltCurrent, SlideTiltTarget, DeltaTime, 8.f);

	if (SlideTimer >= SlideDuration || GetCharacterMovement()->Velocity.Size2D() < DefaultWalkSpeed * 0.5f)
	{
		StopSlide();
	}
}

// --- Double Jump ---

void AExoCharacter::PerformDoubleJump()
{
	if (!bCanDoubleJump || bHasDoubleJumped || bIsDead || bIsDBNO) return;
	if (GetCharacterMovement()->IsMovingOnGround()) return; // Only while airborne
	if (bIsWallRunning) return; // Not during wall-run

	bHasDoubleJumped = true;

	// Cancel downward momentum, then launch upward
	FVector Vel = GetCharacterMovement()->Velocity;
	Vel.Z = FMath::Max(Vel.Z, 0.f);
	GetCharacterMovement()->Velocity = Vel;
	LaunchCharacter(FVector(0.f, 0.f, DoubleJumpForce), false, true);

	// Feedback
	FExoScreenShake::AddShake(0.04f, 0.05f);
	AExoFootstepDust::SpawnLandingDust(GetWorld(), GetActorLocation(), 350.f);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		Audio->PlayWeaponFireSound(nullptr, GetActorLocation(), 0.12f);
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

	// Mantle feedback: camera dip/shake
	FExoScreenShake::AddShake(0.08f, 0.1f);

	// Mantle feedback: hand-grab audio cue
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayWeaponFireSound(nullptr, GetActorLocation(), 0.2f);
	}

	// Mantle feedback: dust at the mantle point
	AExoFootstepDust::SpawnLandingDust(GetWorld(), GetActorLocation(), 500.f);
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

void AExoCharacter::TickCameraBob(float DeltaTime)
{
	if (!FirstPersonCamera) return;

	// Strafe tilt — get lateral velocity relative to facing direction
	float TargetTilt = 0.f;
	{
		FVector Vel = GetVelocity();
		FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		float Lateral = FVector::DotProduct(Vel, RightDir);
		TargetTilt = FMath::Clamp(Lateral / 600.f, -1.f, 1.f) * -1.5f;
		if (bIsADS) TargetTilt *= 0.3f;
	}

	// Smoothly decay slide tilt when not sliding
	if (!bIsSliding)
	{
		SlideTiltCurrent = FMath::FInterpTo(SlideTiltCurrent, 0.f, DeltaTime, 6.f);
	}

	// Decay wall-run tilt when not wall-running
	if (!bIsWallRunning)
	{
		WallRunCameraTilt = FMath::FInterpTo(WallRunCameraTilt, 0.f, DeltaTime, 6.f);
	}

	if (!GetCharacterMovement()->IsMovingOnGround() || bIsSliding || bIsMantling)
	{
		FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
		// During slide, keep the lowered camera position instead of lerping back
		float TargetZ = bIsSliding ? (DefaultCameraZ + SlideCameraOffset) : DefaultCameraZ;
		CamLoc.Z = FMath::FInterpTo(CamLoc.Z, TargetZ, DeltaTime, 8.f);
		FirstPersonCamera->SetRelativeLocation(CamLoc);

		// Apply slide/wall-run camera tilt on top of strafe tilt
		float RollTarget = TargetTilt + SlideTiltCurrent + WallRunCameraTilt;
		FRotator CamRot = FirstPersonCamera->GetRelativeRotation();
		CamRot.Roll = FMath::FInterpTo(CamRot.Roll, RollTarget, DeltaTime, 6.f);
		FirstPersonCamera->SetRelativeRotation(CamRot);
		return;
	}

	float Speed = GetVelocity().Size2D();
	if (Speed < 50.f)
	{
		FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
		CamLoc.Z = FMath::FInterpTo(CamLoc.Z, DefaultCameraZ, DeltaTime, 8.f);
		FirstPersonCamera->SetRelativeLocation(CamLoc);
		CameraBobTimer = 0.f;

		FRotator CamRot = FirstPersonCamera->GetRelativeRotation();
		CamRot.Roll = FMath::FInterpTo(CamRot.Roll, WallRunCameraTilt, DeltaTime, 8.f);
		FirstPersonCamera->SetRelativeRotation(CamRot);
		return;
	}

	float BobFreq = bIsSprinting ? 10.f : 7.f;
	float BobAmpV = bIsSprinting ? 2.5f : 1.2f;
	float BobAmpH = bIsSprinting ? 0.8f : 0.3f;

	CameraBobTimer += DeltaTime * BobFreq;

	float VertBob = FMath::Sin(CameraBobTimer) * BobAmpV;
	float HorzBob = FMath::Cos(CameraBobTimer * 0.5f) * BobAmpH;

	FVector CamLoc = FirstPersonCamera->GetRelativeLocation();
	CamLoc.Z = DefaultCameraZ + VertBob;
	FirstPersonCamera->SetRelativeLocation(CamLoc);

	// Roll = walk sway + strafe tilt + wall-run tilt
	FRotator CamRot = FirstPersonCamera->GetRelativeRotation();
	float TargetRoll = HorzBob + TargetTilt + WallRunCameraTilt;
	CamRot.Roll = FMath::FInterpTo(CamRot.Roll, TargetRoll, DeltaTime, 10.f);
	FirstPersonCamera->SetRelativeRotation(CamRot);
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
			// Detect surface type via downward trace with physical material
			EFootstepSurface Surface = EFootstepSurface::Concrete;
			FVector FootLoc = GetActorLocation();
			FHitResult FootHit;
			FCollisionQueryParams FParams;
			FParams.AddIgnoredActor(this);
			FParams.bReturnPhysicalMaterial = true;

			if (GetWorld()->LineTraceSingleByChannel(FootHit,
				FootLoc, FootLoc - FVector(0.f, 0.f, 200.f), ECC_Visibility, FParams))
			{
				bool bSurfaceResolved = false;

				// Primary: detect via Physical Material surface type
				// SurfaceType1=Metal, SurfaceType2=Dirt, SurfaceType3=Water, SurfaceType4=Concrete
				if (FootHit.PhysMaterial.IsValid())
				{
					switch (FootHit.PhysMaterial->SurfaceType)
					{
					case SurfaceType1:
						Surface = EFootstepSurface::Metal;
						bSurfaceResolved = true;
						break;
					case SurfaceType3:
						Surface = EFootstepSurface::Water;
						bSurfaceResolved = true;
						break;
					case SurfaceType4:
						Surface = EFootstepSurface::Concrete;
						bSurfaceResolved = true;
						break;
					default:
						break;
					}

					// Secondary: match by physical material name if surface type is Default
					if (!bSurfaceResolved)
					{
						FString MatName = FootHit.PhysMaterial->GetName().ToLower();
						if (MatName.Contains(TEXT("metal")) || MatName.Contains(TEXT("steel")))
						{
							Surface = EFootstepSurface::Metal;
							bSurfaceResolved = true;
						}
						else if (MatName.Contains(TEXT("water")) || MatName.Contains(TEXT("mud")))
						{
							Surface = EFootstepSurface::Water;
							bSurfaceResolved = true;
						}
						else if (MatName.Contains(TEXT("concrete")) || MatName.Contains(TEXT("stone")))
						{
							Surface = EFootstepSurface::Concrete;
							bSurfaceResolved = true;
						}
					}
				}

				// Fallback: Z-height heuristic when no physical material is assigned
				if (!bSurfaceResolved)
				{
					float HitZ = FootHit.ImpactPoint.Z;
					if (HitZ > 100.f) Surface = EFootstepSurface::Metal;
					else if (HitZ < -50.f) Surface = EFootstepSurface::Water;
				}
			}
			Audio->PlayFootstepSound(FootLoc, bIsSprinting, Surface);
		}
		// Dust puff at feet when sprinting
		AExoFootstepDust::SpawnFootstepDust(GetWorld(), GetActorLocation(), bIsSprinting);
	}
}
