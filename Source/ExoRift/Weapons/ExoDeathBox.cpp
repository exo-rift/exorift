// ExoDeathBox.cpp — Loot container at player death location
#include "Weapons/ExoDeathBox.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoPickupFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "ExoRift.h"

AExoDeathBox::AExoDeathBox()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 125.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	UStaticMesh* Cyl = CylFinder.Succeeded() ? CylFinder.Object : nullptr;

	auto MakeMesh = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) C->SetStaticMesh(Mesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		return C;
	};

	BoxBody = MakeMesh(TEXT("Body"), Cube);
	RootComponent = BoxBody;
	BoxBody->SetRelativeScale3D(FVector(0.5f, 0.35f, 0.25f));
	BoxBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxBody->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	BoxLid = MakeMesh(TEXT("Lid"), Cube);
	BoxLid->SetupAttachment(BoxBody);
	BoxLid->SetRelativeLocation(FVector(0.f, 0.f, 28.f));
	BoxLid->SetRelativeScale3D(FVector(1.08f, 1.08f, 0.1f));

	HoloBeam = MakeMesh(TEXT("Beam"), Cyl);
	HoloBeam->SetupAttachment(BoxBody);
	HoloBeam->SetRelativeLocation(FVector(0.f, 0.f, 500.f));
	HoloBeam->SetRelativeScale3D(FVector(0.025f, 0.025f, 10.f));

	BoxLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BoxLight"));
	BoxLight->SetupAttachment(BoxBody);
	BoxLight->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	BoxLight->SetIntensity(6000.f);
	BoxLight->SetAttenuationRadius(600.f);
	BoxLight->CastShadows = false;
}

void AExoDeathBox::InitFromPlayer(const FString& PlayerName,
	const TArray<TPair<EWeaponType, EWeaponRarity>>& Weapons)
{
	DeadPlayerName = PlayerName;
	StoredWeapons = Weapons;
	BuildVisuals();
}

void AExoDeathBox::BuildVisuals()
{
	UMaterialInterface* EmissiveOpaque = FExoMaterialFactory::GetEmissiveOpaque();
	UMaterialInterface* EmissiveAdditive = FExoMaterialFactory::GetEmissiveAdditive();
	if (!EmissiveOpaque || !EmissiveAdditive) return;

	// Death box: dark body with red-orange glow
	FLinearColor DeathColor(1.f, 0.35f, 0.15f);

	// Body — solid glowing element (opaque, unlit)
	BodyMat = UMaterialInstanceDynamic::Create(EmissiveOpaque, this);
	BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(DeathColor.R * 1.5f, DeathColor.G * 1.5f, DeathColor.B * 1.5f));
	BoxBody->SetMaterial(0, BodyMat);

	// Lid — solid glowing element (opaque, unlit)
	UMaterialInstanceDynamic* LidMat = UMaterialInstanceDynamic::Create(EmissiveOpaque, this);
	LidMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(DeathColor.R * 0.8f, DeathColor.G * 0.8f, DeathColor.B * 0.8f));
	BoxLid->SetMaterial(0, LidMat);

	// Holo beam — pure energy overlay (additive, unlit)
	BeamMat = UMaterialInstanceDynamic::Create(EmissiveAdditive, this);
	BeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(DeathColor.R * 3.f, DeathColor.G * 3.f, DeathColor.B * 3.f));
	HoloBeam->SetMaterial(0, BeamMat);

	BoxLight->SetLightColor(DeathColor);
}

void AExoDeathBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;

	if (bLooted)
	{
		// Fade out after looting
		float FadeT = FMath::Clamp((Age - DespawnTime) / 5.f, 0.f, 1.f);
		float Alpha = 1.f - FadeT;
		BoxLight->SetIntensity(2000.f * Alpha);
		if (BeamMat)
		{
			BeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Alpha, Alpha * 0.35f, Alpha * 0.15f));
		}
		return;
	}

	// Pulsing glow
	float Pulse = 0.7f + 0.3f * FMath::Sin(Age * 3.f);
	BoxLight->SetIntensity(6000.f * Pulse);

	if (BodyMat)
	{
		float Em = 1.5f * Pulse;
		BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Em, Em * 0.35f, Em * 0.15f));
	}

	if (BeamMat)
	{
		float BeamPulse = 0.6f + 0.4f * FMath::Sin(Age * 2.f);
		BeamMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(3.f * BeamPulse, 1.05f * BeamPulse, 0.45f * BeamPulse));
	}

	// Slowly shrink beam over time
	float BeamFade = FMath::Clamp(1.f - Age / DespawnTime, 0.f, 1.f);
	HoloBeam->SetRelativeScale3D(FVector(0.025f * BeamFade, 0.025f * BeamFade, 10.f * BeamFade));
}

void AExoDeathBox::Interact(AExoCharacter* Interactor)
{
	if (bLooted || !Interactor || StoredWeapons.Num() == 0) return;
	bLooted = true;

	AExoPickupFlash::SpawnAt(GetWorld(), GetActorLocation() + FVector(0, 0, 30.f),
		FLinearColor(1.f, 0.35f, 0.15f));

	// Spawn weapon pickups around the box
	for (int32 i = 0; i < StoredWeapons.Num(); i++)
	{
		float Angle = (2.f * PI / StoredWeapons.Num()) * i;
		FVector Offset(FMath::Cos(Angle) * 100.f, FMath::Sin(Angle) * 100.f, 50.f);

		FActorSpawnParameters Params;
		AExoWeaponPickup* Pickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
			AExoWeaponPickup::StaticClass(), GetActorLocation() + Offset,
			FRotator::ZeroRotator, Params);
		if (Pickup)
		{
			Pickup->WeaponType = StoredWeapons[i].Key;
			Pickup->Rarity = StoredWeapons[i].Value;
			Pickup->bRespawns = false;
		}
	}

	// Dim the visuals
	HoloBeam->SetVisibility(false);
	BoxLight->SetIntensity(2000.f);
	DespawnTime = Age; // Start fade from now

	UE_LOG(LogExoRift, Log, TEXT("DeathBox of %s looted"), *DeadPlayerName);
}

FString AExoDeathBox::GetInteractionPrompt()
{
	if (bLooted) return FString();
	return FString::Printf(TEXT("[E] Loot %s's Box"), *DeadPlayerName);
}
