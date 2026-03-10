// ExoWeaponAura.cpp — Animated rarity glow on weapons
#include "Visual/ExoWeaponAura.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UExoWeaponAura::UExoWeaponAura()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphF.Succeeded()) SphereMesh = SphF.Object;
}

void UExoWeaponAura::InitAura(EWeaponRarity InRarity, const FLinearColor& RarityColor)
{
	AuraColor = RarityColor;

	switch (InRarity)
	{
	case EWeaponRarity::Epic:      ActiveOrbs = 2; break;
	case EWeaponRarity::Legendary: ActiveOrbs = 4; break;
	default: ActiveOrbs = 0; break;
	}

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (ActiveOrbs == 0 || !SphereMesh || !EmissiveMat) return;

	for (int32 i = 0; i < ActiveOrbs; i++)
	{
		Orbs[i] = NewObject<UStaticMeshComponent>(GetOwner());
		Orbs[i]->SetupAttachment(this);
		Orbs[i]->SetStaticMesh(SphereMesh);
		Orbs[i]->SetRelativeScale3D(FVector(0.015f));
		Orbs[i]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Orbs[i]->CastShadow = false;
		Orbs[i]->RegisterComponent();

		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmissiveMat, GetOwner());
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(RarityColor.R * 25.f, RarityColor.G * 25.f, RarityColor.B * 25.f));
		Orbs[i]->SetMaterial(0, Mat);
		OrbMats.Add(Mat);
	}

	// Ambient aura light
	AuraLight = NewObject<UPointLightComponent>(GetOwner());
	AuraLight->SetupAttachment(this);
	AuraLight->SetRelativeLocation(FVector(20.f, 0.f, 0.f));
	float LightIntensity = (InRarity == EWeaponRarity::Legendary) ? 1200.f : 400.f;
	AuraLight->SetIntensity(LightIntensity);
	AuraLight->SetAttenuationRadius(80.f);
	AuraLight->SetLightColor(RarityColor);
	AuraLight->CastShadows = false;
	AuraLight->RegisterComponent();
}

void UExoWeaponAura::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveOrbs == 0) return;

	Phase += DeltaTime;

	for (int32 i = 0; i < ActiveOrbs; i++)
	{
		if (!Orbs[i]) continue;

		// Orbital path — each orb at different phase offset
		float OrbPhase = Phase * 3.f + (i * 2.f * PI / ActiveOrbs);
		float OrbitRadius = 8.f + FMath::Sin(Phase * 1.5f + i * 0.7f) * 3.f;

		float X = 20.f + FMath::Cos(OrbPhase * 0.5f) * 10.f; // Forward-back drift
		float Y = FMath::Cos(OrbPhase) * OrbitRadius;
		float Z = FMath::Sin(OrbPhase) * OrbitRadius;
		Orbs[i]->SetRelativeLocation(FVector(X, Y, Z));

		// Pulse size
		float SizePulse = 0.012f + 0.005f * FMath::Sin(Phase * 4.f + i * 1.3f);
		Orbs[i]->SetRelativeScale3D(FVector(SizePulse));

		// Pulse emissive
		if (OrbMats.IsValidIndex(i) && OrbMats[i])
		{
			float EmissivePulse = 20.f + 10.f * FMath::Sin(Phase * 5.f + i * 1.7f);
			OrbMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(AuraColor.R * EmissivePulse, AuraColor.G * EmissivePulse,
					AuraColor.B * EmissivePulse));
		}
	}

	// Aura light breathe
	if (AuraLight)
	{
		float BaseLightIntensity = (ActiveOrbs >= 4) ? 1200.f : 400.f;
		float Breathe = 1.f + 0.3f * FMath::Sin(Phase * 2.f);
		AuraLight->SetIntensity(BaseLightIntensity * Breathe);
	}
}
