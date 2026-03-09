#include "Weapons/ExoTrapPickup.h"
#include "Weapons/ExoTrapComponent.h"
#include "Player/ExoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ExoRift.h"

AExoTrapPickup::AExoTrapPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(100.f);
	CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = CollisionSphere;

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(CollisionSphere);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetRelativeScale3D(FVector(0.4f));
}

void AExoTrapPickup::Interact(AExoCharacter* Interactor)
{
	if (!Interactor || !Interactor->IsAlive()) return;

	UExoTrapComponent* TrapComp = Interactor->GetTrapComponent();
	if (!TrapComp) return;

	TrapComp->AddTraps(TrapCount);
	UE_LOG(LogExoRift, Log, TEXT("%s picked up %d mine(s)"), *Interactor->GetName(), TrapCount);
	Destroy();
}

FString AExoTrapPickup::GetInteractionPrompt()
{
	return FString::Printf(TEXT("Pick up Mine x%d"), TrapCount);
}
