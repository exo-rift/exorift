// ExoReactorCore.cpp — Animated energy reactor centerpiece with rotating rings
#include "Map/ExoReactorCore.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoReactorCore::AExoReactorCore()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphF.Succeeded()) SphereMesh = SphF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubF.Succeeded()) CubeMesh = CubF.Object;

	// Materials created at runtime via FExoMaterialFactory

	auto MakeMesh = [&](const TCHAR* Name, UStaticMesh* Mesh) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Mesh) C->SetStaticMesh(Mesh);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		C->SetGenerateOverlapEvents(false);
		return C;
	};

	// Base platform
	BasePlate = MakeMesh(TEXT("Base"), CylinderMesh);
	RootComponent = BasePlate;
	BasePlate->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BasePlate->SetCollisionResponseToAllChannels(ECR_Block);

	// Central pillar
	Pillar = MakeMesh(TEXT("Pillar"), CylinderMesh);
	Pillar->SetupAttachment(BasePlate);

	// Glowing energy core
	CoreSphere = MakeMesh(TEXT("Core"), SphereMesh);
	CoreSphere->SetupAttachment(BasePlate);

	// Rotating rings
	InnerRing = MakeMesh(TEXT("InnerRing"), CylinderMesh);
	InnerRing->SetupAttachment(BasePlate);

	OuterRing = MakeMesh(TEXT("OuterRing"), CylinderMesh);
	OuterRing->SetupAttachment(BasePlate);

	// Energy conduit beams
	for (int32 i = 0; i < NUM_CONDUITS; i++)
	{
		FName Name = *FString::Printf(TEXT("Conduit_%d"), i);
		UStaticMeshComponent* C = MakeMesh(*Name.ToString(), CubeMesh);
		C->SetupAttachment(BasePlate);
		Conduits.Add(C);
	}

	// Lights
	CoreLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("CoreLight"));
	CoreLight->SetupAttachment(BasePlate);
	CoreLight->SetIntensity(8000.f);
	CoreLight->SetAttenuationRadius(4000.f);
	CoreLight->SetLightColor(FLinearColor(0.2f, 0.5f, 1.f));
	CoreLight->CastShadows = false;

	AmbientLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AmbientLight"));
	AmbientLight->SetupAttachment(BasePlate);
	AmbientLight->SetIntensity(3000.f);
	AmbientLight->SetAttenuationRadius(6000.f);
	AmbientLight->SetLightColor(FLinearColor(0.15f, 0.3f, 0.7f));
	AmbientLight->CastShadows = false;
}

void AExoReactorCore::InitReactor()
{
	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	if (!LitMat) return;

	auto MakeStructMat = [&](const FLinearColor& Base) -> UMaterialInstanceDynamic*
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(LitMat, this);
		if (!M) { return nullptr; }
		M->SetVectorParameterValue(TEXT("BaseColor"), Base);
		M->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
		M->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
		M->SetScalarParameterValue(TEXT("Roughness"), 0.2f);
		return M;
	};

	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	auto MakeEmissiveMat = [&](const FLinearColor& Emissive) -> UMaterialInstanceDynamic*
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(EmissiveMat, this);
		if (!M) { return nullptr; }
		M->SetVectorParameterValue(TEXT("EmissiveColor"), Emissive);
		return M;
	};

	// Base plate — dark metallic platform
	BasePlate->SetRelativeScale3D(FVector(30.f, 30.f, 1.f));
	BasePlate->SetMaterial(0, MakeStructMat(FLinearColor(0.04f, 0.04f, 0.06f)));

	// Central pillar
	Pillar->SetRelativeLocation(FVector(0.f, 0.f, 1500.f));
	Pillar->SetRelativeScale3D(FVector(4.f, 4.f, 30.f));
	Pillar->SetMaterial(0, MakeStructMat(FLinearColor(0.06f, 0.06f, 0.08f)));

	// Core sphere — bright pulsing energy ball
	CoreSphere->SetRelativeLocation(FVector(0.f, 0.f, 3500.f));
	CoreSphere->SetRelativeScale3D(FVector(8.f));
	CoreMat = MakeEmissiveMat(FLinearColor(25.f, 45.f, 90.f));
	CoreSphere->SetMaterial(0, CoreMat);

	// Inner ring — tilted, orbiting close to core
	InnerRing->SetRelativeLocation(FVector(0.f, 0.f, 3500.f));
	InnerRing->SetRelativeScale3D(FVector(14.f, 14.f, 0.4f));
	InnerRingMat = MakeEmissiveMat(FLinearColor(6.f, 12.f, 30.f));
	InnerRing->SetMaterial(0, InnerRingMat);

	// Outer ring — opposite tilt, wider orbit
	OuterRing->SetRelativeLocation(FVector(0.f, 0.f, 3500.f));
	OuterRing->SetRelativeScale3D(FVector(20.f, 20.f, 0.3f));
	OuterRingMat = MakeEmissiveMat(FLinearColor(4.f, 8.f, 20.f));
	OuterRing->SetMaterial(0, OuterRingMat);

	// Energy conduit beams — vertical beams around the core
	ConduitMats.SetNum(NUM_CONDUITS);
	for (int32 i = 0; i < NUM_CONDUITS; i++)
	{
		float Angle = (2.f * PI * i) / NUM_CONDUITS;
		float Dist = 800.f;
		FVector Pos(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 2000.f);
		Conduits[i]->SetRelativeLocation(Pos);
		Conduits[i]->SetRelativeScale3D(FVector(0.3f, 0.3f, 40.f));

		ConduitMats[i] = MakeEmissiveMat(FLinearColor(10.f, 20.f, 50.f));
		Conduits[i]->SetMaterial(0, ConduitMats[i]);
	}

	// Position lights
	CoreLight->SetRelativeLocation(FVector(0.f, 0.f, 3500.f));
	AmbientLight->SetRelativeLocation(FVector(0.f, 0.f, 2000.f));
}

void AExoReactorCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Core sphere pulsing — slow breathe with high-freq shimmer
	if (CoreMat)
	{
		float Pulse = 1.f + 0.3f * FMath::Sin(Time * 1.5f)
			+ 0.1f * FMath::Sin(Time * 7.f);
		float Scale = 8.f * (0.95f + 0.05f * FMath::Sin(Time * 2.f));
		CoreSphere->SetRelativeScale3D(FVector(Scale));
		CoreMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(5.f * Pulse, 10.f * Pulse, 20.f * Pulse));
	}

	// Inner ring rotation — tilted spin
	{
		float RotSpeed = 25.f;
		FRotator Rot(30.f, Time * RotSpeed, 0.f);
		InnerRing->SetRelativeRotation(Rot);

		if (InnerRingMat)
		{
			float P = 1.f + 0.4f * FMath::Sin(Time * 3.f);
			InnerRingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(6.f * P, 12.f * P, 30.f * P));
		}
	}

	// Outer ring rotation — opposite direction, different tilt
	{
		float RotSpeed = -15.f;
		FRotator Rot(-20.f, Time * RotSpeed, 15.f);
		OuterRing->SetRelativeRotation(Rot);

		if (OuterRingMat)
		{
			float P = 1.f + 0.3f * FMath::Sin(Time * 2.f + 1.f);
			OuterRingMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(1.f * P, 2.f * P, 5.f * P));
		}
	}

	// Conduit beams pulse and shimmer
	for (int32 i = 0; i < NUM_CONDUITS; i++)
	{
		if (!ConduitMats.IsValidIndex(i) || !ConduitMats[i]) continue;
		float Phase = Time * 4.f + i * 1.5f;
		float P = 1.f + 0.5f * FMath::Sin(Phase);
		ConduitMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(2.f * P, 4.f * P, 10.f * P));
	}

	// Core light flicker
	{
		float LP = 1.f + 0.15f * FMath::Sin(Time * 2.f)
			+ 0.05f * FMath::Sin(Time * 11.f);
		CoreLight->SetIntensity(60000.f * LP);
	}
}
