// ExoHoverVehiclePhysics.cpp — Hover force, movement, and boost physics
#include "Vehicles/ExoHoverVehicle.h"

void AExoHoverVehicle::ApplyHoverForce(float DeltaTime)
{
	FVector CurrentLocation = GetActorLocation();
	FVector TraceStart = CurrentLocation;
	FVector TraceEnd = CurrentLocation - FVector(0.f, 0.f, HoverHeight * 3.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	bool bGroundFound = GetWorld()->LineTraceSingleByChannel(
		Hit, TraceStart, TraceEnd, ECC_Visibility, Params);

	if (bGroundFound)
	{
		float DistToGround = Hit.Distance;
		float Error = HoverHeight - DistToGround;

		// Spring-damper force
		float VerticalVelocity = Velocity.Z;
		float SpringForce = Error * HoverStiffness - VerticalVelocity * HoverDamping;

		Velocity.Z += SpringForce * DeltaTime;
	}
	else
	{
		// No ground below — apply gravity
		Velocity.Z -= 980.f * DeltaTime;
	}

	// Apply vertical movement
	FVector NewLocation = CurrentLocation + FVector(0.f, 0.f, Velocity.Z * DeltaTime);
	SetActorLocation(NewLocation, true);
}

void AExoHoverVehicle::ApplyMovement(float DeltaTime)
{
	// Boost energy management
	if (bIsBoosting && CurrentBoostEnergy > 0.f)
	{
		CurrentBoostEnergy = FMath::Max(0.f, CurrentBoostEnergy - BoostDrainRate * DeltaTime);
	}
	else
	{
		bIsBoosting = false;
		CurrentBoostEnergy = FMath::Min(BoostEnergy, CurrentBoostEnergy + BoostRechargeRate * DeltaTime);
	}

	// Speed multiplier from boost
	float SpeedMult = (bIsBoosting && CurrentBoostEnergy > 0.f) ? BoostMultiplier : 1.f;
	float EffectiveMaxSpeed = MaxSpeed * SpeedMult;

	// Acceleration / deceleration
	if (FMath::Abs(ThrottleInput) > 0.01f)
	{
		CurrentSpeed += ThrottleInput * Acceleration * DeltaTime;
		CurrentSpeed = FMath::Clamp(CurrentSpeed, -EffectiveMaxSpeed * 0.5f, EffectiveMaxSpeed);
	}
	else
	{
		// Drag when no input
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, 0.f, DeltaTime, 2.f);
	}

	// Steering with lean
	if (FMath::Abs(CurrentSpeed) > 10.f)
	{
		float YawDelta = SteerInput * TurnRate * DeltaTime;
		FRotator CurrentRot = GetActorRotation();
		CurrentRot.Yaw += YawDelta;

		// Lean into turns — proportional to speed and steer input
		float SpeedFactor = FMath::Clamp(FMath::Abs(CurrentSpeed) / MaxSpeed, 0.f, 1.f);
		float TargetLean = -SteerInput * 15.f * SpeedFactor;
		CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, TargetLean, DeltaTime, 5.f);
		CurrentRot.Roll = CurrentLeanAngle;

		SetActorRotation(CurrentRot);
	}
	else
	{
		// Return to level when stopped
		CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, 0.f, DeltaTime, 5.f);
		FRotator Rot = GetActorRotation();
		Rot.Roll = CurrentLeanAngle;
		SetActorRotation(Rot);
	}

	// Apply forward movement
	FVector Forward = GetActorForwardVector();
	FVector HorizontalVelocity = Forward * CurrentSpeed;
	Velocity.X = HorizontalVelocity.X;
	Velocity.Y = HorizontalVelocity.Y;

	FVector HorizontalMove = FVector(Velocity.X, Velocity.Y, 0.f) * DeltaTime;
	AddActorWorldOffset(HorizontalMove, true);
}
