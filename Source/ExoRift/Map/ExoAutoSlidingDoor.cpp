// ExoAutoSlidingDoor.cpp — Automatic sliding door for buildings
#include "Map/ExoAutoSlidingDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AExoAutoSlidingDoor::AExoAutoSlidingDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Door frame (static surround)
	DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrame->SetupAttachment(Root);

	// Left panel (slides left to open)
	DoorPanelL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanelL"));
	DoorPanelL->SetupAttachment(Root);

	// Right panel (slides right to open)
	DoorPanelR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanelR"));
	DoorPanelR->SetupAttachment(Root);

	// Trigger volume
	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(Root);
	TriggerVolume->SetSphereRadius(400.f);
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetGenerateOverlapEvents(true);

	// Accent light above the door
	DoorLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DoorLight"));
	DoorLight->SetupAttachment(Root);
	DoorLight->SetIntensity(2000.f);
	DoorLight->SetAttenuationRadius(500.f);
	DoorLight->CastShadows = false;

	// Load engine primitives
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		DoorFrame->SetStaticMesh(CubeFinder.Object);
		DoorPanelL->SetStaticMesh(CubeFinder.Object);
		DoorPanelR->SetStaticMesh(CubeFinder.Object);
	}

	DoorFrame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoorPanelL->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorPanelR->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorPanelL->SetCollisionResponseToAllChannels(ECR_Block);
	DoorPanelR->SetCollisionResponseToAllChannels(ECR_Block);
}

void AExoAutoSlidingDoor::BeginPlay()
{
	Super::BeginPlay();

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AExoAutoSlidingDoor::OnTriggerEnter);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AExoAutoSlidingDoor::OnTriggerExit);

	// Create dynamic material for accent coloring
	if (DoorFrame->GetStaticMesh())
	{
		UMaterialInterface* BaseMat = DoorFrame->GetMaterial(0);
		if (BaseMat)
		{
			AccentMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			AccentMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.06f, 0.065f, 0.08f));
			DoorFrame->SetMaterial(0, AccentMat);
		}

		// Panel materials — slightly lighter than frame
		UMaterialInstanceDynamic* PanelMat = UMaterialInstanceDynamic::Create(
			DoorPanelL->GetMaterial(0), this);
		PanelMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.08f, 0.085f, 0.1f));
		DoorPanelL->SetMaterial(0, PanelMat);

		UMaterialInstanceDynamic* PanelMatR = UMaterialInstanceDynamic::Create(
			DoorPanelR->GetMaterial(0), this);
		PanelMatR->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.08f, 0.085f, 0.1f));
		DoorPanelR->SetMaterial(0, PanelMatR);
	}

	// Store closed positions
	ClosedOffsetL = DoorPanelL->GetRelativeLocation();
	ClosedOffsetR = DoorPanelR->GetRelativeLocation();
}

void AExoAutoSlidingDoor::InitDoor(float Width, float Height, float InSlideDistance,
	const FLinearColor& AccentColor)
{
	DoorWidth = Width;
	DoorHeight = Height;
	SlideAmount = InSlideDistance;

	float HalfW = DoorWidth * 0.5f;
	float PanelW = HalfW * 0.48f; // slight gap in center

	// Frame — thin border around the opening
	float FrameThick = 15.f;
	// Top beam
	DoorFrame->SetRelativeLocation(FVector(0.f, 0.f, DoorHeight));
	DoorFrame->SetRelativeScale3D(FVector(FrameThick / 100.f, DoorWidth / 100.f, FrameThick / 100.f));

	// Panels — each covers half the doorway
	DoorPanelL->SetRelativeLocation(FVector(0.f, -PanelW * 0.5f, DoorHeight * 0.5f));
	DoorPanelL->SetRelativeScale3D(FVector(FrameThick / 100.f, PanelW / 100.f, DoorHeight / 100.f));

	DoorPanelR->SetRelativeLocation(FVector(0.f, PanelW * 0.5f, DoorHeight * 0.5f));
	DoorPanelR->SetRelativeScale3D(FVector(FrameThick / 100.f, PanelW / 100.f, DoorHeight / 100.f));

	// Trigger volume sized to doorway
	TriggerVolume->SetSphereRadius(FMath::Max(DoorWidth, DoorHeight) * 0.8f);

	// Light above the door
	DoorLight->SetRelativeLocation(FVector(0.f, 0.f, DoorHeight + 30.f));
	DoorLight->SetLightColor(AccentColor);

	// Update closed offsets
	ClosedOffsetL = DoorPanelL->GetRelativeLocation();
	ClosedOffsetR = DoorPanelR->GetRelativeLocation();
}

void AExoAutoSlidingDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Target = (OverlapCount > 0) ? 1.f : 0.f;
	OpenBlend = FMath::FInterpTo(OpenBlend, Target, DeltaTime, OpenSpeed);

	// Slide panels apart along local Y axis
	FVector OffsetL = ClosedOffsetL + FVector(0.f, -SlideAmount * OpenBlend, 0.f);
	FVector OffsetR = ClosedOffsetR + FVector(0.f, SlideAmount * OpenBlend, 0.f);
	DoorPanelL->SetRelativeLocation(OffsetL);
	DoorPanelR->SetRelativeLocation(OffsetR);

	// Pulse the door light brighter when open
	float Intensity = FMath::Lerp(1500.f, 4000.f, OpenBlend);
	DoorLight->SetIntensity(Intensity);
}

void AExoAutoSlidingDoor::OnTriggerEnter(UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (Cast<APawn>(OtherActor))
	{
		OverlapCount++;
	}
}

void AExoAutoSlidingDoor::OnTriggerExit(UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (Cast<APawn>(OtherActor))
	{
		OverlapCount = FMath::Max(0, OverlapCount - 1);
	}
}
