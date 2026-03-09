#include "Weapons/ExoProximityMine.h"
#include "Player/ExoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

AExoProximityMine::AExoProximityMine()
{
	PrimaryActorTick.bCanEverTick = true;

	MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
	MineMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = MineMesh;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->InitSphereRadius(200.f);
	TriggerSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->SetupAttachment(MineMesh);
	TriggerSphere->SetActive(false);
}

void AExoProximityMine::BeginPlay()
{
	Super::BeginPlay();
	MineState = EProximityMineState::Deploying;
	StateTimer = 0.f;
	LifetimeTimer = 0.f;
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AExoProximityMine::OnTriggerBeginOverlap);
}

void AExoProximityMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LifetimeTimer += DeltaTime;
	if (LifetimeTimer >= MaxLifetime && MineState != EProximityMineState::Detonated)
	{
		Destroy();
		return;
	}

	StateTimer += DeltaTime;

	switch (MineState)
	{
	case EProximityMineState::Deploying:
		if (StateTimer >= ArmDelay) Arm();
		break;
	case EProximityMineState::Armed:
		PulseTimer += DeltaTime;
		if (PulseTimer >= 0.5f) { bPulseOn = !bPulseOn; PulseTimer = 0.f; }
		break;
	case EProximityMineState::Triggered:
		if (StateTimer >= TriggerDelay) Detonate();
		break;
	default: break;
	}
}

void AExoProximityMine::Arm()
{
	MineState = EProximityMineState::Armed;
	StateTimer = 0.f;
	TriggerSphere->SetActive(true);
	UE_LOG(LogExoRift, Log, TEXT("ProximityMine %s: armed"), *GetName());
}

void AExoProximityMine::TriggerMine()
{
	if (MineState != EProximityMineState::Armed) return;
	MineState = EProximityMineState::Triggered;
	StateTimer = 0.f;
	UE_LOG(LogExoRift, Log, TEXT("ProximityMine %s: triggered"), *GetName());
}

void AExoProximityMine::Detonate()
{
	MineState = EProximityMineState::Detonated;
	AController* InstigatorCtrl = OwnerCharacter ? OwnerCharacter->GetController() : nullptr;
	UGameplayStatics::ApplyRadialDamage(
		this, ExplosionDamage, GetActorLocation(), ExplosionRadius,
		UDamageType::StaticClass(), TArray<AActor*>(), this, InstigatorCtrl, true);
	UE_LOG(LogExoRift, Log, TEXT("ProximityMine %s: detonated"), *GetName());
	Destroy();
}

void AExoProximityMine::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (MineState != EProximityMineState::Armed) return;
	AExoCharacter* Character = Cast<AExoCharacter>(OtherActor);
	if (!Character || !Character->IsAlive()) return;
	if (Character == OwnerCharacter) return;
	TriggerMine();
}
