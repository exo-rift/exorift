#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "ExoRift.h"

AExoSpectatorPawn::AExoSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AExoSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInDeathCam)
	{
		DeathCamTimer += DeltaTime;

		if (DeathCamTimer < PullbackDuration)
		{
			// Phase 1: Pullback — camera smoothly rises and pulls back from death point
			float Alpha = FMath::Clamp(DeathCamTimer / PullbackDuration, 0.f, 1.f);
			// Ease-out for cinematic deceleration
			float EasedAlpha = 1.f - FMath::Pow(1.f - Alpha, 3.f);

			FVector PullbackEnd = DeathLocation + FVector(0.f, -180.f, 350.f);
			FVector CamPos = FMath::Lerp(PullbackStartLoc, PullbackEnd, EasedAlpha);
			SetActorLocation(CamPos);

			// Look at death point (slightly below center for cinematic framing)
			FVector LookTarget = DeathLocation + FVector(0.f, 0.f, -30.f);
			FRotator LookAt = (LookTarget - CamPos).Rotation();
			if (AController* PC = GetController())
			{
				PC->SetControlRotation(LookAt);
			}
		}
		else if (DeathCamTarget && DeathCamTimer < DeathCamDuration)
		{
			// Phase 2: Orbit around killer
			float OrbitTime = DeathCamTimer - PullbackDuration;
			float Angle = OrbitTime * 25.f; // Slower orbit for drama
			float Dist = 450.f;
			float Height = 220.f;

			FVector TargetLoc = DeathCamTarget->GetActorLocation();
			FVector CamOffset(
				FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
				FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
				Height
			);

			SetActorLocation(TargetLoc + CamOffset);

			FRotator LookAt = (TargetLoc - GetActorLocation()).Rotation();
			if (AController* PC = GetController())
			{
				PC->SetControlRotation(LookAt);
			}
		}
		else if (DeathCamTimer >= DeathCamDuration)
		{
			// Transition to free spectate
			bInDeathCam = false;
			StartFreeCam();
		}
	}

	// Smooth follow during free spectate
	if (bInFreeSpectate)
	{
		APawn* Target = GetCurrentSpectateTarget();
		if (Target)
		{
			FVector TargetLoc = Target->GetActorLocation();
			FVector TargetForward = Target->GetActorForwardVector();

			// Desired position: behind and above the target
			FVector DesiredLoc = TargetLoc
				- TargetForward * SpectateFollowDistance
				+ FVector(0.f, 0.f, SpectateFollowHeight);

			// Smoothly interpolate position
			FVector CurrentLoc = GetActorLocation();
			FVector NewLoc = FMath::VInterpTo(CurrentLoc, DesiredLoc, DeltaTime, SpectateInterpSpeed);
			SetActorLocation(NewLoc);

			// Smoothly rotate to look at target (aim at chest height)
			FVector LookTarget = TargetLoc + FVector(0.f, 0.f, 60.f);
			FRotator DesiredRot = (LookTarget - NewLoc).Rotation();
			if (AController* PC = GetController())
			{
				FRotator CurrentRot = PC->GetControlRotation();
				FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, SpectateInterpSpeed);
				PC->SetControlRotation(NewRot);
			}
		}
		else
		{
			// Target died or became invalid — auto-cycle to next
			CycleSpectateTarget(1);
		}
	}

	// Refresh spectate targets periodically
	TargetRefreshTimer += DeltaTime;
	if (TargetRefreshTimer >= 2.f)
	{
		RefreshSpectatableTargets();
		TargetRefreshTimer = 0.f;
	}
}

void AExoSpectatorPawn::StartDeathCam(AActor* Killer, const FVector& InDeathLocation)
{
	DeathCamTarget = Killer;
	DeathLocation = InDeathLocation;
	bInDeathCam = true;
	DeathCamTimer = 0.f;

	// Start at eye-level at death location (pullback begins here)
	PullbackStartLoc = InDeathLocation + FVector(0.f, 0.f, 80.f);
	SetActorLocation(PullbackStartLoc);

	if (Killer)
	{
		FRotator LookAt = (Killer->GetActorLocation() - GetActorLocation()).Rotation();
		PullbackStartRot = LookAt;
		if (AController* PC = GetController())
		{
			PC->SetControlRotation(LookAt);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Death cam started (pullback + orbit)"));
}

void AExoSpectatorPawn::StartFreeCam()
{
	bInFreeSpectate = true;
	RefreshSpectatableTargets();
	if (SpectatableTargets.Num() > 0)
	{
		CycleSpectateTarget(1);
	}
	UE_LOG(LogExoRift, Log, TEXT("Free spectate mode — smooth follow enabled"));
}

void AExoSpectatorPawn::CycleSpectateTarget(int32 Direction)
{
	if (SpectatableTargets.Num() == 0) return;

	CurrentSpectateIndex += Direction;
	if (CurrentSpectateIndex >= SpectatableTargets.Num())
		CurrentSpectateIndex = 0;
	if (CurrentSpectateIndex < 0)
		CurrentSpectateIndex = SpectatableTargets.Num() - 1;

	// Smooth interpolation to the new target happens in Tick
	APawn* Target = GetCurrentSpectateTarget();
	if (Target)
	{
		UE_LOG(LogExoRift, Log, TEXT("Spectating: %s"), *GetSpectateTargetName());
	}
}

void AExoSpectatorPawn::RefreshSpectatableTargets()
{
	SpectatableTargets.Reset();

	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Char = *It;
		if (Char && Char->IsAlive())
		{
			SpectatableTargets.Add(Char);
		}
	}

	// Clamp index if targets shrunk
	if (CurrentSpectateIndex >= SpectatableTargets.Num())
	{
		CurrentSpectateIndex = SpectatableTargets.Num() > 0 ? 0 : -1;
	}
}

APawn* AExoSpectatorPawn::GetCurrentSpectateTarget() const
{
	if (CurrentSpectateIndex >= 0 && CurrentSpectateIndex < SpectatableTargets.Num())
	{
		return SpectatableTargets[CurrentSpectateIndex].Get();
	}
	return nullptr;
}

FString AExoSpectatorPawn::GetSpectateTargetName() const
{
	APawn* Target = GetCurrentSpectateTarget();
	if (Target)
	{
		APlayerState* PS = Target->GetPlayerState();
		if (PS)
		{
			return PS->GetPlayerName();
		}
		return Target->GetName();
	}
	return FString();
}
