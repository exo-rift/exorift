#include "Map/ExoWaterPlane.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "ExoRift.h"

AExoWaterPlane::AExoWaterPlane()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f;

	WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
	RootComponent = WaterMesh;
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		WaterMesh->SetStaticMesh(PlaneMesh.Object);
	}

	WaterVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterVolume"));
	WaterVolume->SetupAttachment(WaterMesh);
	WaterVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WaterVolume->SetCollisionResponseToAllChannels(ECR_Overlap);
	WaterVolume->SetGenerateOverlapEvents(true);
}

void AExoWaterPlane::BeginPlay()
{
	Super::BeginPlay();

	// Scale the plane to cover the map
	float PlaneScale = WaterExtent / 50.f;
	WaterMesh->SetWorldScale3D(FVector(PlaneScale, PlaneScale, 1.f));
	SetActorLocation(FVector(0.f, 0.f, WaterLevel));

	// Volume extends deep below water surface
	WaterVolume->SetBoxExtent(FVector(WaterExtent, WaterExtent, 5000.f));
	WaterVolume->SetRelativeLocation(FVector(0.f, 0.f, -5000.f));

	UE_LOG(LogExoRift, Log, TEXT("WaterPlane: extent=%.0f, level=%.0f"), WaterExtent, WaterLevel);
}

void AExoWaterPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCausesDamage && HasAuthority())
	{
		ApplyDrowningDamage(DeltaTime);
	}
}

void AExoWaterPlane::ApplyDrowningDamage(float DeltaTime)
{
	TArray<AActor*> Overlapping;
	WaterVolume->GetOverlappingActors(Overlapping, ACharacter::StaticClass());

	for (AActor* Actor : Overlapping)
	{
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character) continue;

		// Only damage if head is below water
		float HeadZ = Character->GetActorLocation().Z + 80.f;
		if (HeadZ < WaterLevel)
		{
			FDamageEvent DmgEvent;
			float TickDamage = DrowningDamage * PrimaryActorTick.TickInterval;
			Character->TakeDamage(TickDamage, DmgEvent, nullptr, this);
		}
	}
}
