// ExoLevelBuilderAtmosphereHelpers.cpp — Hologram, spotlight, conduit, neon helpers
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::SpawnHolographicDisplay(const FVector& Pos, float Yaw, float Scale)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FLinearColor DarkFrame(0.04f, 0.04f, 0.06f);

	// Support pillar
	SpawnStaticMesh(Pos - FVector(0.f, 0.f, Pos.Z * 0.5f),
		FVector(0.15f, 0.15f, Pos.Z / 100.f) * Scale, Rot, CylinderMesh, DarkFrame);

	// Projector base (disk on top of pillar)
	SpawnStaticMesh(Pos, FVector(0.6f, 0.6f, 0.05f) * Scale, Rot, CylinderMesh, DarkFrame);

	// Holographic screen — thin glowing rectangle floating above
	float ScreenH = 180.f * Scale;
	FVector ScreenPos = Pos + FVector(0.f, 0.f, ScreenH * 0.5f + 40.f);
	UStaticMeshComponent* Screen = SpawnStaticMesh(ScreenPos,
		FVector(2.5f, 0.02f, 1.5f) * Scale,
		Rot + FRotator(5.f, 0.f, 0.f),
		CubeMesh, FLinearColor(0.03f, 0.15f, 0.25f));
	if (Screen && BaseMaterial)
	{
		UMaterialInstanceDynamic* SM = Cast<UMaterialInstanceDynamic>(Screen->GetMaterial(0));
		if (SM) SM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(0.5f, 2.f, 4.f));
	}

	// Scanline bars (3 horizontal lines on screen)
	for (int32 i = 0; i < 3; i++)
	{
		float BarZ = -40.f + i * 40.f;
		FVector BarPos = ScreenPos + Rot.RotateVector(FVector(0.f, 2.f, BarZ * Scale));
		UStaticMeshComponent* Bar = SpawnStaticMesh(BarPos,
			FVector(2.2f, 0.01f, 0.05f) * Scale, Rot,
			CubeMesh, FLinearColor(0.1f, 0.4f, 0.6f));
		if (Bar && BaseMaterial)
		{
			UMaterialInstanceDynamic* BM = Cast<UMaterialInstanceDynamic>(Bar->GetMaterial(0));
			if (BM) BM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(1.f, 3.f, 5.f));
		}
	}

	// Hologram light — cyan glow
	UPointLightComponent* HoloLight = NewObject<UPointLightComponent>(this);
	HoloLight->SetupAttachment(RootComponent);
	HoloLight->SetWorldLocation(ScreenPos);
	HoloLight->SetIntensity(8000.f * Scale);
	HoloLight->SetAttenuationRadius(800.f * Scale);
	HoloLight->SetLightColor(FLinearColor(0.15f, 0.5f, 0.8f));
	HoloLight->CastShadows = false;
	HoloLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnSpotlightBeam(const FVector& Base, float Height,
	const FLinearColor& Color)
{
	FLinearColor DarkMetal(0.06f, 0.06f, 0.08f);

	// Pole
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height * 0.5f),
		FVector(0.3f, 0.3f, Height / 100.f), FRotator::ZeroRotator,
		CylinderMesh, DarkMetal);

	// Fixture housing (small box at top)
	SpawnStaticMesh(Base + FVector(0.f, 0.f, Height),
		FVector(0.5f, 0.5f, 0.3f), FRotator::ZeroRotator,
		CubeMesh, DarkMetal);

	// Spot light pointing down
	USpotLightComponent* Spot = NewObject<USpotLightComponent>(this);
	Spot->SetupAttachment(RootComponent);
	Spot->SetWorldLocation(Base + FVector(0.f, 0.f, Height));
	Spot->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	Spot->SetIntensity(50000.f);
	Spot->SetAttenuationRadius(Height * 1.5f);
	Spot->SetOuterConeAngle(35.f);
	Spot->SetInnerConeAngle(20.f);
	Spot->SetLightColor(Color);
	Spot->CastShadows = false;
	Spot->RegisterComponent();

	// Ground pool of light (bright circle on ground)
	UPointLightComponent* PoolLight = NewObject<UPointLightComponent>(this);
	PoolLight->SetupAttachment(RootComponent);
	PoolLight->SetWorldLocation(Base + FVector(0.f, 0.f, 50.f));
	PoolLight->SetIntensity(3000.f);
	PoolLight->SetAttenuationRadius(500.f);
	PoolLight->SetLightColor(Color);
	PoolLight->CastShadows = false;
	PoolLight->RegisterComponent();

	// Status LED on fixture
	UStaticMeshComponent* LED = SpawnStaticMesh(
		Base + FVector(20.f, 0.f, Height - 10.f),
		FVector(0.04f, 0.04f, 0.04f), FRotator::ZeroRotator,
		SphereMesh, Color);
	if (LED && BaseMaterial)
	{
		UMaterialInstanceDynamic* LM = Cast<UMaterialInstanceDynamic>(LED->GetMaterial(0));
		if (LM) LM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 8.f, Color.G * 8.f, Color.B * 8.f));
	}
}

void AExoLevelBuilder::SpawnEnergyConduit(const FVector& Start, const FVector& End,
	const FLinearColor& Color)
{
	FVector Dir = End - Start;
	float Len = Dir.Size();
	FVector Mid = (Start + End) * 0.5f;
	FRotator Rot = Dir.Rotation();

	// Outer pipe (dark casing)
	SpawnStaticMesh(Mid, FVector(0.25f, 0.25f, Len / 100.f),
		FRotator(0.f, Rot.Yaw, 90.f), CylinderMesh,
		FLinearColor(0.05f, 0.05f, 0.06f));

	// Inner glowing core
	UStaticMeshComponent* Core = SpawnStaticMesh(Mid,
		FVector(0.12f, 0.12f, Len / 100.f + 0.1f),
		FRotator(0.f, Rot.Yaw, 90.f), CylinderMesh, Color);
	if (Core && BaseMaterial)
	{
		UMaterialInstanceDynamic* CM = Cast<UMaterialInstanceDynamic>(Core->GetMaterial(0));
		if (CM) CM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 6.f, Color.G * 6.f, Color.B * 6.f));
	}

	// Junction nodes at start and end
	for (const FVector& Pt : {Start, End})
	{
		SpawnStaticMesh(Pt, FVector(0.4f, 0.4f, 0.4f),
			FRotator::ZeroRotator, SphereMesh,
			FLinearColor(0.05f, 0.05f, 0.06f));

		UPointLightComponent* JL = NewObject<UPointLightComponent>(this);
		JL->SetupAttachment(RootComponent);
		JL->SetWorldLocation(Pt);
		JL->SetIntensity(2000.f);
		JL->SetAttenuationRadius(400.f);
		JL->SetLightColor(Color);
		JL->CastShadows = false;
		JL->RegisterComponent();
	}

	// Mid-span glow light
	UPointLightComponent* MidLight = NewObject<UPointLightComponent>(this);
	MidLight->SetupAttachment(RootComponent);
	MidLight->SetWorldLocation(Mid);
	MidLight->SetIntensity(1500.f);
	MidLight->SetAttenuationRadius(300.f);
	MidLight->SetLightColor(Color);
	MidLight->CastShadows = false;
	MidLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnNeonTube(const FVector& Pos, const FVector& Scale,
	float Yaw, const FLinearColor& Color)
{
	FRotator Rot(0.f, Yaw, 0.f);

	UStaticMeshComponent* Tube = SpawnStaticMesh(Pos, Scale, Rot, CylinderMesh, Color);
	if (Tube && BaseMaterial)
	{
		UMaterialInstanceDynamic* TM = Cast<UMaterialInstanceDynamic>(Tube->GetMaterial(0));
		if (TM) TM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 10.f, Color.G * 10.f, Color.B * 10.f));
	}

	// Neon glow light
	UPointLightComponent* NL = NewObject<UPointLightComponent>(this);
	NL->SetupAttachment(RootComponent);
	NL->SetWorldLocation(Pos);
	float MaxDim = FMath::Max3(Scale.X, Scale.Y, Scale.Z) * 100.f;
	NL->SetIntensity(4000.f);
	NL->SetAttenuationRadius(FMath::Max(MaxDim * 2.f, 300.f));
	NL->SetLightColor(Color);
	NL->CastShadows = false;
	NL->RegisterComponent();
}
