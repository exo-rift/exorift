#include "Map/ExoLootContainer.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoArmorPickup.h"
#include "Weapons/ExoEnergyCellPickup.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "Visual/ExoPickupFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Core/ExoTypes.h"
#include "ExoRift.h"

AExoLootContainer::AExoLootContainer()
{
	PrimaryActorTick.bCanEverTick = true;

	ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
	ContainerMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ContainerMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ContainerMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = ContainerMesh;

	// Load basic shape for crate body
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded())
	{
		ContainerMesh->SetStaticMesh(CubeFind.Object);
		ContainerMesh->SetRelativeScale3D(FVector(1.0f, 0.6f, 0.5f));
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded())
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(MatFind.Object, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.08f, 0.1f, 0.06f));
		ContainerMesh->SetMaterial(0, Mat);
	}
}

void AExoLootContainer::BeginPlay()
{
	Super::BeginPlay();

	// Build visual details as runtime components
	BuildVisuals();
}

void AExoLootContainer::BuildVisuals()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* CubeMesh = CubeFind.Succeeded() ? CubeFind.Object : nullptr;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	UMaterialInterface* BaseMat = MatFind.Succeeded() ? MatFind.Object : nullptr;

	if (!CubeMesh || !BaseMat) return;

	auto MakePart = [&](const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(CubeMesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = true;
		C->RegisterComponent();
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
		if (Lum > 0.15f)
		{
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
		}
		C->SetMaterial(0, Mat);
		return C;
	};

	FLinearColor AccentColor(0.2f, 0.6f, 0.9f); // Blue accent

	// Lid (animates open)
	LidComp = MakePart(FVector(0.f, 0.f, 30.f), FVector(1.05f, 0.65f, 0.04f),
		FLinearColor(0.09f, 0.11f, 0.07f));

	// Accent stripes (brighter emissive)
	MakePart(FVector(0.f, 32.f, 0.f), FVector(0.9f, 0.02f, 0.4f), AccentColor);
	MakePart(FVector(0.f, -32.f, 0.f), FVector(0.9f, 0.02f, 0.4f), AccentColor);

	// Corner braces — L-shaped brackets at each corner of the crate
	FLinearColor BraceCol(0.12f, 0.12f, 0.14f);
	for (float SX : {-1.f, 1.f})
		for (float SY : {-1.f, 1.f})
		{
			FVector Corner(SX * 48.f, SY * 28.f, 0.f);
			MakePart(Corner + FVector(0.f, 0.f, 24.f), FVector(0.04f, 0.04f, 0.15f), BraceCol);
			MakePart(Corner + FVector(0.f, 0.f, -10.f), FVector(0.04f, 0.04f, 0.15f), BraceCol);
		}

	// Lock hasp on front
	MakePart(FVector(52.f, 0.f, 20.f), FVector(0.04f, 0.08f, 0.06f),
		FLinearColor(0.15f, 0.15f, 0.15f));

	// Top beacon light (subtle beam above closed container)
	MakePart(FVector(0.f, 0.f, 35.f), FVector(0.03f, 0.03f, 0.3f), AccentColor);

	// Edge trim strips (top edges)
	FLinearColor TrimCol(0.15f, 0.35f, 0.7f);
	MakePart(FVector(50.f, 0.f, 25.f), FVector(0.02f, 0.55f, 0.02f), TrimCol);
	MakePart(FVector(-50.f, 0.f, 25.f), FVector(0.02f, 0.55f, 0.02f), TrimCol);

	// Bottom accent stripe (glowing indicator of contents)
	MakePart(FVector(0.f, 0.f, -22.f), FVector(0.8f, 0.5f, 0.015f), AccentColor);

	// Interior glow light
	ContainerGlow = NewObject<UPointLightComponent>(this);
	ContainerGlow->SetupAttachment(RootComponent);
	ContainerGlow->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	ContainerGlow->SetIntensity(0.f);
	ContainerGlow->SetAttenuationRadius(500.f);
	ContainerGlow->SetLightColor(FColor(100, 200, 255));
	ContainerGlow->CastShadows = false;
	ContainerGlow->RegisterComponent();
}

void AExoLootContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == ELootContainerState::Opening)
	{
		OpenTimer += DeltaTime;

		// Animate lid opening during the open duration
		float OpenAlpha = FMath::Clamp(OpenTimer / OpenDuration, 0.f, 1.f);
		if (LidComp)
		{
			float LidAngle = OpenAlpha * -110.f;
			LidComp->SetRelativeRotation(FRotator(0.f, 0.f, LidAngle));
			LidComp->SetRelativeLocation(FVector(
				0.f, -20.f * OpenAlpha, 30.f + 15.f * OpenAlpha));
		}

		// Glow ramps up as lid opens
		if (ContainerGlow)
		{
			ContainerGlow->SetIntensity(OpenAlpha * 8000.f);
		}

		if (OpenTimer >= OpenDuration) FinishOpening();
	}
	else if (State == ELootContainerState::Closed)
	{
		float Time = GetWorld()->GetTimeSeconds();

		// Pulsing glow beacon on closed containers
		if (ContainerGlow)
		{
			float Pulse = 0.3f + 0.7f * FMath::Abs(FMath::Sin(Time * 1.5f));
			ContainerGlow->SetIntensity(Pulse * 800.f);
		}

		// Slow rotation of lid accent (visual hint the container is interactive)
		if (LidComp)
		{
			float Bob = FMath::Sin(Time * 2.f) * 0.3f;
			LidComp->SetRelativeLocation(FVector(0.f, 0.f, 30.f + Bob));
		}
	}
}

void AExoLootContainer::Interact(AExoCharacter* Interactor)
{
	if (State != ELootContainerState::Closed || !Interactor) return;
	StartOpening(Interactor);
}

FString AExoLootContainer::GetInteractionPrompt()
{
	return (State == ELootContainerState::Closed) ? TEXT("[E] Open Container") : FString();
}

void AExoLootContainer::StartOpening(AExoCharacter* Interactor)
{
	State = ELootContainerState::Opening;
	OpenTimer = 0.f;
	PendingInteractor = Interactor;

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayImpactSound(GetActorLocation(), false);
	}

	UE_LOG(LogExoRift, Log, TEXT("LootContainer %s: opening"), *GetName());
}

void AExoLootContainer::FinishOpening()
{
	State = ELootContainerState::Open;

	// Opening flash VFX — blue energy burst
	AExoPickupFlash::SpawnAt(GetWorld(),
		GetActorLocation() + FVector(0.f, 0.f, 50.f),
		FLinearColor(0.2f, 0.6f, 1.f));

	SpawnLoot();
	State = ELootContainerState::Empty;
	PendingInteractor = nullptr;

	// Dim the glow after emptied
	if (ContainerGlow)
	{
		ContainerGlow->SetIntensity(1000.f);
		ContainerGlow->SetLightColor(FColor(50, 80, 100));
	}

	UE_LOG(LogExoRift, Log, TEXT("LootContainer %s: opened and emptied"), *GetName());
}

void AExoLootContainer::SpawnLoot()
{
	const int32 ItemCount = FMath::RandRange(MinItems, MaxItems);
	const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, 80.f);
	FActorSpawnParameters Params;

	for (int32 i = 0; i < ItemCount; ++i)
	{
		// Spread items in an arc above the container
		float Angle = ((float)i / ItemCount) * PI + PI * 0.25f;
		FVector Offset(FMath::Cos(Angle) * 100.f, FMath::Sin(Angle) * 100.f, 30.f * i);
		FVector SpawnLoc = Origin + Offset;

		const float TypeRoll = FMath::FRand();

		if (TypeRoll < 0.5f)
		{
			auto* Pickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
				AExoWeaponPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
			if (Pickup)
			{
				Pickup->Rarity = RollRarity();
				Pickup->WeaponType = static_cast<EWeaponType>(FMath::RandRange(0, 5));
			}
		}
		else if (TypeRoll < 0.7f)
		{
			GetWorld()->SpawnActor<AExoEnergyCellPickup>(
				AExoEnergyCellPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
		}
		else if (TypeRoll < 0.85f)
		{
			GetWorld()->SpawnActor<AExoArmorPickup>(
				AExoArmorPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
		}
		else
		{
			GetWorld()->SpawnActor<AExoEnergyCellPickup>(
				AExoEnergyCellPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
		}
	}
}

FVector AExoLootContainer::ComputeEjectVelocity() const
{
	float Angle = FMath::FRandRange(0.f, 360.f);
	float Speed = FMath::FRandRange(200.f, 400.f);
	FVector Dir(FMath::Cos(FMath::DegreesToRadians(Angle)),
		FMath::Sin(FMath::DegreesToRadians(Angle)), 0.6f);
	return Dir.GetSafeNormal() * Speed;
}

EWeaponRarity AExoLootContainer::RollRarity() const
{
	const float Roll = FMath::FRand();
	if (Roll < 0.60f) return EWeaponRarity::Common;
	if (Roll < 0.85f) return EWeaponRarity::Rare;
	if (Roll < 0.95f) return EWeaponRarity::Epic;
	return EWeaponRarity::Legendary;
}
