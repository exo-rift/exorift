// ExoHoverVehicle.cpp — Constructor, input, physics, enter/exit
// VFX (BeginPlay, UpdateVFX) in ExoHoverVehicleVFX.cpp
#include "Vehicles/ExoHoverVehicle.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "ExoRift.h"

AExoHoverVehicle::AExoHoverVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root mesh
	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = VehicleMesh;
	VehicleMesh->SetSimulatePhysics(false);
	VehicleMesh->SetCollisionProfileName(TEXT("Pawn"));

	// Third-person camera arm
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(VehicleMesh);
	CameraArm->TargetArmLength = 400.f;
	CameraArm->SetRelativeLocation(FVector(0.f, 0.f, 80.f));
	CameraArm->bUsePawnControlRotation = true;
	CameraArm->bDoCollisionTest = true;

	DriverCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DriverCamera"));
	DriverCamera->SetupAttachment(CameraArm);

	// Interaction sphere (players walk into range to get the prompt)
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(VehicleMesh);
	InteractionSphere->SetSphereRadius(250.f);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	InteractionSphere->SetGenerateOverlapEvents(true);

	// Thruster meshes (small cylinders at rear)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	if (CylFinder.Succeeded())
	{
		ThrusterL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrusterL"));
		ThrusterL->SetupAttachment(VehicleMesh);
		ThrusterL->SetStaticMesh(CylFinder.Object);
		ThrusterL->SetRelativeLocation(FVector(-80.f, -40.f, -10.f));
		ThrusterL->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.15f));
		ThrusterL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThrusterL->CastShadow = false;

		ThrusterR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrusterR"));
		ThrusterR->SetupAttachment(VehicleMesh);
		ThrusterR->SetStaticMesh(CylFinder.Object);
		ThrusterR->SetRelativeLocation(FVector(-80.f, 40.f, -10.f));
		ThrusterR->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.15f));
		ThrusterR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThrusterR->CastShadow = false;
	}

	// Engine glow lights (cyan-blue hover energy)
	EngineGlowL = CreateDefaultSubobject<UPointLightComponent>(TEXT("EngineGlowL"));
	EngineGlowL->SetupAttachment(VehicleMesh);
	EngineGlowL->SetRelativeLocation(FVector(-80.f, -40.f, -30.f));
	EngineGlowL->SetIntensity(5000.f);
	EngineGlowL->SetAttenuationRadius(300.f);
	EngineGlowL->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
	EngineGlowL->CastShadows = false;

	EngineGlowR = CreateDefaultSubobject<UPointLightComponent>(TEXT("EngineGlowR"));
	EngineGlowR->SetupAttachment(VehicleMesh);
	EngineGlowR->SetRelativeLocation(FVector(-80.f, 40.f, -30.f));
	EngineGlowR->SetIntensity(5000.f);
	EngineGlowR->SetAttenuationRadius(300.f);
	EngineGlowR->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
	EngineGlowR->CastShadows = false;

	CurrentBoostEnergy = BoostEnergy;

	AutoPossessAI = EAutoPossessAI::Disabled;
}

// ---------------------------------------------------------------------------
// IExoInteractable
// ---------------------------------------------------------------------------

void AExoHoverVehicle::Interact(AExoCharacter* Interactor)
{
	if (!Interactor || bIsOccupied) return;
	EnterVehicle(Interactor);
}

FString AExoHoverVehicle::GetInteractionPrompt()
{
	if (bIsOccupied) return FString();
	return TEXT("[E] Enter Hover Bike");
}

// ---------------------------------------------------------------------------
// Enter / Exit
// ---------------------------------------------------------------------------

void AExoHoverVehicle::EnterVehicle(AExoCharacter* Driver)
{
	if (!Driver || bIsOccupied) return;

	DriverCharacter = Driver;
	bIsOccupied = true;

	// Hide the character mesh
	Driver->SetActorHiddenInGame(true);
	Driver->SetActorEnableCollision(false);

	// Store the controller and possess the vehicle
	StoredDriverController = Driver->GetController();
	APlayerController* PC = Cast<APlayerController>(StoredDriverController);
	if (PC)
	{
		PC->Possess(this);
	}

	UE_LOG(LogExoRift, Log, TEXT("HoverVehicle: %s entered"), *Driver->GetName());
}

void AExoHoverVehicle::ExitVehicle()
{
	AExoCharacter* Driver = DriverCharacter.Get();
	if (!Driver) { bIsOccupied = false; return; }

	// Find a safe exit position to the right of the vehicle
	FVector ExitOffset = GetActorRightVector() * 200.f;
	FVector ExitLocation = GetActorLocation() + ExitOffset;

	// Trace down to find ground
	FHitResult Hit;
	FVector TraceStart = ExitLocation + FVector(0.f, 0.f, 500.f);
	FVector TraceEnd = ExitLocation - FVector(0.f, 0.f, 1000.f);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
	{
		ExitLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 90.f);
	}

	// Repossess the character
	APlayerController* PC = Cast<APlayerController>(StoredDriverController);
	if (PC)
	{
		PC->Possess(Driver);
	}

	Driver->SetActorLocation(ExitLocation);
	Driver->SetActorHiddenInGame(false);
	Driver->SetActorEnableCollision(true);

	// Reset vehicle state
	bIsOccupied = false;
	DriverCharacter = nullptr;
	StoredDriverController = nullptr;
	CurrentSpeed = 0.f;
	Velocity = FVector::ZeroVector;
	bIsBoosting = false;
	ThrottleInput = 0.f;
	SteerInput = 0.f;

	UE_LOG(LogExoRift, Log, TEXT("HoverVehicle: driver exited"));
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

void AExoHoverVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AExoHoverVehicle::HandleThrottle);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AExoHoverVehicle::HandleSteer);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AExoHoverVehicle::HandleBoostPressed);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AExoHoverVehicle::HandleBoostReleased);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AExoHoverVehicle::HandleExitVehicle);
}

void AExoHoverVehicle::HandleThrottle(float Value)
{
	ThrottleInput = FMath::Clamp(Value, -1.f, 1.f);
}

void AExoHoverVehicle::HandleSteer(float Value)
{
	SteerInput = FMath::Clamp(Value, -1.f, 1.f);
}

void AExoHoverVehicle::HandleBoostPressed()
{
	bIsBoosting = true;
}

void AExoHoverVehicle::HandleBoostReleased()
{
	bIsBoosting = false;
}

void AExoHoverVehicle::HandleExitVehicle()
{
	ExitVehicle();
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void AExoHoverVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyHoverForce(DeltaTime);

	if (bIsOccupied)
	{
		ApplyMovement(DeltaTime);
	}

	UpdateVFX(DeltaTime);
}

// ---------------------------------------------------------------------------
// Hover Physics
// ---------------------------------------------------------------------------

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

