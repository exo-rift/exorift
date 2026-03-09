#include "Map/ExoSupplyDrop.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoEnergyCellPickup.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Core/ExoAudioManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
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

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatF.Succeeded()) BaseMaterialRef = MatF.Object;
}

void AExoSupplyDrop::BuildCrateMesh()
{
	auto MakePart = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator) -> UStaticMeshComponent*
	{
		if (!Mesh) return nullptr;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = true;
		C->RegisterComponent();
		if (BaseMaterialRef)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterialRef, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			C->SetMaterial(0, Mat);
		}
		return C;
	};

	FLinearColor CrateColor(0.1f, 0.12f, 0.08f);      // Military olive
	FLinearColor AccentColor(0.9f, 0.7f, 0.1f);         // Gold accent
	FLinearColor ParaColor(0.8f, 0.4f, 0.1f);           // Orange chute

	// Main crate body
	CrateBody = MakePart(CubeMeshRef, FVector(0.f, 0.f, 0.f),
		FVector(1.2f, 0.8f, 0.6f), CrateColor);

	// Lid (will animate open)
	CrateLid = MakePart(CubeMeshRef, FVector(0.f, 0.f, 35.f),
		FVector(1.25f, 0.85f, 0.05f), CrateColor);

	// Gold accent stripes on sides
	MakePart(CubeMeshRef, FVector(0.f, 42.f, 0.f), FVector(1.0f, 0.02f, 0.45f), AccentColor);
	MakePart(CubeMeshRef, FVector(0.f, -42.f, 0.f), FVector(1.0f, 0.02f, 0.45f), AccentColor);

	// Corner reinforcements
	for (int32 x = -1; x <= 1; x += 2)
	{
		for (int32 y = -1; y <= 1; y += 2)
		{
			MakePart(CubeMeshRef,
				FVector(x * 55.f, y * 35.f, -20.f),
				FVector(0.08f, 0.08f, 0.15f),
				FLinearColor(0.15f, 0.15f, 0.15f));
		}
	}

	// Parachute dome (half sphere above)
	ParachuteDome = MakePart(SphereMeshRef, FVector(0.f, 0.f, 350.f),
		FVector(2.5f, 2.5f, 1.5f), ParaColor);

	// Parachute struts (lines connecting dome to crate)
	ParachuteStrut1 = MakePart(CylinderMeshRef, FVector(40.f, 0.f, 180.f),
		FVector(0.02f, 0.02f, 2.f), FLinearColor(0.3f, 0.3f, 0.3f),
		FRotator(0.f, 0.f, 8.f));
	ParachuteStrut2 = MakePart(CylinderMeshRef, FVector(-40.f, 0.f, 180.f),
		FVector(0.02f, 0.02f, 2.f), FLinearColor(0.3f, 0.3f, 0.3f),
		FRotator(0.f, 0.f, -8.f));

	// Inner crate glow
	CrateGlow = NewObject<UPointLightComponent>(this);
	CrateGlow->SetupAttachment(RootComponent);
	CrateGlow->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	CrateGlow->SetIntensity(0.f);
	CrateGlow->SetAttenuationRadius(500.f);
	CrateGlow->SetLightColor(FColor(255, 200, 50));
	CrateGlow->RegisterComponent();
}

void AExoSupplyDrop::BeginPlay()
{
	Super::BeginPlay();
	BuildCrateMesh();
	TransitionToState(ESupplyDropState::Falling);
}

void AExoSupplyDrop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ESupplyDropState::Falling)
	{
		TickFalling(DeltaTime);
	}

	// Animate lid opening
	if (CurrentState == ESupplyDropState::Opened || CurrentState == ESupplyDropState::Depleted)
	{
		LidOpenAlpha = FMath::FInterpTo(LidOpenAlpha, 1.f, DeltaTime, 3.f);
		if (CrateLid)
		{
			// Hinge the lid open
			float LidAngle = LidOpenAlpha * -120.f;
			CrateLid->SetRelativeRotation(FRotator(0.f, 0.f, LidAngle));
			CrateLid->SetRelativeLocation(FVector(0.f, -30.f * LidOpenAlpha, 35.f + 20.f * LidOpenAlpha));
		}

		// Glow from inside when opened
		if (CrateGlow)
		{
			CrateGlow->SetIntensity(LidOpenAlpha * 15000.f);
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
		// Remove parachute on landing
		if (ParachuteDome) { ParachuteDome->DestroyComponent(); ParachuteDome = nullptr; }
		if (ParachuteStrut1) { ParachuteStrut1->DestroyComponent(); ParachuteStrut1 = nullptr; }
		if (ParachuteStrut2) { ParachuteStrut2->DestroyComponent(); ParachuteStrut2 = nullptr; }
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

void AExoSupplyDrop::SpawnLoot()
{
	FVector Origin = GetActorLocation();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn 2 weapons at Epic or Legendary rarity
	const EWeaponType WeaponTypes[] = {
		EWeaponType::Rifle, EWeaponType::Pistol, EWeaponType::GrenadeLauncher };
	const EWeaponRarity HighRarities[] = {
		EWeaponRarity::Epic, EWeaponRarity::Legendary };

	for (int32 i = 0; i < 2; ++i)
	{
		float Angle = i * PI;
		FVector Offset(FMath::Cos(Angle) * 120.f, FMath::Sin(Angle) * 120.f, 30.f);
		FVector SpawnLoc = Origin + Offset;

		AExoWeaponPickup* WeaponPickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
			AExoWeaponPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (WeaponPickup)
		{
			WeaponPickup->WeaponType = WeaponTypes[FMath::RandRange(0, 2)];
			WeaponPickup->Rarity = HighRarities[FMath::RandRange(0, 1)];
		}
	}

	// Spawn 2 energy cells
	for (int32 i = 0; i < 2; ++i)
	{
		float Angle = (i * PI) + PI * 0.5f;
		FVector Offset(FMath::Cos(Angle) * 100.f, FMath::Sin(Angle) * 100.f, 20.f);
		FVector SpawnLoc = Origin + Offset;

		AExoEnergyCellPickup* CellPickup = GetWorld()->SpawnActor<AExoEnergyCellPickup>(
			AExoEnergyCellPickup::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (CellPickup)
		{
			CellPickup->EnergyAmount = 100.f;
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("Supply drop spawned loot at %s"), *Origin.ToString());
}
