#include "Map/ExoSupplyDrop.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Core/ExoAudioManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

AExoSupplyDrop::AExoSupplyDrop()
{
	PrimaryActorTick.bCanEverTick = true;

	CrateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
	RootComponent = CrateMesh;
	CrateMesh->SetCollisionProfileName(TEXT("BlockAll"));
	CrateMesh->SetVisibility(false); // Root invisible, children visible

	BeaconLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeaconLight"));
	BeaconLight->SetupAttachment(RootComponent);
	BeaconLight->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	BeaconLight->SetIntensity(0.f);
	BeaconLight->SetAttenuationRadius(BeaconRange);
	BeaconLight->SetLightColor(FColor(255, 200, 50));

	// Cache meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeF.Succeeded()) CubeMeshRef = CubeF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereF(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereF.Succeeded()) SphereMeshRef = SphereF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMeshRef = CylF.Object;

	// BaseMaterialRef no longer needed — LitEmissive used at runtime
}

// BuildCrateMesh and SpawnLoot moved to ExoSupplyDropVisuals.cpp

void AExoSupplyDrop::BeginPlay()
{
	Super::BeginPlay();
	BuildCrateMesh();
	TransitionToState(ESupplyDropState::Falling);
}

void AExoSupplyDrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	if (CurrentState == ESupplyDropState::Falling)
	{
		TickFalling(DeltaTime);

		// Pulsing beacon during descent
		if (BeaconLight)
		{
			float Pulse = 0.6f + 0.4f * FMath::Abs(FMath::Sin(Time * 2.f));
			BeaconLight->SetIntensity(BeaconIntensity * 0.3f * Pulse);
		}

		// Animate smoke trail — flickering exhaust
		if (SmokeTrail)
		{
			float TrailFlicker = 4.5f + 1.5f * FMath::Sin(Time * 25.f)
				+ 0.5f * FMath::Sin(Time * 61.f);
			SmokeTrail->SetRelativeScale3D(FVector(0.8f, 0.8f, TrailFlicker));
		}
		if (TrailLight)
		{
			float TLPulse = 60000.f + 30000.f * FMath::Sin(Time * 18.f);
			TrailLight->SetIntensity(TLPulse);
		}
		// Descent beam — pulsing sky streak
		if (DescentBeam && DescentBeamMat)
		{
			float BeamPulse = 1.f + 0.3f * FMath::Sin(Time * 8.f);
			DescentBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(9.f * BeamPulse, 4.5f * BeamPulse, 1.2f * BeamPulse));
		}

		// Beacon column — slow pulsing glow
		if (BeaconColumn && BeaconColumnMat)
		{
			float ColPulse = 0.7f + 0.3f * FMath::Sin(Time * 3.f);
			BeaconColumnMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(12.f * ColPulse, 1.5f * ColPulse, 1.5f * ColPulse));
		}

		// Flashing beacon light — alternates red/white every 0.4s
		if (BeaconFlashLight)
		{
			BeaconFlashTimer += DeltaTime;
			bool bWhitePhase = FMath::Fmod(BeaconFlashTimer, 0.8f) < 0.4f;
			if (bWhitePhase)
			{
				BeaconFlashLight->SetLightColor(FLinearColor(1.f, 1.f, 1.f));
				BeaconFlashLight->SetIntensity(150000.f);
			}
			else
			{
				BeaconFlashLight->SetLightColor(FLinearColor(1.f, 0.1f, 0.1f));
				BeaconFlashLight->SetIntensity(120000.f);
			}
		}
	}
	else if (CurrentState == ESupplyDropState::Landed)
	{
		// Pulsing beacon on ground — attracts players
		if (BeaconLight)
		{
			float Pulse = 0.7f + 0.3f * FMath::Abs(FMath::Sin(Time * 1.5f));
			BeaconLight->SetIntensity(BeaconIntensity * Pulse);
		}

		// Expand and fade the impact ring
		if (ImpactRing)
		{
			ImpactRingTimer += DeltaTime;
			float T01 = FMath::Clamp(ImpactRingTimer / 2.f, 0.f, 1.f);
			float Expand = FMath::Lerp(0.5f, 8.f, T01);
			float Alpha = 1.f - T01;
			ImpactRing->SetRelativeScale3D(FVector(Expand, Expand, 0.02f * Alpha));
			if (ImpactRingMat)
			{
				ImpactRingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(3.5f * Alpha, 1.8f * Alpha, 0.5f * Alpha));
			}
			if (T01 >= 1.f)
			{
				ImpactRing->DestroyComponent();
				ImpactRing = nullptr;
			}
		}
	}

	// Animate lid opening
	if (CurrentState == ESupplyDropState::Opened || CurrentState == ESupplyDropState::Depleted)
	{
		LidOpenAlpha = FMath::FInterpTo(LidOpenAlpha, 1.f, DeltaTime, 3.f);
		if (CrateLid)
		{
			float LidAngle = LidOpenAlpha * -120.f;
			CrateLid->SetRelativeRotation(FRotator(0.f, 0.f, LidAngle));
			CrateLid->SetRelativeLocation(FVector(0.f, -30.f * LidOpenAlpha, 35.f + 20.f * LidOpenAlpha));
		}

		if (CrateGlow)
		{
			float GlowPulse = 1.f + 0.2f * FMath::Sin(Time * 4.f);
			CrateGlow->SetIntensity(LidOpenAlpha * 35000.f * GlowPulse);
		}
	}
}

void AExoSupplyDrop::TickFalling(float DeltaTime)
{
	SwayTimer += DeltaTime;

	FVector Location = GetActorLocation();

	// Parachute sway
	float SwayX = FMath::Sin(SwayTimer * 1.2f) * SwayAmplitude * DeltaTime;
	float SwayY = FMath::Cos(SwayTimer * 0.8f) * SwayAmplitude * 0.6f * DeltaTime;

	FVector NewLocation = Location + FVector(SwayX, SwayY, -DropSpeed * DeltaTime);

	// Rock the crate slightly
	float RockAngle = FMath::Sin(SwayTimer * 1.5f) * 5.f;
	SetActorRotation(FRotator(RockAngle, GetActorRotation().Yaw, RockAngle * 0.5f));

	// Trace downward to detect ground
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHitGround = GetWorld()->LineTraceSingleByChannel(
		Hit, Location, NewLocation - FVector(0.f, 0.f, 50.f),
		ECC_WorldStatic, Params);

	if (bHitGround)
	{
		SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, 5.f));
		SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
		TransitionToState(ESupplyDropState::Landed);
	}
	else
	{
		SetActorLocation(NewLocation);
	}
}

void AExoSupplyDrop::TransitionToState(ESupplyDropState NewState)
{
	CurrentState = NewState;

	switch (NewState)
	{
	case ESupplyDropState::Falling:
		BeaconLight->SetIntensity(BeaconIntensity * 0.3f); // Visible while falling
		BeaconLight->SetAttenuationRadius(BeaconRange);
		break;

	case ESupplyDropState::Landed:
		BeaconLight->SetIntensity(BeaconIntensity);
		// Remove parachute and smoke trail on landing
		if (ParachuteDome) { ParachuteDome->DestroyComponent(); ParachuteDome = nullptr; }
		if (ParachuteStrut1) { ParachuteStrut1->DestroyComponent(); ParachuteStrut1 = nullptr; }
		if (ParachuteStrut2) { ParachuteStrut2->DestroyComponent(); ParachuteStrut2 = nullptr; }
		if (SmokeTrail) { SmokeTrail->DestroyComponent(); SmokeTrail = nullptr; }
		if (TrailLight) { TrailLight->DestroyComponent(); TrailLight = nullptr; }
		if (DescentBeam) { DescentBeam->DestroyComponent(); DescentBeam = nullptr; }
		if (BeaconColumn) { BeaconColumn->DestroyComponent(); BeaconColumn = nullptr; }
		if (BeaconFlashLight) { BeaconFlashLight->DestroyComponent(); BeaconFlashLight = nullptr; }

		// Spawn impact ring — expanding dust cloud
		if (CylinderMeshRef)
		{
			ImpactRing = NewObject<UStaticMeshComponent>(this);
			ImpactRing->SetupAttachment(RootComponent);
			ImpactRing->SetStaticMesh(CylinderMeshRef);
			ImpactRing->SetRelativeLocation(FVector(0.f, 0.f, 5.f));
			ImpactRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.02f));
			ImpactRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ImpactRing->CastShadow = false;
			ImpactRing->RegisterComponent();
			UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
			ImpactRingMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			if (!ImpactRingMat) { return; }
			ImpactRingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(3.5f, 1.8f, 0.5f));
			ImpactRing->SetMaterial(0, ImpactRingMat);
			ImpactRingTimer = 0.f;
		}

		// Landing sound
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayImpactSound(GetActorLocation(), false);
		}
		UE_LOG(LogExoRift, Log, TEXT("Supply drop landed at %s"), *GetActorLocation().ToString());
		break;

	case ESupplyDropState::Opened:
		SpawnLoot();
		BeaconLight->SetIntensity(BeaconIntensity * 0.5f);
		// Open sound
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		{
			Audio->PlayImpactSound(GetActorLocation(), false);
		}
		break;

	case ESupplyDropState::Depleted:
		BeaconLight->SetIntensity(0.f);
		SetLifeSpan(30.f);
		break;
	}
}

void AExoSupplyDrop::Interact(AExoCharacter* Interactor)
{
	if (CurrentState != ESupplyDropState::Landed || !Interactor) return;
	TransitionToState(ESupplyDropState::Opened);

	// Grant shield directly to the interactor
	UExoShieldComponent* Shield = Interactor->GetShieldComponent();
	if (Shield)
	{
		Shield->AddShield(25.f);
	}

	// Delay depletion slightly so player sees loot spawn
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
	{
		TransitionToState(ESupplyDropState::Depleted);
	}, 0.5f, false);
}

FString AExoSupplyDrop::GetInteractionPrompt()
{
	if (CurrentState == ESupplyDropState::Landed)
	{
		return TEXT("[E] Open Supply Drop");
	}
	return FString();
}
