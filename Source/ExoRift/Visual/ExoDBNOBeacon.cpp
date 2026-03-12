// ExoDBNOBeacon.cpp — Pulsing distress beacon above DBNO players
#include "Visual/ExoDBNOBeacon.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoDBNOBeacon::AExoDBNOBeacon()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(
		TEXT("/Engine/BasicShapes/Cube"));

	// Vertical beam — tall thin cylinder
	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
	RootComponent = BeamMesh;
	if (CylF.Succeeded()) BeamMesh->SetStaticMesh(CylF.Object);
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->CastShadow = false;
	BeamMesh->SetRelativeScale3D(FVector(BeamRadius, BeamRadius, BeamHeight / 100.f));
	BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, BeamHeight * 0.5f));

	// Diamond cap — rotated cube at beam top
	DiamondCap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DiamondCap"));
	DiamondCap->SetupAttachment(BeamMesh);
	if (CubeF.Succeeded()) DiamondCap->SetStaticMesh(CubeF.Object);
	DiamondCap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DiamondCap->CastShadow = false;
	DiamondCap->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));
	// Place at the top of the beam; beam pivot is center so offset up by half
	DiamondCap->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	DiamondCap->SetRelativeRotation(FRotator(45.f, 0.f, 45.f));

	// Pulsing point light
	PulseLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PulseLight"));
	PulseLight->SetupAttachment(BeamMesh);
	PulseLight->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	PulseLight->SetIntensity(40000.f);
	PulseLight->SetAttenuationRadius(1200.f);
	PulseLight->SetLightColor(FLinearColor(1.f, 0.6f, 0.1f));
	PulseLight->CastShadows = false;
}

void AExoDBNOBeacon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	Age += DeltaTime;

	// Sine pulse: 0..1 range at PulseFreq Hz
	float Pulse = 0.5f + 0.5f * FMath::Sin(Age * PulseFreq * 2.f * PI);
	float Brightness = FMath::Lerp(2.f, 12.f, Pulse);

	// Warm orange-yellow: R high, G medium, B low
	FLinearColor BeamCol(1.0f * Brightness, 0.55f * Brightness, 0.05f * Brightness);
	FLinearColor CapCol(1.0f * Brightness * 1.5f, 0.65f * Brightness * 1.5f, 0.1f * Brightness);

	if (BeamMat) BeamMat->SetVectorParameterValue(TEXT("EmissiveColor"), BeamCol);
	if (CapMat) CapMat->SetVectorParameterValue(TEXT("EmissiveColor"), CapCol);

	// Light pulses with the beam
	PulseLight->SetIntensity(FMath::Lerp(10000.f, 50000.f, Pulse));

	// Slow diamond rotation
	DiamondCap->AddRelativeRotation(FRotator(0.f, CapSpinRate * DeltaTime, 0.f));
}

AExoDBNOBeacon* AExoDBNOBeacon::SpawnBeacon(UWorld* World, AActor* AttachTo)
{
	if (!World || !AttachTo) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoDBNOBeacon* Beacon = World->SpawnActor<AExoDBNOBeacon>(
		AExoDBNOBeacon::StaticClass(), AttachTo->GetActorLocation(),
		FRotator::ZeroRotator, Params);
	if (!Beacon) return nullptr;

	Beacon->AttachToActor(AttachTo, FAttachmentTransformRules::KeepWorldTransform);

	// Apply emissive additive material to both meshes
	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (AddMat)
	{
		Beacon->BeamMat = UMaterialInstanceDynamic::Create(AddMat, Beacon);
		if (!Beacon->BeamMat) { return nullptr; }
		Beacon->BeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(6.f, 3.3f, 0.3f));
		Beacon->BeamMesh->SetMaterial(0, Beacon->BeamMat);

		Beacon->CapMat = UMaterialInstanceDynamic::Create(AddMat, Beacon);
		if (!Beacon->CapMat) { return nullptr; }
		Beacon->CapMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(9.f, 5.f, 0.5f));
		Beacon->DiamondCap->SetMaterial(0, Beacon->CapMat);
	}

	return Beacon;
}

void AExoDBNOBeacon::DestroyBeacon()
{
	Destroy();
}
