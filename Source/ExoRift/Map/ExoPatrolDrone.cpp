// ExoPatrolDrone.cpp — Ambient flying patrol drone for atmosphere
#include "Map/ExoPatrolDrone.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoPatrolDrone::AExoPatrolDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));

	// Body — flat rectangular chassis
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = BodyMesh;
	if (CubeFind.Succeeded()) BodyMesh->SetStaticMesh(CubeFind.Object);
	BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.3f, 0.1f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->CastShadow = false;

	// Left rotor disc
	RotorL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RotorL"));
	RotorL->SetupAttachment(BodyMesh);
	if (CylFind.Succeeded()) RotorL->SetStaticMesh(CylFind.Object);
	RotorL->SetRelativeLocation(FVector(20.f, -20.f, 8.f));
	RotorL->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.005f));
	RotorL->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RotorL->CastShadow = false;

	// Right rotor disc
	RotorR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RotorR"));
	RotorR->SetupAttachment(BodyMesh);
	if (CylFind.Succeeded()) RotorR->SetStaticMesh(CylFind.Object);
	RotorR->SetRelativeLocation(FVector(20.f, 20.f, 8.f));
	RotorR->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.005f));
	RotorR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RotorR->CastShadow = false;

	// Main navigation light
	DroneLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DroneLight"));
	DroneLight->SetupAttachment(BodyMesh);
	DroneLight->SetRelativeLocation(FVector(30.f, 0.f, 0.f));
	DroneLight->SetIntensity(4000.f);
	DroneLight->SetAttenuationRadius(500.f);
	DroneLight->CastShadows = false;

	// Downward scan light (searching beam feel)
	ScanLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ScanLight"));
	ScanLight->SetupAttachment(BodyMesh);
	ScanLight->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	ScanLight->SetIntensity(2000.f);
	ScanLight->SetAttenuationRadius(400.f);
	ScanLight->CastShadows = false;
}

void AExoPatrolDrone::InitDrone(const TArray<FVector>& InWaypoints,
	const FLinearColor& Color, float InSpeed, float InHoverHeight)
{
	Waypoints = InWaypoints;
	Speed = InSpeed;
	HoverHeight = InHoverHeight;
	BobPhase = FMath::FRand() * PI * 2.f;

	DroneLight->SetLightColor(Color);
	ScanLight->SetLightColor(FLinearColor(Color.R * 0.5f, Color.G * 0.5f, Color.B * 0.5f));

	// Dark metallic body with accent
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	{
		UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* BodyMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 2.f);
		BodyMesh->SetMaterial(0, BodyMat);

		UMaterialInstanceDynamic* RotorMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		RotorMat->SetVectorParameterValue(TEXT("EmissiveColor"), Color * 0.5f);
		RotorL->SetMaterial(0, RotorMat);
		RotorR->SetMaterial(0, RotorMat);
	}

	// Start at first waypoint
	if (Waypoints.Num() > 0)
	{
		FVector Start = Waypoints[0];
		Start.Z += HoverHeight;
		SetActorLocation(Start);
	}
}

void AExoPatrolDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Waypoints.Num() < 2) return;

	// Move toward current waypoint
	FVector Target = Waypoints[CurrentWaypoint];
	Target.Z += HoverHeight;

	FVector Pos = GetActorLocation();
	FVector Dir = Target - Pos;
	float Dist = Dir.Size();

	if (Dist < 100.f)
	{
		CurrentWaypoint = (CurrentWaypoint + 1) % Waypoints.Num();
	}
	else
	{
		Dir.Normalize();
		FVector NewPos = Pos + Dir * Speed * DeltaTime;

		// Gentle hover bob
		BobPhase += DeltaTime;
		NewPos.Z += FMath::Sin(BobPhase * 2.f) * 15.f;

		SetActorLocation(NewPos);

		// Face direction of travel with gentle banking
		FRotator TargetRot = Dir.Rotation();
		float Bank = FMath::Clamp(Dir.Y * 5.f, -15.f, 15.f);
		TargetRot.Roll = Bank;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 3.f));
	}

	// Spin rotors
	RotorAngle += DeltaTime * 1800.f;
	RotorL->SetRelativeRotation(FRotator(0.f, RotorAngle, 0.f));
	RotorR->SetRelativeRotation(FRotator(0.f, -RotorAngle, 0.f));

	// Pulsing scan light
	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = 0.6f + 0.4f * FMath::Sin(Time * 3.f + BobPhase);
	ScanLight->SetIntensity(2000.f * Pulse);
}
