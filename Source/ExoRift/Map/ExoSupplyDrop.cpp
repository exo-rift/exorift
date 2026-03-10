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
	for (float cx : {-55.f, 55.f})
		for (float cy : {-35.f, 35.f})
			MakePart(CubeMeshRef, FVector(cx, cy, -20.f),
				FVector(0.08f, 0.08f, 0.15f), FLinearColor(0.15f, 0.15f, 0.15f));

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

	// Top edge trim (gold lines)
	MakePart(CubeMeshRef, FVector(60.f, 0.f, 32.f), FVector(0.02f, 0.7f, 0.02f), AccentColor);
	MakePart(CubeMeshRef, FVector(-60.f, 0.f, 32.f), FVector(0.02f, 0.7f, 0.02f), AccentColor);

	// Hazard stripe on front face
	MakePart(CubeMeshRef, FVector(62.f, 0.f, 0.f), FVector(0.02f, 0.5f, 0.1f),
		FLinearColor(0.6f, 0.5f, 0.05f));

	// Lock hasp (front center)
	MakePart(CubeMeshRef, FVector(62.f, 0.f, 25.f), FVector(0.04f, 0.08f, 0.06f),
		FLinearColor(0.15f, 0.15f, 0.15f));

	// Inner crate glow
	CrateGlow = NewObject<UPointLightComponent>(this);
	CrateGlow->SetupAttachment(RootComponent);
	CrateGlow->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	CrateGlow->SetIntensity(0.f);
	CrateGlow->SetAttenuationRadius(500.f);
	CrateGlow->SetLightColor(FColor(255, 200, 50));
	CrateGlow->RegisterComponent();

	// Smoke trail — elongated cylinder trailing above during descent
	SmokeTrail = MakePart(CylinderMeshRef, FVector(0.f, 0.f, 500.f),
		FVector(0.8f, 0.8f, 5.f), FLinearColor(0.15f, 0.12f, 0.08f));
	if (SmokeTrail) SmokeTrail->CastShadow = false;

	// Trail light — warm glow from engine exhaust
	TrailLight = NewObject<UPointLightComponent>(this);
	TrailLight->SetupAttachment(RootComponent);
	TrailLight->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	TrailLight->SetIntensity(15000.f);
	TrailLight->SetAttenuationRadius(2000.f);
	TrailLight->SetLightColor(FLinearColor(1.f, 0.5f, 0.1f));
	TrailLight->CastShadows = false;
	TrailLight->RegisterComponent();
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
			float TLPulse = 12000.f + 6000.f * FMath::Sin(Time * 18.f);
			TrailLight->SetIntensity(TLPulse);
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
					FLinearColor(1.5f * Alpha, 0.8f * Alpha, 0.2f * Alpha));
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
			CrateGlow->SetIntensity(LidOpenAlpha * 15000.f * GlowPulse);
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

		// Spawn impact ring — expanding dust cloud
		if (CylinderMeshRef && BaseMaterialRef)
		{
			ImpactRing = NewObject<UStaticMeshComponent>(this);
			ImpactRing->SetupAttachment(RootComponent);
			ImpactRing->SetStaticMesh(CylinderMeshRef);
			ImpactRing->SetRelativeLocation(FVector(0.f, 0.f, 5.f));
			ImpactRing->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.02f));
			ImpactRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ImpactRing->CastShadow = false;
			ImpactRing->RegisterComponent();
			ImpactRingMat = UMaterialInstanceDynamic::Create(BaseMaterialRef, this);
			ImpactRingMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.6f, 0.4f, 0.15f));
			ImpactRingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(1.5f, 0.8f, 0.2f));
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
