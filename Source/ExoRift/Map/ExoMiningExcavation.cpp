// ExoMiningExcavation.cpp — Open-pit mining site with mineral veins
#include "Map/ExoMiningExcavation.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMiningExcavation::AExoMiningExcavation()
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

UStaticMeshComponent* AExoMiningExcavation::AddPart(
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
	SiteParts.Add(Part);
	return Part;
}

void AExoMiningExcavation::BuildSite()
{
	FLinearColor Rock(0.045f, 0.04f, 0.035f);
	FLinearColor DarkRock(0.03f, 0.03f, 0.025f);
	FLinearColor Metal(0.06f, 0.06f, 0.08f);
	FLinearColor RustMetal(0.08f, 0.05f, 0.03f);

	// === PIT FLOOR — sunken area below ground level ===
	AddPart(FVector(0.f, 0.f, -400.f), FVector(40.f, 40.f, 0.5f),
		FRotator::ZeroRotator, CubeMesh, DarkRock);

	// === TERRACED WALLS — stepped rock walls around the pit ===
	struct FWallSeg { FVector Pos; FVector Scale; FRotator Rot; };
	FWallSeg Walls[] = {
		// North wall tiers
		{{0.f, 2000.f, -200.f}, {40.f, 5.f, 4.f}, {-10.f, 0.f, 0.f}},
		{{0.f, 1600.f, -350.f}, {35.f, 3.f, 3.f}, {-5.f, 0.f, 0.f}},
		// South wall tiers
		{{0.f, -2000.f, -200.f}, {40.f, 5.f, 4.f}, {10.f, 0.f, 0.f}},
		{{0.f, -1600.f, -350.f}, {35.f, 3.f, 3.f}, {5.f, 0.f, 0.f}},
		// East wall
		{{2000.f, 0.f, -200.f}, {5.f, 40.f, 4.f}, {0.f, 0.f, 8.f}},
		{{1600.f, 0.f, -350.f}, {3.f, 35.f, 3.f}, {0.f, 0.f, 4.f}},
		// West wall (partially collapsed — ramp access)
		{{-1800.f, 0.f, -200.f}, {5.f, 30.f, 3.f}, {0.f, 0.f, -12.f}},
	};
	for (const auto& W : Walls)
		AddPart(W.Pos, W.Scale, W.Rot, CubeMesh, Rock);

	// Access ramp (west side)
	AddPart(FVector(-2500.f, 0.f, -200.f), FVector(15.f, 8.f, 0.3f),
		FRotator(0.f, 0.f, -12.f), CubeMesh, DarkRock);

	// === EXPOSED MINERAL VEINS — glowing crystal formations ===
	FLinearColor CrystalPurple(0.4f, 0.1f, 0.8f);
	FLinearColor CrystalTeal(0.05f, 0.6f, 0.5f);
	FLinearColor CrystalGold(0.7f, 0.5f, 0.1f);

	struct FVeinDef { FVector Pos; FVector Scale; FRotator Rot; FLinearColor Color; };
	FVeinDef Veins[] = {
		// North wall — purple cluster
		{{200.f, 1700.f, -100.f}, {1.f, 0.5f, 2.f}, {-15.f, 10.f, 0.f}, CrystalPurple},
		{{-300.f, 1800.f, -150.f}, {0.6f, 0.4f, 1.5f}, {-20.f, -5.f, 10.f}, CrystalPurple},
		{{100.f, 1650.f, -200.f}, {0.8f, 0.3f, 1.8f}, {-10.f, 15.f, -5.f}, CrystalPurple},
		// East wall — teal vein
		{{1700.f, 400.f, -250.f}, {0.5f, 1.2f, 1.5f}, {5.f, 0.f, 12.f}, CrystalTeal},
		{{1800.f, -200.f, -180.f}, {0.4f, 0.8f, 2.f}, {0.f, -8.f, 15.f}, CrystalTeal},
		// Pit floor — gold deposit
		{{-400.f, -300.f, -380.f}, {1.5f, 1.f, 0.6f}, {3.f, 25.f, 0.f}, CrystalGold},
		{{200.f, -500.f, -370.f}, {1.f, 0.8f, 0.8f}, {-5.f, 40.f, 5.f}, CrystalGold},
	};

	for (const auto& V : Veins)
	{
		UStaticMeshComponent* Crystal = AddPart(V.Pos, V.Scale, V.Rot, CubeMesh, V.Color);
		if (Crystal)
		{
			Crystal->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			UMaterialInstanceDynamic* CM = Cast<UMaterialInstanceDynamic>(Crystal->GetMaterial(0));
			if (CM)
			{
				CM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(V.Color.R * 12.f, V.Color.G * 12.f, V.Color.B * 12.f));
				MineralMats.Add(CM);
			}
		}

		UPointLightComponent* GL = NewObject<UPointLightComponent>(this);
		GL->SetupAttachment(RootComponent);
		GL->SetRelativeLocation(V.Pos);
		GL->SetIntensity(5000.f);
		GL->SetAttenuationRadius(800.f);
		GL->SetLightColor(V.Color);
		GL->CastShadows = false;
		GL->RegisterComponent();
		MineralLights.Add(GL);
	}

	// === DRILLING EQUIPMENT — crane-like structure ===
	// Derrick tower
	AddPart(FVector(500.f, 500.f, -100.f), FVector(0.5f, 0.5f, 6.f),
		FRotator::ZeroRotator, CylinderMesh, Metal);
	AddPart(FVector(500.f, 500.f, -100.f), FVector(0.3f, 0.3f, 6.f),
		FRotator(8.f, 0.f, 0.f), CylinderMesh, Metal);
	// Derrick arm
	AddPart(FVector(500.f, 500.f, 200.f), FVector(0.2f, 0.2f, 5.f),
		FRotator(0.f, 0.f, 60.f), CylinderMesh, RustMetal);
	// Drill head
	AddPart(FVector(500.f, 200.f, -350.f), FVector(0.4f, 0.4f, 1.5f),
		FRotator::ZeroRotator, CylinderMesh, Metal);

	// === CONVEYOR BELTS — angled transport lines ===
	AddPart(FVector(-800.f, 800.f, -200.f), FVector(0.8f, 15.f, 0.1f),
		FRotator(0.f, -30.f, 15.f), CubeMesh, RustMetal);
	// Conveyor supports
	for (int32 i = 0; i < 4; i++)
	{
		float T = i / 3.f;
		FVector SupPos = FMath::Lerp(
			FVector(-1200.f, 1600.f, -300.f),
			FVector(-400.f, 0.f, -100.f), T);
		AddPart(SupPos, FVector(0.15f, 0.15f, 1.5f + T * 2.f),
			FRotator::ZeroRotator, CylinderMesh, Metal);
	}

	// === ORE PILES — scattered extraction material ===
	FVector PilePositions[] = {
		{-600.f, -800.f, -380.f}, {800.f, -600.f, -380.f},
		{-200.f, 1000.f, -380.f}, {1000.f, 800.f, -380.f},
	};
	for (int32 i = 0; i < 4; i++)
	{
		float S = 1.5f + (i % 3) * 0.8f;
		AddPart(PilePositions[i],
			FVector(S, S * 0.8f, S * 0.4f),
			FRotator(0.f, i * 37.f, 0.f), SphereMesh, DarkRock);
	}

	// === WORK LIGHTS — industrial floods illuminating the pit ===
	FLinearColor WorkLight(0.9f, 0.85f, 0.7f);
	FVector FloodPositions[] = {
		{1500.f, 1500.f, 200.f}, {-1500.f, 1500.f, 200.f},
		{1500.f, -1500.f, 200.f}, {-1500.f, -1500.f, 200.f},
	};
	for (const FVector& FP : FloodPositions)
	{
		AddPart(FP, FVector(0.15f, 0.15f, 4.f),
			FRotator::ZeroRotator, CylinderMesh, Metal);

		UPointLightComponent* FL = NewObject<UPointLightComponent>(this);
		FL->SetupAttachment(RootComponent);
		FL->SetRelativeLocation(FP + FVector(0.f, 0.f, 200.f));
		FL->SetIntensity(15000.f);
		FL->SetAttenuationRadius(3000.f);
		FL->SetLightColor(WorkLight);
		FL->CastShadows = false;
		FL->RegisterComponent();
	}
}

void AExoMiningExcavation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Crystal veins pulse with a slow shimmer
	for (int32 i = 0; i < MineralMats.Num(); i++)
	{
		if (!MineralMats[i]) continue;
		float Phase = Time * 1.2f + i * 0.9f;
		float Pulse = 0.6f + 0.4f * FMath::Sin(Phase);

		FLinearColor Base;
		MineralMats[i]->GetVectorParameterValue(TEXT("BaseColor"), Base);
		MineralMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Base.R * 12.f * Pulse, Base.G * 12.f * Pulse,
				Base.B * 12.f * Pulse));

		if (MineralLights.IsValidIndex(i) && MineralLights[i])
			MineralLights[i]->SetIntensity(5000.f * Pulse);
	}
}
