// ExoHoverVehicle.cpp — Constructor, input, physics, enter/exit
// VFX (BeginPlay, UpdateVFX) in ExoHoverVehicleVFX.cpp
#include "Vehicles/ExoHoverVehicle.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Core/ExoAudioManager.h"
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
	EngineGlowL->SetIntensity(12000.f);
	EngineGlowL->SetAttenuationRadius(300.f);
	EngineGlowL->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
	EngineGlowL->CastShadows = false;

	EngineGlowR = CreateDefaultSubobject<UPointLightComponent>(TEXT("EngineGlowR"));
	EngineGlowR->SetupAttachment(VehicleMesh);
	EngineGlowR->SetRelativeLocation(FVector(-80.f, 40.f, -30.f));
	EngineGlowR->SetIntensity(12000.f);
	EngineGlowR->SetAttenuationRadius(300.f);
	EngineGlowR->SetLightColor(FLinearColor(0.1f, 0.5f, 1.f));
	EngineGlowR->CastShadows = false;

	// Headlights — forward-facing spotlights
	HeadlightL = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightL"));
	HeadlightL->SetupAttachment(VehicleMesh);
	HeadlightL->SetRelativeLocation(FVector(90.f, -30.f, 5.f));
	HeadlightL->SetRelativeRotation(FRotator(-5.f, 0.f, 0.f));
	HeadlightL->SetIntensity(15000.f);
	HeadlightL->SetOuterConeAngle(30.f);
	HeadlightL->SetInnerConeAngle(15.f);
	HeadlightL->SetAttenuationRadius(5000.f);
	HeadlightL->SetLightColor(FLinearColor(0.9f, 0.95f, 1.f));
	HeadlightL->CastShadows = false;

	HeadlightR = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightR"));
	HeadlightR->SetupAttachment(VehicleMesh);
	HeadlightR->SetRelativeLocation(FVector(90.f, 30.f, 5.f));
	HeadlightR->SetRelativeRotation(FRotator(-5.f, 0.f, 0.f));
	HeadlightR->SetIntensity(15000.f);
	HeadlightR->SetOuterConeAngle(30.f);
	HeadlightR->SetInnerConeAngle(15.f);
	HeadlightR->SetAttenuationRadius(5000.f);
	HeadlightR->SetLightColor(FLinearColor(0.9f, 0.95f, 1.f));
	HeadlightR->CastShadows = false;

	// Body detail — windshield visor, side panels, rear fin
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFinder.Succeeded())
	{
		Windshield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Windshield"));
		Windshield->SetupAttachment(VehicleMesh);
		Windshield->SetStaticMesh(CubeFinder.Object);
		Windshield->SetRelativeLocation(FVector(40.f, 0.f, 20.f));
		Windshield->SetRelativeScale3D(FVector(0.2f, 0.6f, 0.01f));
		Windshield->SetRelativeRotation(FRotator(30.f, 0.f, 0.f));
		Windshield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Windshield->CastShadow = false;

		SidePanelL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SidePanelL"));
		SidePanelL->SetupAttachment(VehicleMesh);
		SidePanelL->SetStaticMesh(CubeFinder.Object);
		SidePanelL->SetRelativeLocation(FVector(0.f, -55.f, 5.f));
		SidePanelL->SetRelativeScale3D(FVector(0.8f, 0.02f, 0.12f));
		SidePanelL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SidePanelL->CastShadow = false;

		SidePanelR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SidePanelR"));
		SidePanelR->SetupAttachment(VehicleMesh);
		SidePanelR->SetStaticMesh(CubeFinder.Object);
		SidePanelR->SetRelativeLocation(FVector(0.f, 55.f, 5.f));
		SidePanelR->SetRelativeScale3D(FVector(0.8f, 0.02f, 0.12f));
		SidePanelR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SidePanelR->CastShadow = false;

		RearFin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearFin"));
		RearFin->SetupAttachment(VehicleMesh);
		RearFin->SetStaticMesh(CubeFinder.Object);
		RearFin->SetRelativeLocation(FVector(-75.f, 0.f, 25.f));
		RearFin->SetRelativeScale3D(FVector(0.15f, 0.01f, 0.2f));
		RearFin->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RearFin->CastShadow = false;
	}

	// Hover dust cloud meshes (scaled spheres underneath)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphFinder.Succeeded())
	{
		HoverDustL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HoverDustL"));
		HoverDustL->SetupAttachment(VehicleMesh);
		HoverDustL->SetStaticMesh(SphFinder.Object);
		HoverDustL->SetRelativeLocation(FVector(-40.f, -30.f, -80.f));
		HoverDustL->SetRelativeScale3D(FVector(0.01f));
		HoverDustL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HoverDustL->CastShadow = false;
		HoverDustL->SetVisibility(false);

		HoverDustR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HoverDustR"));
		HoverDustR->SetupAttachment(VehicleMesh);
		HoverDustR->SetStaticMesh(SphFinder.Object);
		HoverDustR->SetRelativeLocation(FVector(-40.f, 30.f, -80.f));
		HoverDustR->SetRelativeScale3D(FVector(0.01f));
		HoverDustR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HoverDustR->CastShadow = false;
		HoverDustR->SetVisibility(false);
	}

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

	// Engine startup hum — ascending sci-fi whir
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayVehicleEngineSound(GetActorLocation(), true);
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

	// Engine shutdown sound
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayVehicleEngineSound(GetActorLocation(), false);
	}

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

// Hover physics in ExoHoverVehiclePhysics.cpp

