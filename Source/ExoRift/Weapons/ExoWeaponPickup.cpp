#include "Weapons/ExoWeaponPickup.h"
#include "Weapons/ExoWeaponRifle.h"
#include "Weapons/ExoWeaponPistol.h"
#include "Weapons/ExoWeaponGrenadeLauncher.h"
#include "Player/ExoCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ExoRift.h"

AExoWeaponPickup::AExoWeaponPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(150.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = CollisionSphere;

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(CollisionSphere);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetRelativeScale3D(FVector(0.5f));

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AExoWeaponPickup::OnOverlapBegin);

	// Random bob phase so pickups don't all bob in sync
	BobPhase = FMath::RandRange(0.f, 2.f * PI);
}

void AExoWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	BaseLocation = GetActorLocation();
}

void AExoWeaponPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		// Floating bob + rotation
		BobPhase += DeltaTime * 2.f;
		float Bob = FMath::Sin(BobPhase) * 20.f;
		SetActorLocation(BaseLocation + FVector(0.f, 0.f, Bob));

		FRotator Rot = GetActorRotation();
		Rot.Yaw += DeltaTime * 45.f;
		SetActorRotation(Rot);
	}
	else if (bRespawns)
	{
		RespawnTimer -= DeltaTime;
		if (RespawnTimer <= 0.f)
		{
			SetPickupActive(true);
		}
	}
}

void AExoWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsActive) return;

	AExoCharacter* Character = Cast<AExoCharacter>(OtherActor);
	if (Character && Character->IsAlive())
	{
		SpawnWeaponForPlayer(Character);
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
}

void AExoWeaponPickup::SpawnWeaponForPlayer(AExoCharacter* Character)
{
	if (!Character) return;

	TSubclassOf<AExoWeaponBase> WeaponClass;
	switch (WeaponType)
	{
	case EWeaponType::Rifle: WeaponClass = AExoWeaponRifle::StaticClass(); break;
	case EWeaponType::Pistol: WeaponClass = AExoWeaponPistol::StaticClass(); break;
	case EWeaponType::GrenadeLauncher: WeaponClass = AExoWeaponGrenadeLauncher::StaticClass(); break;
	}

	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;

		AExoWeaponBase* Weapon = GetWorld()->SpawnActor<AExoWeaponBase>(
			WeaponClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		if (Weapon)
		{
			Character->EquipWeapon(Weapon);
			UE_LOG(LogExoRift, Log, TEXT("%s picked up %s"),
				*Character->GetName(), *Weapon->GetWeaponName());
		}
	}
}

void AExoWeaponPickup::SetPickupActive(bool bActive)
{
	bIsActive = bActive;
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);
}
