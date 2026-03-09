#include "Player/ExoAbilityComponent.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoShieldComponent.h"
#include "Player/ExoDecoyActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ExoRift.h"

UExoAbilityComponent::UExoAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UExoAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize default abilities
	FExoAbility Dash;
	Dash.Type = EExoAbilityType::Dash;
	Dash.Cooldown = 8.f;
	Dash.CooldownRemaining = 0.f;
	Dash.AbilityName = TEXT("Dash");
	Abilities.Add(Dash);

	FExoAbility Scan;
	Scan.Type = EExoAbilityType::AreaScan;
	Scan.Cooldown = 20.f;
	Scan.CooldownRemaining = 0.f;
	Scan.AbilityName = TEXT("Area Scan");
	Abilities.Add(Scan);

	FExoAbility Shield;
	Shield.Type = EExoAbilityType::ShieldBubble;
	Shield.Cooldown = 30.f;
	Shield.CooldownRemaining = 0.f;
	Shield.AbilityName = TEXT("Shield Bubble");
	Abilities.Add(Shield);

	FExoAbility Grapple;
	Grapple.Type = EExoAbilityType::GrappleHook;
	Grapple.Cooldown = 15.f;
	Grapple.CooldownRemaining = 0.f;
	Grapple.AbilityName = TEXT("Grapple Hook");
	Abilities.Add(Grapple);

	FExoAbility DecoyAbility;
	DecoyAbility.Type = EExoAbilityType::Decoy;
	DecoyAbility.Cooldown = 25.f;
	DecoyAbility.CooldownRemaining = 0.f;
	DecoyAbility.AbilityName = TEXT("Decoy");
	Abilities.Add(DecoyAbility);
}

void UExoAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Reduce cooldown timers
	for (FExoAbility& Ability : Abilities)
	{
		if (Ability.CooldownRemaining > 0.f)
		{
			float PrevCooldown = Ability.CooldownRemaining;
			Ability.CooldownRemaining = FMath::Max(Ability.CooldownRemaining - DeltaTime, 0.f);

			if (PrevCooldown > 0.f && Ability.CooldownRemaining <= 0.f)
			{
				OnAbilityCooldownComplete.Broadcast(Ability.Type);
			}
		}
	}

	TickAreaScan(DeltaTime);
	TickGrapple(DeltaTime);
}

void UExoAbilityComponent::UseAbility(EExoAbilityType Type)
{
	// Grapple blocks other abilities while active
	if (bIsGrappling && Type != EExoAbilityType::GrappleHook) return;

	for (FExoAbility& Ability : Abilities)
	{
		if (Ability.Type != Type) continue;
		if (!Ability.bIsReady()) return;

		switch (Type)
		{
		case EExoAbilityType::Dash:         ExecuteDash(); break;
		case EExoAbilityType::AreaScan:     ExecuteAreaScan(); break;
		case EExoAbilityType::ShieldBubble: ExecuteShieldBubble(); break;
		case EExoAbilityType::GrappleHook:  ExecuteGrapple(); break;
		case EExoAbilityType::Decoy:        ExecuteDecoy(); break;
		}

		Ability.CooldownRemaining = Ability.Cooldown;
		OnAbilityUsed.Broadcast(Type);

		UE_LOG(LogExoRift, Log, TEXT("Ability used: %s"), *Ability.AbilityName);
		return;
	}
}

void UExoAbilityComponent::ExecuteDash()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	FVector LookDir = Char->GetControlRotation().Vector();
	LookDir.Z = 0.f;
	LookDir.Normalize();

	Char->LaunchCharacter(LookDir * DashImpulse, true, false);
}

void UExoAbilityComponent::ExecuteAreaScan()
{
	ScannedEnemies.Empty();
	ScanTimeRemaining = ScanDuration;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Origin = Owner->GetActorLocation();

	// Find all characters within scan radius
	for (TActorIterator<AExoCharacter> It(GetWorld()); It; ++It)
	{
		AExoCharacter* Other = *It;
		if (Other == Owner) continue;
		if (!Other->IsAlive()) continue;

		float Dist = FVector::Dist(Origin, Other->GetActorLocation());
		if (Dist <= ScanRadius)
		{
			ScannedEnemies.Add(Other);
		}
	}

	UE_LOG(LogExoRift, Log, TEXT("AreaScan revealed %d enemies"), ScannedEnemies.Num());
}

void UExoAbilityComponent::ExecuteShieldBubble()
{
	AExoCharacter* Char = Cast<AExoCharacter>(GetOwner());
	if (!Char) return;

	UExoShieldComponent* Shield = Char->GetShieldComponent();
	if (Shield)
	{
		Shield->AddShield(ShieldRestoreAmount);
	}
}

void UExoAbilityComponent::ExecuteGrapple()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	FVector Start = Char->GetActorLocation();
	FVector Forward = Char->GetControlRotation().Vector();
	FVector End = Start + Forward * GrappleRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_WorldStatic, Params);

	if (bHit)
	{
		bIsGrappling = true;
		GrappleTarget = Hit.ImpactPoint;
		GrappleStartLocation = Start;
		GrappleTimer = 0.f;

		// Disable gravity while grappling for smooth traversal
		UCharacterMovementComponent* Movement = Char->GetCharacterMovement();
		if (Movement)
		{
			Movement->GravityScale = 0.f;
		}

		CreateGrappleBeam();

		UE_LOG(LogExoRift, Log, TEXT("Grapple attached at distance %.0f"),
			FVector::Dist(Start, GrappleTarget));
	}
}

void UExoAbilityComponent::ExecuteDecoy()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SpawnLoc = Owner->GetActorLocation();
	FRotator SpawnRot = Owner->GetActorRotation();

	AExoDecoyActor* Decoy = World->SpawnActor<AExoDecoyActor>(
		AExoDecoyActor::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);

	if (Decoy)
	{
		Decoy->SetLifeSpan(DecoyLifetime);
		UE_LOG(LogExoRift, Log, TEXT("Decoy deployed, auto-destroy in %.0fs"), DecoyLifetime);
	}
}

void UExoAbilityComponent::TickAreaScan(float DeltaTime)
{
	if (ScanTimeRemaining <= 0.f) return;

	ScanTimeRemaining -= DeltaTime;
	if (ScanTimeRemaining <= 0.f)
	{
		ScannedEnemies.Empty();
		ScanTimeRemaining = 0.f;
	}
}

void UExoAbilityComponent::TickGrapple(float DeltaTime)
{
	if (!bIsGrappling) return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char)
	{
		bIsGrappling = false;
		DestroyGrappleBeam();
		return;
	}

	GrappleTimer += DeltaTime;
	float Alpha = FMath::Clamp(GrappleTimer / GrappleDuration, 0.f, 1.f);

	float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 2.f);
	FVector NewLocation = FMath::Lerp(GrappleStartLocation, GrappleTarget, EasedAlpha);
	Char->SetActorLocation(NewLocation);

	UpdateGrappleBeam();

	if (Alpha >= 1.f)
	{
		bIsGrappling = false;
		DestroyGrappleBeam();

		UCharacterMovementComponent* Movement = Char->GetCharacterMovement();
		if (Movement)
		{
			Movement->GravityScale = 1.f;
		}

		FVector ArrivalDir = (GrappleTarget - GrappleStartLocation).GetSafeNormal();
		Char->LaunchCharacter(ArrivalDir * 400.f, false, false);
	}
}

void UExoAbilityComponent::CreateGrappleBeam()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	static UStaticMesh* CylMesh = nullptr;
	static UMaterialInterface* BaseMat = nullptr;
	if (!CylMesh)
	{
		CylMesh = LoadObject<UStaticMesh>(nullptr,
			TEXT("/Engine/BasicShapes/Cylinder"));
		BaseMat = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	}

	if (!CylMesh || !BaseMat) return;

	// Beam cylinder
	GrappleBeam = NewObject<UStaticMeshComponent>(Owner);
	GrappleBeam->SetStaticMesh(CylMesh);
	GrappleBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrappleBeam->CastShadow = false;
	GrappleBeam->RegisterComponent();

	GrappleBeamMat = UMaterialInstanceDynamic::Create(BaseMat, Owner);
	FLinearColor BeamCol(1.f, 6.f, 3.f); // Bright green energy
	GrappleBeamMat->SetVectorParameterValue(TEXT("BaseColor"), BeamCol);
	GrappleBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"), BeamCol);
	GrappleBeam->SetMaterial(0, GrappleBeamMat);

	// Glow light at target
	GrappleLight = NewObject<UPointLightComponent>(Owner);
	GrappleLight->SetWorldLocation(GrappleTarget);
	GrappleLight->SetIntensity(20000.f);
	GrappleLight->SetAttenuationRadius(800.f);
	GrappleLight->SetLightColor(FLinearColor(0.2f, 1.f, 0.5f));
	GrappleLight->CastShadows = false;
	GrappleLight->RegisterComponent();
}

void UExoAbilityComponent::UpdateGrappleBeam()
{
	if (!GrappleBeam) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector CharPos = Owner->GetActorLocation();
	FVector Mid = (CharPos + GrappleTarget) * 0.5f;
	FVector Dir = GrappleTarget - CharPos;
	float Length = Dir.Size();

	// Position beam at midpoint
	GrappleBeam->SetWorldLocation(Mid);

	// Orient cylinder along the beam direction
	FRotator BeamRot = Dir.Rotation();
	BeamRot.Pitch += 90.f; // Cylinder default is Z-up
	GrappleBeam->SetWorldRotation(BeamRot);

	// Scale: thin cylinder stretched to beam length
	float LenScale = Length / 100.f;
	float Time = GetWorld()->GetTimeSeconds();
	float Pulse = 1.f + 0.15f * FMath::Sin(Time * 30.f);
	GrappleBeam->SetWorldScale3D(FVector(0.015f * Pulse, 0.015f * Pulse, LenScale));

	// Pulse emissive
	if (GrappleBeamMat)
	{
		float EmPulse = 1.f + 0.3f * FMath::Sin(Time * 20.f);
		FLinearColor BeamCol(1.f * EmPulse, 6.f * EmPulse, 3.f * EmPulse);
		GrappleBeamMat->SetVectorParameterValue(TEXT("EmissiveColor"), BeamCol);
	}

	if (GrappleLight)
	{
		GrappleLight->SetWorldLocation(GrappleTarget);
	}
}

void UExoAbilityComponent::DestroyGrappleBeam()
{
	if (GrappleBeam)
	{
		GrappleBeam->DestroyComponent();
		GrappleBeam = nullptr;
	}
	if (GrappleLight)
	{
		GrappleLight->DestroyComponent();
		GrappleLight = nullptr;
	}
	GrappleBeamMat = nullptr;
}
