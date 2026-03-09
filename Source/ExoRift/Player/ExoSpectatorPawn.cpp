#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
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

		if (DeathCamTarget && DeathCamTimer < DeathCamDuration)
		{
			// Orbit around killer
			float Angle = DeathCamTimer * 30.f; // degrees per second
			float Dist = 400.f;
			float Height = 200.f;

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

	// Start at death location looking at killer
	SetActorLocation(InDeathLocation + FVector(0.f, 0.f, 200.f));

	if (Killer)
	{
		FRotator LookAt = (Killer->GetActorLocation() - GetActorLocation()).Rotation();
		if (AController* PC = GetController())
		{
			PC->SetControlRotation(LookAt);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Death cam started"));
}

void AExoSpectatorPawn::StartFreeCam()
{
	RefreshSpectatableTargets();
	if (SpectatableTargets.Num() > 0)
	{
		CycleSpectateTarget(1);
	}
	UE_LOG(LogExoRift, Log, TEXT("Free spectate mode"));
}

void AExoSpectatorPawn::CycleSpectateTarget(int32 Direction)
{
	if (SpectatableTargets.Num() == 0) return;

	CurrentSpectateIndex += Direction;
	if (CurrentSpectateIndex >= SpectatableTargets.Num())
		CurrentSpectateIndex = 0;
	if (CurrentSpectateIndex < 0)
		CurrentSpectateIndex = SpectatableTargets.Num() - 1;

	APawn* Target = SpectatableTargets[CurrentSpectateIndex].Get();
	if (Target)
	{
		// Attach camera behind target
		FVector BehindTarget = Target->GetActorLocation() - Target->GetActorForwardVector() * 300.f + FVector(0.f, 0.f, 150.f);
		SetActorLocation(BehindTarget);

		FRotator LookAt = (Target->GetActorLocation() - GetActorLocation()).Rotation();
		if (AController* PC = GetController())
		{
			PC->SetControlRotation(LookAt);
		}
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
}
