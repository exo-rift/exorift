// ExoDeathEffect.cpp — Energy burst, rising pillar, scorch mark, scattering fragments
#include "Visual/ExoDeathEffect.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoScreenShake.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AExoDeathEffect::AExoDeathEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 1.5f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));

	// Core flash sphere
	CoreFlash = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreFlash"));
	RootComponent = CoreFlash;
	CoreFlash->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreFlash->CastShadow = false;
	CoreFlash->SetGenerateOverlapEvents(false);
	if (SphereFinder.Succeeded()) CoreFlash->SetStaticMesh(SphereFinder.Object);
	CoreFlash->SetWorldScale3D(FVector(0.1f));

	// Primary expanding shockwave ring
	ShockRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShockRing"));
	ShockRing->SetupAttachment(CoreFlash);
	ShockRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShockRing->CastShadow = false;
	ShockRing->SetGenerateOverlapEvents(false);
	if (CylFinder.Succeeded()) ShockRing->SetStaticMesh(CylFinder.Object);
	ShockRing->SetWorldScale3D(FVector(0.01f, 0.01f, 0.001f));

	// Secondary delayed ring — expands slightly after the first
	SecondaryRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondaryRing"));
	SecondaryRing->SetupAttachment(CoreFlash);
	SecondaryRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SecondaryRing->CastShadow = false;
	SecondaryRing->SetGenerateOverlapEvents(false);
	SecondaryRing->SetVisibility(false);
	if (CylFinder.Succeeded()) SecondaryRing->SetStaticMesh(CylFinder.Object);

	// Rising energy pillar — vertical column of light
	EnergyPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EnergyPillar"));
	EnergyPillar->SetupAttachment(CoreFlash);
	EnergyPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnergyPillar->CastShadow = false;
	EnergyPillar->SetGenerateOverlapEvents(false);
	EnergyPillar->SetVisibility(false);
	if (CylFinder.Succeeded()) EnergyPillar->SetStaticMesh(CylFinder.Object);

	// Ground scorch mark
	GroundScorch = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundScorch"));
	GroundScorch->SetupAttachment(CoreFlash);
	GroundScorch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GroundScorch->CastShadow = false;
	GroundScorch->SetGenerateOverlapEvents(false);
	GroundScorch->SetVisibility(false);
	if (CylFinder.Succeeded()) GroundScorch->SetStaticMesh(CylFinder.Object);

	// Bright burst light
	BurstLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BurstLight"));
	BurstLight->SetupAttachment(CoreFlash);
	BurstLight->SetIntensity(450000.f);
	BurstLight->SetAttenuationRadius(5000.f);
	BurstLight->SetLightColor(FLinearColor(0.3f, 0.6f, 1.f));
	BurstLight->CastShadows = false;

	// Pillar overhead light
	PillarLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PillarLight"));
	PillarLight->SetupAttachment(CoreFlash);
	PillarLight->SetIntensity(0.f);
	PillarLight->SetAttenuationRadius(3000.f);
	PillarLight->CastShadows = false;

	// Fragment cubes — more fragments for dramatic scatter
	for (int32 i = 0; i < NumFragments; i++)
	{
		FName Name = *FString::Printf(TEXT("Frag_%d"), i);
		UStaticMeshComponent* Frag = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Frag->SetupAttachment(CoreFlash);
		Frag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Frag->CastShadow = false;
		Frag->SetGenerateOverlapEvents(false);
		// Mix cube and sphere fragments for variety
		if (i % 3 == 0 && SphereFinder.Succeeded())
			Frag->SetStaticMesh(SphereFinder.Object);
		else if (CubeFinder.Succeeded())
			Frag->SetStaticMesh(CubeFinder.Object);
		Fragments.Add(Frag);
	}
}

void AExoDeathEffect::Init(const FLinearColor& AccentColor)
{
	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveAdditive();
	UMaterialInterface* EmOpaque = FExoMaterialFactory::GetEmissiveOpaque();

	// Core flash — intense white-blue burst
	if (EmMat)
	{
		UMaterialInstanceDynamic* FlashMat = UMaterialInstanceDynamic::Create(EmMat, this);
		if (!FlashMat) { return; }
		FlashMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(70.f, 80.f, 115.f));
		CoreFlash->SetMaterial(0, FlashMat);
	}

	// Primary shock ring — accent-colored
	if (EmMat)
	{
		UMaterialInstanceDynamic* RingMat = UMaterialInstanceDynamic::Create(EmMat, this);
		if (!RingMat) { return; }
		RingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(AccentColor.R * 10.f, AccentColor.G * 10.f, AccentColor.B * 10.f));
		ShockRing->SetMaterial(0, RingMat);
	}

	// Secondary ring — white pulse
	if (EmMat)
	{
		UMaterialInstanceDynamic* Sec = UMaterialInstanceDynamic::Create(EmMat, this);
		if (!Sec) { return; }
		Sec->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(18.f, 22.f, 35.f));
		SecondaryRing->SetMaterial(0, Sec);
	}

	// Energy pillar — tall column, accent color
	if (EmMat)
	{
		UMaterialInstanceDynamic* PilMat = UMaterialInstanceDynamic::Create(EmMat, this);
		if (!PilMat) { return; }
		PilMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(AccentColor.R * 6.f, AccentColor.G * 6.f, AccentColor.B * 6.f));
		EnergyPillar->SetMaterial(0, PilMat);
	}

	// Ground scorch — dark ember glow
	if (EmOpaque)
	{
		UMaterialInstanceDynamic* ScMat = UMaterialInstanceDynamic::Create(EmOpaque, this);
		if (!ScMat) { return; }
		ScMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(AccentColor.R * 0.5f, AccentColor.G * 0.3f, AccentColor.B * 0.2f));
		GroundScorch->SetMaterial(0, ScMat);
	}

	BurstLight->SetLightColor(AccentColor);
	PillarLight->SetLightColor(AccentColor);

	// Screen shake
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		FExoScreenShake::AddExplosionShake(GetActorLocation(),
			PC->GetPawn()->GetActorLocation(), 4000.f, 1.5f);
	}

	// Fragment velocities and materials — high-energy scatter
	FragVelocities.SetNum(NumFragments);
	for (int32 i = 0; i < NumFragments; i++)
	{
		FVector Vel = FMath::VRand() * FMath::RandRange(500.f, 1200.f);
		Vel.Z = FMath::Abs(Vel.Z) + 300.f;
		FragVelocities[i] = Vel;

		float S = FMath::RandRange(0.02f, 0.08f);
		Fragments[i]->SetWorldScale3D(FVector(S));

		if (EmMat)
		{
			UMaterialInstanceDynamic* FragMat = UMaterialInstanceDynamic::Create(EmMat, this);
			if (!FragMat) { return; }
			float Brightness = FMath::RandRange(3.f, 10.f);
			FragMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(AccentColor.R * Brightness,
					AccentColor.G * Brightness, AccentColor.B * Brightness));
			Fragments[i]->SetMaterial(0, FragMat);
		}
	}
}

void AExoDeathEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);

	// Core flash: rapid expand then fade
	{
		float ExpandT = FMath::Clamp(Age / (Lifetime * 0.15f), 0.f, 1.f);
		float MaxScale = 3.5f;
		float Scale = FMath::Lerp(0.1f, MaxScale, FMath::Sqrt(ExpandT));
		float FadeT = FMath::Clamp((Age - Lifetime * 0.1f) / (Lifetime * 0.4f), 0.f, 1.f);
		Scale *= (1.f - FadeT * FadeT);
		CoreFlash->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));
	}

	// Primary shock ring: expand outward
	{
		float RingT = FMath::Clamp(Age / (Lifetime * 0.6f), 0.f, 1.f);
		float RingScale = 6.f * RingT;
		float RingAlpha = 1.f - RingT;
		ShockRing->SetWorldScale3D(FVector(RingScale, RingScale, 0.004f * RingAlpha));
		ShockRing->SetVisibility(RingT < 1.f);
	}

	// Secondary ring: delayed by 0.15s, smaller and faster
	{
		float Delay = 0.15f;
		float SecT = FMath::Clamp((Age - Delay) / (Lifetime * 0.4f), 0.f, 1.f);
		if (SecT > 0.f && !SecondaryRing->IsVisible())
			SecondaryRing->SetVisibility(true);
		if (SecT > 0.f)
		{
			float SecScale = 4.f * SecT;
			float SecAlpha = 1.f - SecT;
			SecondaryRing->SetWorldScale3D(FVector(SecScale, SecScale, 0.003f * SecAlpha));
			if (SecT >= 1.f) SecondaryRing->SetVisibility(false);
		}
	}

	// Energy pillar: rises from center, thin and tall
	{
		float PilT = FMath::Clamp((Age - 0.05f) / (Lifetime * 0.7f), 0.f, 1.f);
		if (PilT > 0.f && !EnergyPillar->IsVisible())
			EnergyPillar->SetVisibility(true);
		if (PilT > 0.f)
		{
			float PilHeight = 800.f * FMath::Sqrt(PilT);
			float PilWidth = 0.15f * (1.f - PilT * PilT);
			EnergyPillar->SetRelativeLocation(FVector(0.f, 0.f, PilHeight * 0.5f));
			EnergyPillar->SetWorldScale3D(FVector(PilWidth, PilWidth, PilHeight / 100.f));
			PillarLight->SetRelativeLocation(FVector(0.f, 0.f, PilHeight));
			PillarLight->SetIntensity(90000.f * (1.f - PilT) * (1.f - PilT));
			if (PilT >= 1.f) EnergyPillar->SetVisibility(false);
		}
	}

	// Ground scorch: appears after flash, persists
	{
		float ScT = FMath::Clamp((Age - 0.1f) / (Lifetime * 0.3f), 0.f, 1.f);
		if (ScT > 0.f && !GroundScorch->IsVisible())
			GroundScorch->SetVisibility(true);
		if (ScT > 0.f)
		{
			float ScScale = 2.5f * FMath::Sqrt(ScT);
			GroundScorch->SetRelativeLocation(FVector(0.f, 0.f, -GetActorLocation().Z + 2.f));
			GroundScorch->SetWorldScale3D(FVector(ScScale, ScScale, 0.02f));
		}
	}

	// Lights: fast decay
	BurstLight->SetIntensity(450000.f * (1.f - T) * (1.f - T) * (1.f - T));

	// Fragments: fly outward with gravity and spin
	for (int32 i = 0; i < Fragments.Num(); i++)
	{
		if (!Fragments[i]) continue;
		FVector Pos = FragVelocities[i] * Age;
		Pos.Z -= 490.f * Age * Age;
		Fragments[i]->SetRelativeLocation(Pos);

		FRotator Rot(Age * 400.f * (i + 1), Age * 250.f * (i + 2), Age * 180.f);
		Fragments[i]->SetRelativeRotation(Rot);

		float S = FMath::Lerp(0.06f, 0.002f, T * T);
		Fragments[i]->SetWorldScale3D(FVector(S));

		// Fade fragment emissive
		UMaterialInstanceDynamic* FMat = Cast<UMaterialInstanceDynamic>(Fragments[i]->GetMaterial(0));
		if (FMat)
		{
			FLinearColor Em;
			FMat->GetVectorParameterValue(TEXT("EmissiveColor"), Em);
			float Fade = (1.f - T * T);
			FMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Em.R * Fade, Em.G * Fade, Em.B * Fade));
		}
	}

	if (Age >= Lifetime) Destroy();
}
