// ExoAutoSlidingDoor.cpp — Automatic sliding door for buildings
#include "Map/ExoAutoSlidingDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Pawn.h"
#include "Core/ExoAudioManager.h"
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
	DoorLight->SetAttenuationRadius(800.f);
	DoorLight->CastShadows = false;

	// Try loading real door assets in quality order, fall back to cube
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SF_DoorFinder(
		TEXT("/Game/Meshes/SciFiDoor/Sci-fi-door"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> QT_DoorFinder(
		TEXT("/Game/Meshes/Quaternius_SciFi/Door_Single"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> KN_DoorFinder(
		TEXT("/Game/Meshes/Kenney_SpaceKit/door"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> KN_GateFinder(
		TEXT("/Game/Meshes/Kenney_SpaceKit/gate-door"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));

	UStaticMesh* DoorMesh = nullptr;
	if (SF_DoorFinder.Succeeded())       { DoorMesh = SF_DoorFinder.Object; }
	else if (QT_DoorFinder.Succeeded())  { DoorMesh = QT_DoorFinder.Object; }
	else if (KN_DoorFinder.Succeeded())  { DoorMesh = KN_DoorFinder.Object; }
	else if (KN_GateFinder.Succeeded())  { DoorMesh = KN_GateFinder.Object; }

	if (DoorMesh)
	{
		bUsingRealMesh = true;
		DoorFrame->SetStaticMesh(DoorMesh);
		DoorPanelL->SetStaticMesh(DoorMesh);
		DoorPanelR->SetStaticMesh(DoorMesh);
	}
	else if (CubeFinder.Succeeded())
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

	// Only apply dynamic material overrides for cube fallback —
	// real imported meshes keep their own PBR materials.
	if (!bUsingRealMesh && DoorFrame->GetStaticMesh())
	{
		UMaterialInterface* BaseMat = DoorFrame->GetMaterial(0);
		if (BaseMat)
		{
			AccentMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			if (!AccentMat) { return; }
			AccentMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.06f, 0.065f, 0.08f));
			DoorFrame->SetMaterial(0, AccentMat);
		}

		// Panel materials — slightly lighter than frame
		UMaterialInstanceDynamic* PanelMat = UMaterialInstanceDynamic::Create(
			DoorPanelL->GetMaterial(0), this);
		if (!PanelMat) { return; }
		PanelMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.08f, 0.085f, 0.1f));
		DoorPanelL->SetMaterial(0, PanelMat);

		UMaterialInstanceDynamic* PanelMatR = UMaterialInstanceDynamic::Create(
			DoorPanelR->GetMaterial(0), this);
		if (!PanelMatR) { return; }
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

	if (bUsingRealMesh)
	{
		// Real door meshes: frame is the static surround, panels slide apart.
		// Most imported door meshes are ~200cm wide x ~300cm tall at scale 1.
		// Scale uniformly to match requested dimensions.
		float MeshRefHeight = 300.f;
		float MeshRefWidth = 200.f;
		float ScaleH = DoorHeight / MeshRefHeight;
		float ScaleW = DoorWidth / MeshRefWidth;
		float UniformScale = FMath::Min(ScaleH, ScaleW);

		// Frame sits centered at ground level (mesh origin assumed at bottom)
		DoorFrame->SetRelativeLocation(FVector::ZeroVector);
		DoorFrame->SetRelativeScale3D(FVector(UniformScale));
		DoorFrame->SetVisibility(false); // hide frame; real mesh on panels suffices

		// Left panel — shifted left half of doorway
		DoorPanelL->SetRelativeLocation(FVector(0.f, -DoorWidth * 0.25f, 0.f));
		DoorPanelL->SetRelativeScale3D(FVector(UniformScale * 0.5f, UniformScale * 0.5f, UniformScale));

		// Right panel — shifted right half of doorway
		DoorPanelR->SetRelativeLocation(FVector(0.f, DoorWidth * 0.25f, 0.f));
		DoorPanelR->SetRelativeScale3D(FVector(UniformScale * 0.5f, UniformScale * 0.5f, UniformScale));
	}
	else
	{
		// Cube fallback — build frame + panels from primitives
		float HalfW = DoorWidth * 0.5f;
		float PanelW = HalfW * 0.48f; // slight gap in center
		float FrameThick = 15.f;

		// Top beam
		DoorFrame->SetRelativeLocation(FVector(0.f, 0.f, DoorHeight));
		DoorFrame->SetRelativeScale3D(FVector(FrameThick / 100.f, DoorWidth / 100.f, FrameThick / 100.f));

		// Panels — each covers half the doorway
		DoorPanelL->SetRelativeLocation(FVector(0.f, -PanelW * 0.5f, DoorHeight * 0.5f));
		DoorPanelL->SetRelativeScale3D(FVector(FrameThick / 100.f, PanelW / 100.f, DoorHeight / 100.f));

		DoorPanelR->SetRelativeLocation(FVector(0.f, PanelW * 0.5f, DoorHeight * 0.5f));
		DoorPanelR->SetRelativeScale3D(FVector(FrameThick / 100.f, PanelW / 100.f, DoorHeight / 100.f));
	}

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
	float Intensity = FMath::Lerp(3000.f, 8000.f, OpenBlend);
	DoorLight->SetIntensity(Intensity);
}

void AExoAutoSlidingDoor::OnTriggerEnter(UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (Cast<APawn>(OtherActor))
	{
		OverlapCount++;

		// Sci-fi door slide sound when opening
		if (OverlapCount == 1)
		{
			if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
			{
				Audio->PlayDoorSlideSound(GetActorLocation(), true);
			}
		}
	}
}

void AExoAutoSlidingDoor::OnTriggerExit(UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (Cast<APawn>(OtherActor))
	{
		OverlapCount = FMath::Max(0, OverlapCount - 1);

		// Sci-fi door slide sound when closing (descending pitch)
		if (OverlapCount == 0)
		{
			if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
			{
				Audio->PlayDoorSlideSound(GetActorLocation(), false);
			}
		}
	}
}
