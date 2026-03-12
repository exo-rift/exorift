// ExoJumpPad.cpp — Launch pad for vertical repositioning
#include "Map/ExoJumpPad.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Visual/ExoScreenShake.h"
#include "Core/ExoAudioManager.h"
#include "Visual/ExoLaunchColumn.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoJumpPad::AExoJumpPad()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphFind(
		TEXT("/Engine/BasicShapes/Sphere"));

	// Dark metal base
	BasePlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BasePlatform"));
	RootComponent = BasePlatform;
	if (CylFind.Succeeded()) BasePlatform->SetStaticMesh(CylFind.Object);
	BasePlatform->SetRelativeScale3D(FVector(3.f, 3.f, 0.15f));
	BasePlatform->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BasePlatform->SetCollisionResponseToAllChannels(ECR_Block);

	// Glowing ring on top
	GlowRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlowRing"));
	GlowRing->SetupAttachment(BasePlatform);
	if (CylFind.Succeeded()) GlowRing->SetStaticMesh(CylFind.Object);
	GlowRing->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	GlowRing->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.02f));
	GlowRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GlowRing->CastShadow = false;

	// Center lens (sphere)
	CenterLens = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CenterLens"));
	CenterLens->SetupAttachment(BasePlatform);
	if (SphFind.Succeeded()) CenterLens->SetStaticMesh(SphFind.Object);
	CenterLens->SetRelativeLocation(FVector(0.f, 0.f, 15.f));
	CenterLens->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.1f));
	CenterLens->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CenterLens->CastShadow = false;

	// Trigger volume
	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(BasePlatform);
	TriggerVolume->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	TriggerVolume->SetSphereRadius(120.f);
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetGenerateOverlapEvents(true);

	// Accent light
	PadLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PadLight"));
	PadLight->SetupAttachment(BasePlatform);
	PadLight->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	PadLight->SetIntensity(12000.f);
	PadLight->SetAttenuationRadius(1000.f);
	PadLight->CastShadows = false;
}

void AExoJumpPad::BeginPlay()
{
	Super::BeginPlay();
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AExoJumpPad::OnPadOverlap);

	// Apply PBR metallic material to base platform
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (LitMat && BasePlatform->GetStaticMesh())
	{
		UMaterialInstanceDynamic* BaseMat = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!BaseMat) { return; }
		BaseMat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.05f, 0.055f, 0.07f));
		BaseMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		BaseMat->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
		BaseMat->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
		BasePlatform->SetMaterial(0, BaseMat);
	}

	{
		UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		RingMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!RingMat) { return; }
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			AccentColor * 14.f);
		GlowRing->SetMaterial(0, RingMat);

		LensMat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!LensMat) { return; }
		LensMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			AccentColor * 22.f);
		CenterLens->SetMaterial(0, LensMat);
	}

	PadLight->SetLightColor(AccentColor);
}

void AExoJumpPad::InitPad(float InLaunchSpeed, const FLinearColor& Color)
{
	LaunchSpeed = InLaunchSpeed;
	AccentColor = Color;
	PadLight->SetLightColor(Color);
}

void AExoJumpPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Cooldown > 0.f) Cooldown -= DeltaTime;

	float Time = GetWorld()->GetTimeSeconds();
	bool bReady = Cooldown <= 0.f;

	// Pulsing glow when ready, dim when on cooldown
	float Pulse = bReady ? (0.7f + 0.3f * FMath::Sin(Time * 3.f)) : 0.15f;

	if (RingMat)
	{
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			AccentColor * 4.f * Pulse);
	}
	if (LensMat)
	{
		LensMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			AccentColor * 6.f * Pulse);
	}
	PadLight->SetIntensity(bReady ? 4000.f * Pulse : 800.f);
}

void AExoJumpPad::OnPadOverlap(UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor, UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (Cooldown > 0.f) return;

	ACharacter* Char = Cast<ACharacter>(OtherActor);
	if (!Char) return;

	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!CMC) return;

	// Launch upward
	Char->LaunchCharacter(FVector(0.f, 0.f, LaunchSpeed), false, true);
	Cooldown = CooldownTime;

	// Feedback
	FExoScreenShake::AddShake(0.15f, 0.1f);

	if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
	{
		Audio->PlayImpactSound(GetActorLocation(), false);
	}

	// Launch column VFX — dramatic energy burst shooting upward
	AExoLaunchColumn::SpawnColumn(GetWorld(), GetActorLocation(), AccentColor, LaunchSpeed * 0.4f);
}
