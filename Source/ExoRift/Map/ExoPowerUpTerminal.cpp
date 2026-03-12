// ExoPowerUpTerminal.cpp — Interactable buff terminal
#include "Map/ExoPowerUpTerminal.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Weapons/ExoWeaponBase.h"
#include "Visual/ExoPickupFlash.h"
#include "Visual/ExoPostProcess.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"
#include "GameFramework/CharacterMovementComponent.h"

AExoPowerUpTerminal::AExoPowerUpTerminal()
{
	PrimaryActorTick.bCanEverTick = true;

	// Try real imported computer mesh first
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ComputerFinder(
		TEXT("/Game/Meshes/Quaternius_SciFi/Props_Computer"));
	UStaticMesh* ComputerAsset = ComputerFinder.Succeeded() ? ComputerFinder.Object : nullptr;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	UStaticMesh* Cyl = CylFinder.Succeeded() ? CylFinder.Object : nullptr;

	auto MakePart = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) C->SetStaticMesh(Mesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		return C;
	};

	bHasRealMesh = (ComputerAsset != nullptr);

	// Base cylinder — always present as root + collision
	BaseMesh = MakePart(TEXT("Base"), Cyl);
	RootComponent = BaseMesh;
	BaseMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.03f));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BaseMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BaseMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	if (bHasRealMesh)
	{
		// Real asset — single mesh replaces pillar + screen primitives
		ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Computer"));
		ComputerMesh->SetStaticMesh(ComputerAsset);
		ComputerMesh->SetupAttachment(BaseMesh);
		ComputerMesh->SetRelativeLocation(FVector(0.f, 0.f, 5.f));
		ComputerMesh->SetRelativeScale3D(FVector(80.f, 80.f, 80.f));
		ComputerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ComputerMesh->CastShadow = true;

		// Hide base disc — the real mesh has its own base
		BaseMesh->SetVisibility(false);

		// Create hidden fallback parts (UE CDO requires stable subobject set)
		PillarMesh = MakePart(TEXT("Pillar"), nullptr);
		PillarMesh->SetupAttachment(BaseMesh);
		PillarMesh->SetVisibility(false);
		ScreenMesh = MakePart(TEXT("Screen"), nullptr);
		ScreenMesh->SetupAttachment(BaseMesh);
		ScreenMesh->SetVisibility(false);

		// Light positioned above the real computer mesh
		TerminalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
		TerminalLight->SetupAttachment(ComputerMesh);
		TerminalLight->SetRelativeLocation(FVector(0.f, 0.f, 1.2f));
		TerminalLight->SetIntensity(12000.f);
		TerminalLight->SetAttenuationRadius(800.f);
		TerminalLight->CastShadows = false;
	}
	else
	{
		// Fallback: primitive shapes
		ComputerMesh = nullptr;

		PillarMesh = MakePart(TEXT("Pillar"), Cube);
		PillarMesh->SetupAttachment(BaseMesh);
		PillarMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
		PillarMesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 3.f));

		ScreenMesh = MakePart(TEXT("Screen"), Cube);
		ScreenMesh->SetupAttachment(PillarMesh);
		ScreenMesh->SetRelativeLocation(FVector(12.f, 0.f, 18.f));
		ScreenMesh->SetRelativeScale3D(FVector(0.02f, 2.5f, 1.5f));
		ScreenMesh->SetRelativeRotation(FRotator(15.f, 0.f, 0.f));

		TerminalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
		TerminalLight->SetupAttachment(ScreenMesh);
		TerminalLight->SetRelativeLocation(FVector(10.f, 0.f, 0.f));
		TerminalLight->SetIntensity(12000.f);
		TerminalLight->SetAttenuationRadius(800.f);
		TerminalLight->CastShadows = false;
	}

	TypeColor = FLinearColor(0.2f, 0.8f, 1.f);
}

void AExoPowerUpTerminal::InitTerminal(EPowerUpType Type)
{
	PowerUp = Type;
	switch (Type)
	{
	case EPowerUpType::SpeedBoost:     TypeColor = FLinearColor(0.2f, 0.9f, 1.f); break;
	case EPowerUpType::DamageBoost:    TypeColor = FLinearColor(1.f, 0.3f, 0.1f); break;
	case EPowerUpType::ShieldRecharge: TypeColor = FLinearColor(0.2f, 0.5f, 1.f); break;
	case EPowerUpType::OverheatReset:  TypeColor = FLinearColor(0.3f, 1.f, 0.4f); break;
	}
	BuildVisuals();
}

void AExoPowerUpTerminal::BuildVisuals()
{
	// Real mesh: preserve imported materials, only set light color
	if (bHasRealMesh)
	{
		TerminalLight->SetLightColor(TypeColor);
		return;
	}

	// Fallback: apply dynamic PBR materials to primitive shapes
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (!LitMat) return;

	auto ApplyStructMat = [&](UStaticMeshComponent* C, const FLinearColor& Color)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!M) { return; }
		M->SetVectorParameterValue(TEXT("BaseColor"), Color);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		M->SetScalarParameterValue(TEXT("Metallic"), 0.88f);
		M->SetScalarParameterValue(TEXT("Roughness"), 0.25f);
		C->SetMaterial(0, M);
	};

	ApplyStructMat(BaseMesh, FLinearColor(0.05f, 0.05f, 0.07f));
	ApplyStructMat(PillarMesh, FLinearColor(0.06f, 0.06f, 0.08f));

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	ScreenMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	if (!ScreenMat) { return; }
	FLinearColor ScreenColor = TypeColor * 0.3f;
	ScreenMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(ScreenColor.R * 10.f, ScreenColor.G * 10.f, ScreenColor.B * 10.f));
	ScreenMesh->SetMaterial(0, ScreenMat);
	TerminalLight->SetLightColor(TypeColor);
}

void AExoPowerUpTerminal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUsed)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f)
		{
			bUsed = false;
			if (bHasRealMesh && ComputerMesh)
				ComputerMesh->SetVisibility(true);
			else
				ScreenMesh->SetVisibility(true);
			TerminalLight->SetIntensity(12000.f);
		}
		return;
	}

	// Pulse glow
	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 3.f);
	if (!bHasRealMesh && ScreenMat)
	{
		ScreenMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(TypeColor.R * 3.f * Pulse, TypeColor.G * 3.f * Pulse,
				TypeColor.B * 3.f * Pulse));
	}
	TerminalLight->SetIntensity(4000.f * Pulse);
}

void AExoPowerUpTerminal::Interact(AExoCharacter* Interactor)
{
	if (bUsed || !Interactor) return;
	bUsed = true;
	RespawnTimer = RespawnTime;

	ApplyPowerUp(Interactor);

	AExoPickupFlash::SpawnAt(GetWorld(), GetActorLocation() + FVector(0, 0, 80.f), TypeColor);
	if (bHasRealMesh && ComputerMesh)
		ComputerMesh->SetVisibility(false);
	else
		ScreenMesh->SetVisibility(false);
	TerminalLight->SetIntensity(1000.f);
}

void AExoPowerUpTerminal::ApplyPowerUp(AExoCharacter* Target)
{
	switch (PowerUp)
	{
	case EPowerUpType::SpeedBoost:
		if (Target->GetCharacterMovement())
		{
			float Base = Target->GetCharacterMovement()->MaxWalkSpeed;
			Target->GetCharacterMovement()->MaxWalkSpeed = Base * 1.3f;
			// Speed boost post-process
			AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
			if (PP) PP->ApplySpeedBoostEffect(0.5f);
		}
		break;

	case EPowerUpType::DamageBoost:
		// Triggers via post-process visual feedback
		{
			AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
			if (PP) PP->TriggerShieldFlash();
		}
		break;

	case EPowerUpType::ShieldRecharge:
		if (Target->GetShieldComponent())
			Target->GetShieldComponent()->AddShield(50.f);
		{
			AExoPostProcess* PP = AExoPostProcess::Get(GetWorld());
			if (PP) PP->TriggerShieldFlash();
		}
		break;

	case EPowerUpType::OverheatReset:
		if (Target->GetCurrentWeapon())
		{
			Target->GetCurrentWeapon()->ResetHeat();
		}
		break;
	}
}

FString AExoPowerUpTerminal::GetInteractionPrompt()
{
	if (bUsed) return FString();
	return FString::Printf(TEXT("[E] %s"), *GetPowerUpName());
}

FString AExoPowerUpTerminal::GetPowerUpName() const
{
	switch (PowerUp)
	{
	case EPowerUpType::SpeedBoost:     return TEXT("Speed Boost");
	case EPowerUpType::DamageBoost:    return TEXT("Damage Boost");
	case EPowerUpType::ShieldRecharge: return TEXT("Shield Recharge");
	case EPowerUpType::OverheatReset:  return TEXT("Coolant Flush");
	default:                           return TEXT("Power-Up");
	}
}
