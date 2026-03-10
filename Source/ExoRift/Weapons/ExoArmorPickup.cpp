#include "Weapons/ExoArmorPickup.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoArmorComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoPickupFlash.h"
#include "ExoRift.h"

AExoArmorPickup::AExoArmorPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(120.f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = CollisionSphere;

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(CollisionSphere);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetRelativeScale3D(FVector(0.4f));

	// Glow light for visibility at distance
	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(CollisionSphere);
	GlowLight->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
	GlowLight->SetIntensity(3000.f);
	GlowLight->SetAttenuationRadius(400.f);
	GlowLight->CastShadows = false;

	BobPhase = FMath::RandRange(0.f, 2.f * PI);
}

void AExoArmorPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();

	// Set glow light color and intensity based on tier
	if (GlowLight)
	{
		FLinearColor Col = GetTierColor();
		GlowLight->SetLightColor(Col);
		float TierIntensity = (Tier == EArmorTier::Heavy) ? 6000.f :
			(Tier == EArmorTier::Medium) ? 4000.f : 2000.f;
		GlowLight->SetIntensity(TierIntensity);
		GlowLight->SetAttenuationRadius(
			(Tier == EArmorTier::Heavy) ? 500.f : 350.f);
	}
}

void AExoArmorPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		// Floating bob + rotation
		BobPhase += DeltaTime * 2.f;
		float Bob = FMath::Sin(BobPhase) * 18.f;
		SetActorLocation(BaseLocation + FVector(0.f, 0.f, Bob));

		FRotator Rot = GetActorRotation();
		Rot.Yaw += DeltaTime * 50.f;
		SetActorRotation(Rot);

		// Tint display mesh with tier color + pulsing emissive
		FLinearColor Col = GetTierColor();
		float Pulse = 1.5f + 0.5f * FMath::Sin(BobPhase * 1.5f);
		if (DisplayMesh)
		{
			UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(
				DisplayMesh->GetMaterial(0));
			if (!DynMat)
			{
				// Create MID from LitEmissive so EmissiveColor actually works
				UMaterialInterface* LitEmissive = FExoMaterialFactory::GetLitEmissive();
				if (LitEmissive)
				{
					DynMat = UMaterialInstanceDynamic::Create(LitEmissive, this);
					DisplayMesh->SetMaterial(0, DynMat);
				}
			}
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("BaseColor"), Col);
				DynMat->SetVectorParameterValue(TEXT("EmissiveColor"), Col * Pulse);
			}
		}

		// Pulse glow light
		if (GlowLight)
		{
			float BaseInt = (Tier == EArmorTier::Heavy) ? 6000.f :
				(Tier == EArmorTier::Medium) ? 4000.f : 2000.f;
			GlowLight->SetIntensity(BaseInt * (0.7f + 0.3f * FMath::Sin(BobPhase * 1.5f)));
		}
	}
	else if (bRespawns)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f) SetPickupActive(true);
	}
}

// ---------------------------------------------------------------------------
// IExoInteractable
// ---------------------------------------------------------------------------

void AExoArmorPickup::Interact(AExoCharacter* Interactor)
{
	if (!bIsActive || !Interactor || !Interactor->IsAlive()) return;

	// Collection flash VFX
	AExoPickupFlash::SpawnAt(GetWorld(), GetActorLocation(), GetTierColor());

	UExoArmorComponent* Armor = Interactor->GetArmorComponent();
	if (!Armor) return;

	if (bIsHelmet)
	{
		Armor->EquipHelmet(Tier);
	}
	else
	{
		Armor->EquipVest(Tier);
	}

	UE_LOG(LogExoRift, Log, TEXT("%s picked up %s"), *Interactor->GetName(), *GetDisplayName());

	SetPickupActive(false);
	if (bRespawns)
	{
		RespawnTimer = RespawnTime;
	}
	else
	{
		SetLifeSpan(0.1f);
	}
}

FString AExoArmorPickup::GetInteractionPrompt()
{
	return FString::Printf(TEXT("[E] %s"), *GetDisplayName());
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

FLinearColor AExoArmorPickup::GetTierColor() const
{
	switch (Tier)
	{
	case EArmorTier::Light:  return FLinearColor(0.6f, 0.6f, 0.65f, 1.f); // Grey
	case EArmorTier::Medium: return FLinearColor(0.2f, 0.5f, 1.0f, 1.f);  // Blue
	case EArmorTier::Heavy:  return FLinearColor(1.0f, 0.85f, 0.2f, 1.f); // Gold
	default:                 return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	}
}

FString AExoArmorPickup::GetDisplayName() const
{
	FString TierName;
	switch (Tier)
	{
	case EArmorTier::Light:  TierName = TEXT("Light"); break;
	case EArmorTier::Medium: TierName = TEXT("Medium"); break;
	case EArmorTier::Heavy:  TierName = TEXT("Heavy"); break;
	default:                 TierName = TEXT("Unknown"); break;
	}
	return FString::Printf(TEXT("%s %s"), *TierName, bIsHelmet ? TEXT("Helmet") : TEXT("Vest"));
}

void AExoArmorPickup::SetPickupActive(bool bActive)
{
	bIsActive = bActive;
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
}
