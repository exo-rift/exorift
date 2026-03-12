// ExoDropPod.cpp — Constructor, flight logic, landing sequence
// Visual construction & VFX in ExoDropPodVisuals.cpp
#include "Map/ExoDropPod.h"
#include "Map/ExoDropPodManager.h"
#include "Player/ExoCharacter.h"
#include "Visual/ExoTracerManager.h"
#include "Visual/ExoPostProcess.h"
#include "Visual/ExoScreenShake.h"
#include "Visual/ExoKillScorch.h"
#include "Core/ExoAudioManager.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "ExoRift.h"

AExoDropPod::AExoDropPod()
{
	PrimaryActorTick.bCanEverTick = true;

	PodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PodRoot"));
	RootComponent = PodMesh;
	PodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PodMesh->SetVisibility(false);

	PodCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PodCamera"));
	PodCamera->SetupAttachment(PodMesh);
	PodCamera->SetRelativeLocation(FVector(-400.f, 0.f, 250.f));
	PodCamera->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));
	CameraBaseOffset = FVector(-400.f, 0.f, 250.f);

	ThrusterLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ThrusterLight"));
	ThrusterLight->SetupAttachment(PodMesh);
	ThrusterLight->SetRelativeLocation(FVector(0.f, 0.f, -150.f));
	ThrusterLight->SetIntensity(0.f);
	ThrusterLight->SetAttenuationRadius(4200.f);
	ThrusterLight->SetLightColor(FColor(180, 200, 255));

	// Cache engine meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFind(
		TEXT("/Engine/BasicShapes/Cone"));
	if (ConeFind.Succeeded()) ConeMesh = ConeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFind.Succeeded()) SphereMesh = SphereFind.Object;

	// BaseMaterial no longer needed — LitEmissive used at runtime

	// Zero-init VFX arrays
	for (int32 i = 0; i < CONTRAIL_SEGMENTS; i++)
	{
		ContrailSegments[i] = nullptr;
		ContrailMats[i] = nullptr;
	}
	for (int32 i = 0; i < STEAM_PUFFS; i++)
	{
		SteamPuffs[i] = nullptr;
		SteamMats[i] = nullptr;
	}
}

void AExoDropPod::InitPod(AController* InPassenger, AExoDropPodManager* InManager)
{
	Passenger = InPassenger;
	Manager = InManager;

	BuildPodMesh();
	BuildContrail();
	PrevPodLocation = GetActorLocation();

	if (Passenger)
	{
		if (APawn* OldPawn = Passenger->GetPawn())
		{
			OldPawn->SetActorHiddenInGame(true);
			OldPawn->SetActorEnableCollision(false);
		}
		// Set camera to follow the drop pod during descent
		if (APlayerController* PC = Cast<APlayerController>(Passenger))
		{
			PC->SetViewTargetWithBlend(this, 0.f);
		}
	}

	Phase = EDropPodPhase::FreeFall;
}

void AExoDropPod::ApplySteerInput(FVector2D Input)
{
	SteerInput = Input;
}

void AExoDropPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Phase == EDropPodPhase::Landed)
	{
		UpdateLandedSequence(DeltaTime);
		return;
	}

	// Detect ground
	if (!bGroundDetected)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, 200000.f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			GroundZ = Hit.ImpactPoint.Z + 200.f;
			bGroundDetected = true;
		}
	}

	float CurrentZ = GetActorLocation().Z;
	float AltAboveGround = bGroundDetected ? (CurrentZ - GroundZ) : 50000.f;

	// Phase transitions
	if (Phase == EDropPodPhase::FreeFall && AltAboveGround < BrakeAltitude)
		Phase = EDropPodPhase::Braking;
	if (Phase == EDropPodPhase::Braking && AltAboveGround < 500.f)
		Phase = EDropPodPhase::Landing;

	// Descent speed
	float DescentSpeed;
	float BrakeAlpha = 0.f;
	switch (Phase)
	{
	case EDropPodPhase::FreeFall:
		DescentSpeed = MaxDescentSpeed;
		break;
	case EDropPodPhase::Braking:
		BrakeAlpha = 1.f - (AltAboveGround / BrakeAltitude);
		DescentSpeed = FMath::Lerp(MaxDescentSpeed, LandingSpeed, BrakeAlpha);
		break;
	case EDropPodPhase::Landing:
	default:
		BrakeAlpha = 1.f;
		DescentSpeed = LandingSpeed;
		break;
	}

	// Steering
	FVector HorizontalMove = FVector::ZeroVector;
	if (Phase != EDropPodPhase::Landing)
	{
		float SteerMult = (Phase == EDropPodPhase::FreeFall) ? SteerSpeed : SteerSpeed * 0.3f;
		HorizontalMove = FVector(SteerInput.Y, SteerInput.X, 0.f) * SteerMult * DeltaTime;

		float TargetTilt = SteerInput.X * 15.f;
		PodTilt = FMath::FInterpTo(PodTilt, TargetTilt, DeltaTime, 5.f);
		SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, PodTilt));
	}
	else
	{
		PodTilt = FMath::FInterpTo(PodTilt, 0.f, DeltaTime, 8.f);
		SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, PodTilt));
	}

	// Camera vibration during descent (subtle at speed)
	if (PodCamera)
	{
		float SpeedFrac = DescentSpeed / MaxDescentSpeed;
		float Time = GetWorld()->GetTimeSeconds();
		float VibX = FMath::Sin(Time * 45.f) * 2.f * SpeedFrac;
		float VibY = FMath::Cos(Time * 31.f) * 1.5f * SpeedFrac;
		float VibZ = FMath::Sin(Time * 53.f) * 1.f * SpeedFrac;
		PodCamera->SetRelativeLocation(CameraBaseOffset + FVector(VibX, VibY, VibZ));
	}

	// Move
	FVector NewLoc = GetActorLocation() + HorizontalMove
		- FVector(0.f, 0.f, DescentSpeed * DeltaTime);

	if (bGroundDetected && NewLoc.Z <= GroundZ)
	{
		NewLoc.Z = GroundZ;
		SetActorLocation(NewLoc);

		// Impact effects
		FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), 800.f);
		SpawnLandingDust();
		SpawnLandingScorch();

		// Camera shake on impact
		ShakeIntensity = 12.f;
		ShakeDecay = 8.f;

		// Landing screen effects
		FExoScreenShake::AddShake(2.f, 0.5f);
		if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
		{
			PP->TriggerDamageFlash(0.6f);
			PP->ApplySpeedBoostEffect(0.f); // Clear descent blur
		}

		// Audio
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayExplosionSound(GetActorLocation());
		}

		Phase = EDropPodPhase::Landed;
		LandedTimer = 0.f;

		UE_LOG(LogExoRift, Log, TEXT("Drop pod touched down at %s"),
			*GetActorLocation().ToString());
	}
	else
	{
		SetActorLocation(NewLoc);
	}

	UpdateContrail(DeltaTime);
	UpdateThrusterVFX(DeltaTime, BrakeAlpha);

	// Screen effects during descent
	if (AExoPostProcess* PP = AExoPostProcess::Get(GetWorld()))
	{
		float SpeedFrac = DescentSpeed / MaxDescentSpeed;
		// Speed boost effect proportional to descent speed
		PP->ApplySpeedBoostEffect(SpeedFrac * 0.8f);
	}
}

void AExoDropPod::UpdateLandedSequence(float DeltaTime)
{
	LandedTimer += DeltaTime;

	// Camera shake decay
	if (ShakeIntensity > 0.1f && PodCamera)
	{
		ShakeIntensity = FMath::FInterpTo(ShakeIntensity, 0.f, DeltaTime, ShakeDecay);
		float Time = GetWorld()->GetTimeSeconds();
		float SX = FMath::Sin(Time * 35.f) * ShakeIntensity;
		float SY = FMath::Cos(Time * 27.f) * ShakeIntensity * 0.7f;
		float SZ = FMath::Sin(Time * 41.f) * ShakeIntensity * 0.5f;
		PodCamera->SetRelativeLocation(CameraBaseOffset + FVector(SX, SY, SZ));
	}
	else if (PodCamera)
	{
		// Smooth camera pull-back as door opens
		float PullBack = FMath::Clamp((LandedTimer - 0.5f) * 0.8f, 0.f, 1.f);
		FVector TargetOffset = CameraBaseOffset + FVector(-100.f * PullBack, 0.f, 50.f * PullBack);
		PodCamera->SetRelativeLocation(FMath::Lerp(
			PodCamera->GetRelativeLocation(), TargetOffset, DeltaTime * 3.f));
	}

	// Impact flash fade
	if (ImpactFlash)
	{
		float FlashIntensity = FMath::Max(0.f, 1.f - LandedTimer * 4.f) * 220000.f;
		ImpactFlash->SetIntensity(FlashIntensity);
	}

	// Dust ring expansion
	if (DustRing && DustMat)
	{
		DustAge += DeltaTime;
		float Expand = 1.f + DustAge * 8.f;
		float DustAlpha = FMath::Max(0.f, 1.f - DustAge * 1.5f);
		DustRing->SetWorldScale3D(FVector(Expand, Expand, 0.05f));
		DustMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.66f * DustAlpha, 0.55f * DustAlpha, 0.33f * DustAlpha));
		if (DustAlpha <= 0.01f)
		{
			DustRing->SetVisibility(false);
		}
	}

	// Thruster wind-down
	if (ThrusterLight)
	{
		float Fade = FMath::Max(0.f, 1.f - LandedTimer * 2.f);
		ThrusterLight->SetIntensity(8000.f * Fade);
	}
	if (ThrusterFlame)
	{
		float Fade = FMath::Max(0.f, 1.f - LandedTimer * 3.f);
		ThrusterFlame->SetRelativeScale3D(FVector(0.3f * Fade, 0.3f * Fade, 2.f * Fade));
	}

	// Contrail fade-out after landing
	if (bContrailBuilt)
	{
		for (int32 i = 0; i < CONTRAIL_SEGMENTS; i++)
		{
			if (ContrailSegments[i] && ContrailMats[i])
			{
				float Fade = FMath::Max(0.f, 1.f - LandedTimer * 2.5f);
				ContrailMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(8.f * Fade, 18.f * Fade, 50.f * Fade));
				if (Fade <= 0.01f)
					ContrailSegments[i]->SetVisibility(false);
			}
		}
		if (ContrailLight)
		{
			float Fade = FMath::Max(0.f, 1.f - LandedTimer * 2.5f);
			ContrailLight->SetIntensity(60000.f * Fade);
		}
	}

	// Door steam burst — triggered 0.6s after landing (door opening moment)
	if (LandedTimer > 0.6f && !bDoorSteamSpawned)
	{
		SpawnDoorSteam();
		bDoorSteamSpawned = true;
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayDoorSlideSound(GetActorLocation(), true);
		}
	}

	// Animate steam puffs
	if (SteamAge >= 0.f)
	{
		SteamAge += DeltaTime;
		float SteamFade = FMath::Max(0.f, 1.f - SteamAge * 1.8f);
		for (int32 i = 0; i < STEAM_PUFFS; i++)
		{
			if (!SteamPuffs[i]) continue;
			SteamPuffs[i]->AddWorldOffset(SteamVelocities[i] * DeltaTime);
			float Expand = 0.15f + SteamAge * 0.6f;
			SteamPuffs[i]->SetWorldScale3D(FVector(Expand, Expand, Expand * 0.6f));
			if (SteamMats[i])
			{
				SteamMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(22.f * SteamFade, 28.f * SteamFade, 40.f * SteamFade));
			}
			if (SteamFade <= 0.01f)
				SteamPuffs[i]->SetVisibility(false);
		}
	}

	if (LandedTimer > 1.5f)
	{
		OnLanded();
	}
}

void AExoDropPod::OnLanded()
{
	if (Phase != EDropPodPhase::Landed) return;
	Phase = EDropPodPhase::FreeFall; // Prevent re-entry

	if (Passenger)
	{
		FVector SpawnLoc = GetActorLocation() + FVector(200.f, 0.f, 100.f);
		FRotator SpawnRot = FRotator(0.f, GetActorRotation().Yaw, 0.f);

		APawn* TargetPawn = nullptr;
		if (APawn* ExistingPawn = Passenger->GetPawn())
		{
			ExistingPawn->SetActorLocation(SpawnLoc);
			ExistingPawn->SetActorRotation(SpawnRot);
			ExistingPawn->SetActorHiddenInGame(false);
			ExistingPawn->SetActorEnableCollision(true);
			TargetPawn = ExistingPawn;
		}
		else
		{
			AExoCharacter* NewChar = GetWorld()->SpawnActor<AExoCharacter>(
				AExoCharacter::StaticClass(), SpawnLoc, SpawnRot);
			if (NewChar) { Passenger->Possess(NewChar); TargetPawn = NewChar; }
		}

		// Transition camera back to the player character
		if (APlayerController* PC = Cast<APlayerController>(Passenger))
		{
			if (TargetPawn)
				PC->SetViewTargetWithBlend(TargetPawn, 0.5f);
		}
	}

	if (Manager)
	{
		Manager->OnPodLanded(this);
	}

	SetLifeSpan(3.f);
}
