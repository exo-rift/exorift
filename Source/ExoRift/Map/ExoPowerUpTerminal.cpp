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

	// Base cylinder
	BaseMesh = MakePart(TEXT("Base"), Cyl);
	RootComponent = BaseMesh;
	BaseMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.03f));
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BaseMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BaseMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Vertical pillar
	PillarMesh = MakePart(TEXT("Pillar"), Cube);
	PillarMesh->SetupAttachment(BaseMesh);
	PillarMesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	PillarMesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 3.f));

	// Angled screen
	ScreenMesh = MakePart(TEXT("Screen"), Cube);
	ScreenMesh->SetupAttachment(PillarMesh);
	ScreenMesh->SetRelativeLocation(FVector(12.f, 0.f, 18.f));
	ScreenMesh->SetRelativeScale3D(FVector(0.02f, 2.5f, 1.5f));
	ScreenMesh->SetRelativeRotation(FRotator(15.f, 0.f, 0.f));

	TerminalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	TerminalLight->SetupAttachment(ScreenMesh);
	TerminalLight->SetRelativeLocation(FVector(10.f, 0.f, 0.f));
	TerminalLight->SetIntensity(5000.f);
	TerminalLight->SetAttenuationRadius(500.f);
	TerminalLight->CastShadows = false;

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
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!BaseMat) return;

	auto ApplyStructMat = [&](UStaticMeshComponent* C, const FLinearColor& Color)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(BaseMat, this);
		M->SetVectorParameterValue(TEXT("BaseColor"), Color);
		C->SetMaterial(0, M);
	};

	ApplyStructMat(BaseMesh, FLinearColor(0.05f, 0.05f, 0.07f));
	ApplyStructMat(PillarMesh, FLinearColor(0.06f, 0.06f, 0.08f));

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	ScreenMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
	FLinearColor ScreenColor = TypeColor * 0.3f;
	ScreenMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(ScreenColor.R * 4.f, ScreenColor.G * 4.f, ScreenColor.B * 4.f));
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
			ScreenMesh->SetVisibility(true);
			TerminalLight->SetIntensity(5000.f);
		}
		return;
	}

	// Pulse screen glow
	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = 0.7f + 0.3f * FMath::Sin(Time * 3.f);
	if (ScreenMat)
	{
		ScreenMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(TypeColor.R * 4.f * Pulse, TypeColor.G * 4.f * Pulse,
				TypeColor.B * 4.f * Pulse));
	}
	TerminalLight->SetIntensity(5000.f * Pulse);
}

void AExoPowerUpTerminal::Interact(AExoCharacter* Interactor)
{
	if (bUsed || !Interactor) return;
	bUsed = true;
	RespawnTimer = RespawnTime;

	ApplyPowerUp(Interactor);

	AExoPickupFlash::SpawnAt(GetWorld(), GetActorLocation() + FVector(0, 0, 80.f), TypeColor);
	ScreenMesh->SetVisibility(false);
	TerminalLight->SetIntensity(500.f);
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
