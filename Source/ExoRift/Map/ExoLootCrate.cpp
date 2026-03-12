// ExoLootCrate.cpp — Interactable loot container with opening animation
#include "Map/ExoLootCrate.h"
#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoCharacter.h"
#include "Visual/ExoPickupFlash.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

AExoLootCrate::AExoLootCrate()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;

	auto MakePart = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Cube) C->SetStaticMesh(Cube);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		return C;
	};

	CrateBody = MakePart(TEXT("Body"));
	RootComponent = CrateBody;
	CrateBody->SetRelativeScale3D(FVector(0.6f, 0.4f, 0.35f));
	CrateBody->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CrateBody->SetCollisionResponseToAllChannels(ECR_Ignore);
	CrateBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	CrateLid = MakePart(TEXT("Lid"));
	CrateLid->SetupAttachment(CrateBody);
	CrateLid->SetRelativeLocation(FVector(0.f, 0.f, 38.f));
	CrateLid->SetRelativeScale3D(FVector(1.05f, 1.05f, 0.08f));

	GlowStrip1 = MakePart(TEXT("Strip1"));
	GlowStrip1->SetupAttachment(CrateBody);
	GlowStrip1->SetRelativeLocation(FVector(31.f, 0.f, 0.f));
	GlowStrip1->SetRelativeScale3D(FVector(0.02f, 0.9f, 0.7f));

	GlowStrip2 = MakePart(TEXT("Strip2"));
	GlowStrip2->SetupAttachment(CrateBody);
	GlowStrip2->SetRelativeLocation(FVector(-31.f, 0.f, 0.f));
	GlowStrip2->SetRelativeScale3D(FVector(0.02f, 0.9f, 0.7f));

	CrateLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CrateLight"));
	CrateLight->SetupAttachment(CrateBody);
	CrateLight->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	CrateLight->SetIntensity(9000.f);
	CrateLight->SetAttenuationRadius(800.f);
	CrateLight->CastShadows = false;

	BobPhase = FMath::RandRange(0.f, 6.28f);
}

void AExoLootCrate::BeginPlay()
{
	Super::BeginPlay();
	BuildVisuals();
}

void AExoLootCrate::BuildVisuals()
{
	// Random crate accent color — cyan, amber, or magenta
	int32 Roll = FMath::RandRange(0, 2);
	switch (Roll)
	{
	case 0: CrateColor = FLinearColor(0.2f, 0.8f, 1.f); break;   // Cyan
	case 1: CrateColor = FLinearColor(1.f, 0.7f, 0.1f); break;   // Amber
	default: CrateColor = FLinearColor(0.9f, 0.2f, 0.8f); break;  // Magenta
	}

	// PBR metallic crate body
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat)
	{
		UMaterialInstanceDynamic* BodyMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!BodyMat) { return; }
		BodyMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.06f, 0.06f, 0.08f));
		BodyMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		BodyMat->SetScalarParameterValue(TEXT("Metallic"), 0.88f);
		BodyMat->SetScalarParameterValue(TEXT("Roughness"), 0.25f);
		CrateBody->SetMaterial(0, BodyMat);

		// Lid — slightly lighter
		LidMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!LidMat) { return; }
		LidMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.08f, 0.08f, 0.1f));
		LidMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		LidMat->SetScalarParameterValue(TEXT("Metallic"), 0.85f);
		LidMat->SetScalarParameterValue(TEXT("Roughness"), 0.28f);
		CrateLid->SetMaterial(0, LidMat);
	}

	// Glowing accent strips
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	FLinearColor StripEm(CrateColor.R * 12.f, CrateColor.G * 12.f, CrateColor.B * 12.f);
	StripMat1 = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	if (!StripMat1) { return; }
	StripMat1->SetVectorParameterValue(TEXT("EmissiveColor"), StripEm);
	GlowStrip1->SetMaterial(0, StripMat1);

	StripMat2 = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	if (!StripMat2) { return; }
	StripMat2->SetVectorParameterValue(TEXT("EmissiveColor"), StripEm);
	GlowStrip2->SetMaterial(0, StripMat2);

	CrateLight->SetLightColor(CrateColor);
}

void AExoLootCrate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bOpened)
	{
		// Pulse glow strips
		BobPhase += DeltaTime * 3.f;
		float Pulse = 0.6f + 0.4f * FMath::Sin(BobPhase);
		FLinearColor Em(CrateColor.R * 3.f * Pulse, CrateColor.G * 3.f * Pulse,
			CrateColor.B * 3.f * Pulse);
		if (StripMat1) StripMat1->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		if (StripMat2) StripMat2->SetVectorParameterValue(TEXT("EmissiveColor"), Em);
		CrateLight->SetIntensity(3000.f * Pulse);
	}
	else
	{
		// Opening animation — lid rises and fades
		OpenTimer += DeltaTime;
		float T = FMath::Clamp(OpenTimer / 0.8f, 0.f, 1.f);
		float LidRise = T * 80.f;
		CrateLid->SetRelativeLocation(FVector(0.f, 0.f, 38.f + LidRise));

		// Fade light and strips
		float Fade = 1.f - T;
		CrateLight->SetIntensity(18000.f * Fade);
		if (StripMat1)
		{
			FLinearColor FadeEm(CrateColor.R * 12.f * Fade, CrateColor.G * 12.f * Fade,
				CrateColor.B * 12.f * Fade);
			StripMat1->SetVectorParameterValue(TEXT("EmissiveColor"), FadeEm);
			StripMat2->SetVectorParameterValue(TEXT("EmissiveColor"), FadeEm);
		}

		if (OpenTimer > 1.5f)
		{
			SetActorTickEnabled(false);
			CrateLight->SetIntensity(0.f);
		}
	}
}

void AExoLootCrate::Interact(AExoCharacter* Interactor)
{
	if (bOpened || !Interactor) return;
	bOpened = true;
	OpenTimer = 0.f;

	// Bright flash on open
	AExoPickupFlash::SpawnAt(GetWorld(), GetActorLocation() + FVector(0, 0, 50.f), CrateColor);

	// Burst of light
	CrateLight->SetIntensity(18000.f);

	SpawnContents();
	UE_LOG(LogExoRift, Log, TEXT("LootCrate opened by %s"), *Interactor->GetName());
}

FString AExoLootCrate::GetInteractionPrompt()
{
	if (bOpened) return FString();
	return TEXT("[E] Open Crate");
}

void AExoLootCrate::SpawnContents()
{
	const FVector Base = GetActorLocation() + FVector(0, 0, 80.f);

	for (int32 i = 0; i < ItemCount; i++)
	{
		// Fan out items around the crate
		float Angle = (2.f * PI / ItemCount) * i;
		FVector Offset(FMath::Cos(Angle) * 120.f, FMath::Sin(Angle) * 120.f, 0.f);

		// Random weapon type and rarity
		EWeaponType Type;
		int32 TypeRoll = FMath::RandRange(0, 5);
		switch (TypeRoll)
		{
		case 0: Type = EWeaponType::Rifle; break;
		case 1: Type = EWeaponType::Pistol; break;
		case 2: Type = EWeaponType::Shotgun; break;
		case 3: Type = EWeaponType::SMG; break;
		case 4: Type = EWeaponType::Sniper; break;
		case 5: Type = EWeaponType::GrenadeLauncher; break;
		default: Type = EWeaponType::Rifle; break;
		}

		// Crate loot is slightly better than ground loot
		EWeaponRarity Rarity;
		float RarityRoll = FMath::FRand();
		if (RarityRoll < 0.3f) Rarity = EWeaponRarity::Common;
		else if (RarityRoll < 0.6f) Rarity = EWeaponRarity::Rare;
		else if (RarityRoll < 0.85f) Rarity = EWeaponRarity::Epic;
		else Rarity = EWeaponRarity::Legendary;

		FActorSpawnParameters Params;
		AExoWeaponPickup* Pickup = GetWorld()->SpawnActor<AExoWeaponPickup>(
			AExoWeaponPickup::StaticClass(), Base + Offset, FRotator::ZeroRotator, Params);
		if (Pickup)
		{
			Pickup->WeaponType = Type;
			Pickup->Rarity = Rarity;
			Pickup->bRespawns = false;
		}
	}
}
