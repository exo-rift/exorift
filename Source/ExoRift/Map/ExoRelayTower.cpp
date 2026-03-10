// ExoRelayTower.cpp — Tall communication relay landmark
#include "Map/ExoRelayTower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoRelayTower::AExoRelayTower()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubF.Succeeded()) CubeMesh = CubF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphF.Succeeded()) SphereMesh = SphF.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatF(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatF.Succeeded()) BaseMaterial = MatF.Object;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

UStaticMeshComponent* AExoRelayTower::AddSection(
	const FVector& Pos, const FVector& Scale, const FRotator& Rot,
	UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (!Mesh || !BaseMaterial) return nullptr;

	UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this);
	Part->SetupAttachment(RootComponent);
	Part->SetStaticMesh(Mesh);
	Part->SetRelativeLocation(Pos);
	Part->SetRelativeScale3D(Scale);
	Part->SetRelativeRotation(Rot);
	Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Part->SetCollisionResponseToAllChannels(ECR_Block);
	Part->CastShadow = true;
	Part->RegisterComponent();

	UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Part->SetMaterial(0, Mat);
	Parts.Add(Part);
	return Part;
}

void AExoRelayTower::BuildTower()
{
	FLinearColor Steel(0.06f, 0.06f, 0.08f);
	FLinearColor DarkSteel(0.04f, 0.04f, 0.05f);
	FLinearColor Accent(0.08f, 0.08f, 0.1f);

	// === BASE — heavy concrete/steel foundation ===
	AddSection(FVector(0.f, 0.f, 100.f), FVector(6.f, 6.f, 2.f),
		FRotator::ZeroRotator, CubeMesh, DarkSteel);

	// Foundation bevel ring
	AddSection(FVector(0.f, 0.f, 25.f), FVector(4.f, 4.f, 0.5f),
		FRotator::ZeroRotator, CylinderMesh, Steel);

	// === MAIN SHAFT — tall central column in 3 tapered segments ===
	// Lower shaft (widest)
	AddSection(FVector(0.f, 0.f, 1200.f), FVector(2.f, 2.f, 20.f),
		FRotator::ZeroRotator, CylinderMesh, Steel);

	// Mid shaft (narrower)
	AddSection(FVector(0.f, 0.f, 3200.f), FVector(1.5f, 1.5f, 18.f),
		FRotator::ZeroRotator, CylinderMesh, Accent);

	// Upper shaft (narrowest)
	AddSection(FVector(0.f, 0.f, 5000.f), FVector(1.f, 1.f, 15.f),
		FRotator::ZeroRotator, CylinderMesh, Steel);

	// === LATTICE BRACING — diagonal struts at each transition ===
	auto AddStrut = [&](const FVector& Base, float Yaw)
	{
		AddSection(Base,
			FVector(0.15f, 0.15f, 6.f),
			FRotator(25.f, Yaw, 0.f), CylinderMesh, DarkSteel);
	};
	for (float Y = 0.f; Y < 360.f; Y += 90.f)
	{
		AddStrut(FVector(0.f, 0.f, 500.f), Y);
		AddStrut(FVector(0.f, 0.f, 2200.f), Y + 45.f);
		AddStrut(FVector(0.f, 0.f, 4000.f), Y);
	}

	// === PLATFORM RINGS — walkable platforms at transitions ===
	AddSection(FVector(0.f, 0.f, 2200.f), FVector(3.5f, 3.5f, 0.15f),
		FRotator::ZeroRotator, CylinderMesh, Accent);
	AddSection(FVector(0.f, 0.f, 4200.f), FVector(2.5f, 2.5f, 0.15f),
		FRotator::ZeroRotator, CylinderMesh, Accent);

	// === DISH ASSEMBLY at top — rotating radar/comm dish ===
	// Dish support arm (horizontal)
	DishArm = AddSection(FVector(200.f, 0.f, 5800.f), FVector(4.f, 0.3f, 0.3f),
		FRotator::ZeroRotator, CubeMesh, Steel);

	// Dish head (flattened sphere = parabolic dish)
	DishHead = AddSection(FVector(500.f, 0.f, 5800.f), FVector(3.f, 3.f, 0.6f),
		FRotator(0.f, 0.f, 90.f), SphereMesh, Accent);

	// === ANTENNA SPIRE at very top ===
	AddSection(FVector(0.f, 0.f, 6300.f), FVector(0.1f, 0.1f, 8.f),
		FRotator::ZeroRotator, CylinderMesh, DarkSteel);

	// Antenna tip sphere
	AddSection(FVector(0.f, 0.f, 7100.f), FVector(0.3f, 0.3f, 0.3f),
		FRotator::ZeroRotator, SphereMesh, FLinearColor(0.8f, 0.2f, 0.1f));

	// === BEACON LIGHTS — blinking aviation lights ===
	auto AddBeacon = [&](const FVector& Pos, const FLinearColor& Color, float Intensity)
	{
		// Glowing bulb
		UStaticMeshComponent* Bulb = AddSection(Pos,
			FVector(0.15f, 0.15f, 0.15f), FRotator::ZeroRotator, SphereMesh, Color);
		if (Bulb)
		{
			UMaterialInstanceDynamic* BM = Cast<UMaterialInstanceDynamic>(Bulb->GetMaterial(0));
			if (BM) BM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 30.f, Color.G * 30.f, Color.B * 30.f));
		}

		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetRelativeLocation(Pos);
		Light->SetIntensity(Intensity);
		Light->SetAttenuationRadius(3000.f);
		Light->SetLightColor(Color);
		Light->CastShadows = false;
		Light->RegisterComponent();
		BeaconLights.Add(Light);
	};

	FLinearColor Red(1.f, 0.15f, 0.1f);
	FLinearColor White(0.9f, 0.9f, 1.f);

	// Top beacon (brightest)
	AddBeacon(FVector(0.f, 0.f, 7100.f), Red, 25000.f);
	// Mid-tower beacons
	AddBeacon(FVector(150.f, 0.f, 4200.f), Red, 12000.f);
	AddBeacon(FVector(-150.f, 0.f, 4200.f), Red, 12000.f);
	// Lower platform beacons
	AddBeacon(FVector(200.f, 0.f, 2200.f), White, 8000.f);
	AddBeacon(FVector(-200.f, 0.f, 2200.f), White, 8000.f);

	// === ENERGY CONDUIT along shaft — glowing cable ===
	if (CylinderMesh && BaseMaterial)
	{
		UStaticMeshComponent* Conduit = NewObject<UStaticMeshComponent>(this);
		Conduit->SetupAttachment(RootComponent);
		Conduit->SetStaticMesh(CylinderMesh);
		Conduit->SetRelativeLocation(FVector(120.f, 0.f, 3000.f));
		Conduit->SetRelativeScale3D(FVector(0.08f, 0.08f, 55.f));
		Conduit->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Conduit->CastShadow = false;
		Conduit->RegisterComponent();

		UMaterialInstanceDynamic* CM = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		FLinearColor ConduitCol(0.1f, 0.5f, 0.8f);
		CM->SetVectorParameterValue(TEXT("BaseColor"), ConduitCol);
		CM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(ConduitCol.R * 8.f, ConduitCol.G * 8.f, ConduitCol.B * 8.f));
		Conduit->SetMaterial(0, CM);
	}
}

void AExoRelayTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Rotate dish slowly
	DishAngle += DeltaTime * 12.f; // 12 deg/sec
	if (DishArm)
	{
		FVector ArmPos = DishArm->GetRelativeLocation();
		float R = 200.f;
		DishArm->SetRelativeLocation(FVector(
			R * FMath::Cos(FMath::DegreesToRadians(DishAngle)),
			R * FMath::Sin(FMath::DegreesToRadians(DishAngle)),
			ArmPos.Z));
		DishArm->SetRelativeRotation(FRotator(0.f, DishAngle, 0.f));
	}
	if (DishHead)
	{
		float R = 500.f;
		DishHead->SetRelativeLocation(FVector(
			R * FMath::Cos(FMath::DegreesToRadians(DishAngle)),
			R * FMath::Sin(FMath::DegreesToRadians(DishAngle)),
			5800.f));
		DishHead->SetRelativeRotation(FRotator(0.f, DishAngle, 90.f));
	}

	// Blink beacon lights — alternating 1s on / 1s off pattern
	for (int32 i = 0; i < BeaconLights.Num(); i++)
	{
		if (!BeaconLights[i]) continue;
		float Phase = Time + i * 0.4f; // Staggered blink
		bool bOn = FMath::Fmod(Phase, 2.f) < 1.2f;
		float Base = (i == 0) ? 25000.f : ((i < 3) ? 12000.f : 8000.f);
		BeaconLights[i]->SetIntensity(bOn ? Base : Base * 0.05f);
	}
}
