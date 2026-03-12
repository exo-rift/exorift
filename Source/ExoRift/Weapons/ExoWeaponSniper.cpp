#include "Weapons/ExoWeaponSniper.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "Visual/ExoScreenShake.h"

AExoWeaponSniper::AExoWeaponSniper()
{
	WeaponName = TEXT("Void Lance");
	WeaponType = EWeaponType::Sniper;
	bIsAutomatic = false;
	FireRate = 1.f;
	Damage = 120.f;
	MaxRange = 80000.f;

	// Heat system: 35/100 normalized to 0-1 range
	HeatPerShot = 0.35f;
	CooldownRate = 0.2f;
	OverheatCooldownRate = 0.07f; // Slow recovery from overheat (penalty = 3.0s feel)
	OverheatRecoveryThreshold = 0.3f;

	// Energy cell
	MaxEnergy = 20.f;
	CurrentEnergy = 20.f;
	EnergyPerShot = 5.f;

	// Long-range precision: minimal spread
	BaseSpread = 0.f;
	MaxSpread = 1.f;
	SpreadPerShot = 0.8f;
	SpreadRecoveryRate = 2.f;

	// Heavy recoil for a high-caliber weapon
	RecoilPitch = -1.2f;
	RecoilYawRange = 0.3f;

	// Headshot
	HeadshotMultiplier = 3.f;

	// Damage falloff: full damage to 400m, falloff to 800m
	FalloffStartRange = 40000.f;
	FalloffEndRange = 80000.f;
	MinDamageMultiplier = 0.5f;

	// Deep scope zoom and near-zero ADS spread
	ADSFOV = 30.f;
	ADSSpreadMultiplier = 0.05f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SniperMesh(
		TEXT("/Game/Weapons/Sniper/SKM_Sniper"));
	if (SniperMesh.Succeeded())
	{
		WeaponMesh->SetSkeletalMesh(SniperMesh.Object);
	}

	// Scope glint — bright flash visible to other players
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
		TEXT("/Engine/BasicShapes/Sphere"));
	GlintMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlintMesh"));
	GlintMesh->SetupAttachment(WeaponMesh);
	GlintMesh->SetRelativeLocation(FVector(80.f, 0.f, 8.f)); // Scope lens position
	GlintMesh->SetRelativeScale3D(FVector(0.001f));
	GlintMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GlintMesh->CastShadow = false;
	GlintMesh->SetVisibility(false);
	if (SphereFinder.Succeeded()) GlintMesh->SetStaticMesh(SphereFinder.Object);

	// Scope glint — pure energy flash (additive, unlit)
	UMaterialInterface* EmissiveAdditive = FExoMaterialFactory::GetEmissiveAdditive();
	if (EmissiveAdditive)
	{
		UMaterialInstanceDynamic* GlintMat = UMaterialInstanceDynamic::Create(
			EmissiveAdditive, this);
		if (!GlintMat) { return; }
		GlintMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(180.f, 160.f, 110.f, 1.f)); // Blazing white-gold
		GlintMesh->SetMaterial(0, GlintMat);
	}

	GlintLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlintLight"));
	GlintLight->SetupAttachment(GlintMesh);
	GlintLight->SetIntensity(0.f);
	GlintLight->SetAttenuationRadius(5000.f);
	GlintLight->SetLightColor(FLinearColor(1.f, 0.9f, 0.6f));
	GlintLight->CastShadows = false;
}

void AExoWeaponSniper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Scope glint — ramp up when scoped, pulse for visibility
	float GlintTarget = bIsScoped ? 1.f : 0.f;
	GlintBlend = FMath::FInterpTo(GlintBlend, GlintTarget, DeltaTime, 4.f);
	if (GlintBlend > 0.01f)
	{
		float Time = GetWorld()->GetTimeSeconds();
		float Pulse = 0.6f + 0.4f * FMath::Sin(Time * 3.f);
		float Intensity = GlintBlend * Pulse;
		float S = 0.08f * Intensity;
		GlintMesh->SetVisibility(true);
		GlintMesh->SetRelativeScale3D(FVector(S));
		GlintLight->SetIntensity(140000.f * Intensity);
	}
	else
	{
		GlintMesh->SetVisibility(false);
		GlintLight->SetIntensity(0.f);
	}

	if (bIsScoped)
	{
		// Hold-breath: gradually deplete, adding sway when expired
		if (bIsHoldingBreath)
		{
			HoldBreathTimer += DeltaTime;
			if (HoldBreathTimer >= MaxHoldBreath)
			{
				bIsHoldingBreath = false;
			}
		}

		// When breath runs out, add subtle scope drift
		if (!bIsHoldingBreath)
		{
			float Time = GetWorld()->GetTimeSeconds();
			float DriftX = FMath::Sin(Time * 1.5f) * 0.05f + FMath::Sin(Time * 3.7f) * 0.03f;
			float DriftY = FMath::Sin(Time * 2.1f) * 0.04f + FMath::Sin(Time * 4.3f) * 0.02f;

			APawn* P = Cast<APawn>(GetOwner());
			if (P && P->GetController())
			{
				P->GetController()->SetControlRotation(
					P->GetController()->GetControlRotation() +
					FRotator(DriftX, DriftY, 0.f));
			}
		}
	}
	else
	{
		// Recover hold-breath when not scoped
		HoldBreathTimer = FMath::Max(0.f, HoldBreathTimer - DeltaTime / HoldBreathRecovery * MaxHoldBreath);
		if (HoldBreathTimer <= 0.f)
			bIsHoldingBreath = true;
	}
}

void AExoWeaponSniper::StartADS()
{
	Super::StartADS();
	bIsScoped = true;
	// Reset hold-breath if it was recovered
	if (bIsHoldingBreath && HoldBreathTimer <= 0.f)
		HoldBreathTimer = 0.f;
}

void AExoWeaponSniper::StopADS()
{
	Super::StopADS();
	bIsScoped = false;
}

float AExoWeaponSniper::GetScopeHoldProgress() const
{
	return FMath::Clamp(HoldBreathTimer / MaxHoldBreath, 0.f, 1.f);
}

float AExoWeaponSniper::GetBreathHoldFactor() const
{
	// 0 = holding breath (no sway), ramps to 1 as breath depletes
	if (!bIsScoped) return 1.f;
	if (bIsHoldingBreath)
	{
		// Smooth ramp: starts at 0.15 (near-still), grows as breath depletes
		float Progress = GetScopeHoldProgress(); // 0 = full breath, 1 = empty
		return FMath::Lerp(0.15f, 0.6f, Progress);
	}
	return 1.f; // Breath exhausted — full ADS sway
}
