#include "Player/ExoDecoyActor.h"
#include "Components/StaticMeshComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "ExoRift.h"

AExoDecoyActor::AExoDecoyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Visual representation -- uses a capsule placeholder mesh
	DecoyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DecoyMesh"));
	RootComponent = DecoyMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CapsuleFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_ChamferCube"));
	if (CapsuleFinder.Succeeded())
	{
		DecoyMesh->SetStaticMesh(CapsuleFinder.Object);
	}

	// Scale to roughly human proportions (standing target)
	DecoyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.8f));
	DecoyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DecoyMesh->CastShadow = true;

	// AI perception stimuli: makes bots detect this actor via sight
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(
		TEXT("StimuliSource"));
	StimuliSource->bAutoRegister = true;
	StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
}

void AExoDecoyActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogExoRift, Log, TEXT("Decoy actor spawned at %s"), *GetActorLocation().ToString());
}
