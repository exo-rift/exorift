#include "Weapons/ExoProximityMine.h"
#include "Player/ExoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoTracerManager.h"
#include "Visual/ExoScreenShake.h"
#include "Core/ExoAudioManager.h"
#include "Engine/DamageEvents.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

AExoProximityMine::AExoProximityMine()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFinder(
		TEXT("/Engine/BasicShapes/Sphere"));

	// Main body — squat cylinder
	MineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MineMesh"));
	MineMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = MineMesh;
	if (CylFinder.Succeeded())
		MineMesh->SetStaticMesh(CylFinder.Object);
	MineMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.06f));

	// Antenna — small vertical cylinder
	if (CylFinder.Succeeded())
	{
		Antenna = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Antenna"));
		Antenna->SetupAttachment(MineMesh);
		Antenna->SetStaticMesh(CylFinder.Object);
		Antenna->SetRelativeLocation(FVector(0.f, 0.f, 15.f));
		Antenna->SetRelativeScale3D(FVector(0.04f, 0.04f, 0.5f));
		Antenna->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Antenna->CastShadow = false;
	}

	// LED indicator sphere on top of antenna
	if (SphFinder.Succeeded())
	{
		LEDMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LEDMesh"));
		LEDMesh->SetupAttachment(MineMesh);
		LEDMesh->SetStaticMesh(SphFinder.Object);
		LEDMesh->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
		LEDMesh->SetRelativeScale3D(FVector(0.06f));
		LEDMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		LEDMesh->CastShadow = false;
	}

	// Status light
	StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
	StatusLight->SetupAttachment(MineMesh);
	StatusLight->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
	StatusLight->SetIntensity(500.f);
	StatusLight->SetAttenuationRadius(200.f);
	StatusLight->SetLightColor(FLinearColor(1.f, 0.8f, 0.f));
	StatusLight->CastShadows = false;

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
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(
		this, &AExoProximityMine::OnTriggerBeginOverlap);

	BuildMineVisuals();
}

void AExoProximityMine::BuildMineVisuals()
{
	UMaterialInterface* BaseMat = MineMesh ? MineMesh->GetMaterial(0) : nullptr;
	if (!BaseMat) return;

	// Body — dark metallic
	BodyDynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	BodyDynMat->SetVectorParameterValue(TEXT("BaseColor"),
		FLinearColor(0.06f, 0.06f, 0.08f));
	MineMesh->SetMaterial(0, BodyDynMat);

	// Antenna — silver
	if (Antenna)
	{
		UMaterialInstanceDynamic* AntMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		AntMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.15f, 0.15f, 0.17f));
		Antenna->SetMaterial(0, AntMat);
	}

	// LED — starts yellow (deploying)
	if (LEDMesh)
	{
		LEDDynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
		LEDDynMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(1.f, 0.7f, 0.f));
		LEDDynMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(3.f, 2.f, 0.f));
		LEDMesh->SetMaterial(0, LEDDynMat);
	}
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

	UpdateMineVFX(DeltaTime);
}

void AExoProximityMine::UpdateMineVFX(float DeltaTime)
{
	float Time = GetWorld()->GetTimeSeconds();

	switch (MineState)
	{
	case EProximityMineState::Deploying:
	{
		// Yellow pulsing — deploying
		float Deploy = FMath::Clamp(StateTimer / ArmDelay, 0.f, 1.f);
		float Glow = 0.5f + 0.5f * FMath::Sin(Time * 4.f);
		if (StatusLight) StatusLight->SetIntensity(500.f * Glow);
		if (LEDDynMat)
		{
			LEDDynMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(3.f * Glow, 2.f * Glow, 0.f));
		}
		break;
	}
	case EProximityMineState::Armed:
	{
		// Green slow pulse — armed and watching
		float Pulse = bPulseOn ? 1.f : 0.2f;
		if (StatusLight)
		{
			StatusLight->SetLightColor(FLinearColor(0.1f, 1.f, 0.2f));
			StatusLight->SetIntensity(800.f * Pulse);
		}
		if (LEDDynMat)
		{
			LEDDynMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.1f, 0.8f, 0.1f));
			LEDDynMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.5f * Pulse, 3.f * Pulse, 0.3f * Pulse));
		}
		break;
	}
	case EProximityMineState::Triggered:
	{
		// Rapid red flash — about to explode
		float Flash = FMath::Abs(FMath::Sin(Time * 20.f));
		if (StatusLight)
		{
			StatusLight->SetLightColor(FLinearColor(1.f, 0.1f, 0.05f));
			StatusLight->SetIntensity(5000.f * Flash);
			StatusLight->SetAttenuationRadius(400.f);
		}
		if (LEDDynMat)
		{
			LEDDynMat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(1.f, 0.1f, 0.05f));
			LEDDynMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(10.f * Flash, 1.f * Flash, 0.f));
		}
		// Body glows red
		if (BodyDynMat)
		{
			float BodyGlow = Flash * 0.3f;
			BodyDynMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(BodyGlow, 0.f, 0.f));
		}
		break;
	}
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

	// Explosion VFX and audio
	FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), ExplosionRadius);
	FExoScreenShake::AddExplosionShake(GetActorLocation(), GetActorLocation(), 3000.f, 0.4f);
	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
		Audio->PlayExplosionSound(GetActorLocation());

	UE_LOG(LogExoRift, Log, TEXT("ProximityMine %s: detonated"), *GetName());
	Destroy();
}

void AExoProximityMine::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (MineState != EProximityMineState::Armed) return;
	AExoCharacter* Character = Cast<AExoCharacter>(OtherActor);
	if (!Character || !Character->IsAlive()) return;
	if (Character == OwnerCharacter) return;
	TriggerMine();
}
