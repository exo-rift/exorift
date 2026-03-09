#include "Map/ExoDropPod.h"
#include "Map/ExoDropPodManager.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ExoRift.h"

AExoDropPod::AExoDropPod()
{
	PrimaryActorTick.bCanEverTick = true;

	PodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PodMesh"));
	RootComponent = PodMesh;
	PodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Use a basic shape — will be replaced with real asset
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_Cylinder"));
	if (MeshFinder.Succeeded())
	{
		PodMesh->SetStaticMesh(MeshFinder.Object);
		PodMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 2.5f));
	}

	PodCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PodCamera"));
	PodCamera->SetupAttachment(PodMesh);
	PodCamera->SetRelativeLocation(FVector(0.f, 0.f, 300.f));
	PodCamera->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
}

void AExoDropPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasLanded) return;

	// Detect ground
	if (!bGroundDetected)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, GroundTraceLength);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			GroundZ = Hit.ImpactPoint.Z + 200.f; // Offset above ground
			bGroundDetected = true;
		}
	}

	// Calculate descent speed
	float CurrentZ = GetActorLocation().Z;
	float AltitudeAboveGround = CurrentZ - GroundZ;
	float CurrentSpeed = DescentSpeed;

	if (bGroundDetected && AltitudeAboveGround < DecelerationAltitude)
	{
		float Alpha = AltitudeAboveGround / DecelerationAltitude;
		CurrentSpeed = FMath::Lerp(LandingSpeed, DescentSpeed, Alpha);
	}

	// Move down
	FVector NewLoc = GetActorLocation() - FVector(0.f, 0.f, CurrentSpeed * DeltaTime);

	if (bGroundDetected && NewLoc.Z <= GroundZ)
	{
		NewLoc.Z = GroundZ;
		SetActorLocation(NewLoc);
		OnLanded();
	}
	else
	{
		SetActorLocation(NewLoc);
	}
}

void AExoDropPod::InitPod(AController* InPassenger, AExoDropPodManager* InManager)
{
	Passenger = InPassenger;
	Manager = InManager;
}

void AExoDropPod::OnLanded()
{
	if (bHasLanded) return;
	bHasLanded = true;

	UE_LOG(LogExoRift, Log, TEXT("Drop pod landed at %s"), *GetActorLocation().ToString());

	// Place or spawn character at landing position
	if (Passenger)
	{
		FVector SpawnLoc = GetActorLocation() + FVector(200.f, 0.f, 100.f);
		FRotator SpawnRot = GetActorRotation();

		if (Passenger->GetPawn())
		{
			Passenger->GetPawn()->SetActorLocation(SpawnLoc);
		}
		else
		{
			AExoCharacter* NewChar = GetWorld()->SpawnActor<AExoCharacter>(
				AExoCharacter::StaticClass(), SpawnLoc, SpawnRot);
			if (NewChar) Passenger->Possess(NewChar);
		}
	}

	if (Manager)
	{
		Manager->OnPodLanded(this);
	}

	// Destroy pod after a delay
	SetLifeSpan(5.f);
}
