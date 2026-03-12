// ExoShowcaseRoom.cpp — Photo-realistic PBR showcase room
#include "Map/ExoShowcaseRoom.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AExoShowcaseRoom::AExoShowcaseRoom()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeF(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeF.Succeeded()) CubeMesh = CubeF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereF(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereF.Succeeded()) SphereMesh = SphereF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylF(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylF.Succeeded()) CylinderMesh = CylF.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeF(
		TEXT("/Engine/BasicShapes/Cone"));
	if (ConeF.Succeeded()) ConeMesh = ConeF.Object;
}

void AExoShowcaseRoom::BeginPlay()
{
	Super::BeginPlay();
	if (!CubeMesh) return;

	BuildStructure();
	BuildPanels();
	BuildDetails();
	BuildDisplayObjects();
	BuildLighting();
}

void AExoShowcaseRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bPlayerTeleported) { SetActorTickEnabled(false); return; }

	TeleportDelay += DeltaTime;
	if (TeleportDelay < 3.f) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->GetPawn()) return;

	FVector RoomCenter = GetActorLocation();
	PC->GetPawn()->TeleportTo(
		RoomCenter + FVector(0.f, -150.f, 50.f), FRotator(0.f, 90.f, 0.f));
	bPlayerTeleported = true;
}

// --- Mesh Helpers ---

UStaticMeshComponent* AExoShowcaseRoom::Box(FVector Pos, FVector Size,
	UMaterialInstanceDynamic* Mat, bool bColl)
{
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(CubeMesh);
	C->SetWorldLocation(GetActorLocation() + Pos);
	C->SetWorldScale3D(Size / 100.f);
	if (Mat) C->SetMaterial(0, Mat);
	if (bColl)
	{
		C->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		C->SetCollisionResponseToAllChannels(ECR_Block);
	}
	else
	{
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	C->CastShadow = true;
	C->RegisterComponent();
	return C;
}

UStaticMeshComponent* AExoShowcaseRoom::Sphere(FVector Pos, float Radius,
	UMaterialInstanceDynamic* Mat)
{
	if (!SphereMesh) return nullptr;
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(SphereMesh);
	C->SetWorldLocation(GetActorLocation() + Pos);
	float S = Radius / 50.f; // Engine sphere has 50cm radius at scale 1
	C->SetWorldScale3D(FVector(S));
	if (Mat) C->SetMaterial(0, Mat);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->CastShadow = true;
	C->RegisterComponent();
	return C;
}

UStaticMeshComponent* AExoShowcaseRoom::Cylinder(FVector Pos, float Radius,
	float Height, UMaterialInstanceDynamic* Mat, bool bColl)
{
	if (!CylinderMesh) return nullptr;
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(CylinderMesh);
	C->SetWorldLocation(GetActorLocation() + Pos);
	float RS = Radius / 50.f;  // Engine cylinder is 50cm radius
	float HS = Height / 100.f; // Engine cylinder is 100cm tall
	C->SetWorldScale3D(FVector(RS, RS, HS));
	if (Mat) C->SetMaterial(0, Mat);
	C->SetCollisionEnabled(bColl ? ECollisionEnabled::QueryAndPhysics
		: ECollisionEnabled::NoCollision);
	if (bColl) C->SetCollisionResponseToAllChannels(ECR_Block);
	C->CastShadow = true;
	C->RegisterComponent();
	return C;
}

UStaticMeshComponent* AExoShowcaseRoom::Cone(FVector Pos, float Radius,
	float Height, UMaterialInstanceDynamic* Mat)
{
	if (!ConeMesh) return nullptr;
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(ConeMesh);
	C->SetWorldLocation(GetActorLocation() + Pos);
	float RS = Radius / 50.f;
	float HS = Height / 100.f;
	C->SetWorldScale3D(FVector(RS, RS, HS));
	if (Mat) C->SetMaterial(0, Mat);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->CastShadow = true;
	C->RegisterComponent();
	return C;
}

UMaterialInstanceDynamic* AExoShowcaseRoom::PBR(FLinearColor Color, float Met,
	float Rough, float Spec, bool bMetal)
{
	UMaterialInterface* Base = FExoMaterialFactory::GetLitEmissive();
	if (!Base) return nullptr;
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
	if (!MID) return nullptr;
	MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
	MID->SetScalarParameterValue(TEXT("Metallic"), Met);
	MID->SetScalarParameterValue(TEXT("Roughness"), Rough);
	MID->SetScalarParameterValue(TEXT("Specular"), Spec);
	MID->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.f, 0.f, 0.f));
	return MID;
}

UMaterialInstanceDynamic* AExoShowcaseRoom::Glow(FLinearColor Color)
{
	UMaterialInterface* Base = FExoMaterialFactory::GetLitEmissive();
	if (!Base) return nullptr;
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this);
	if (!MID) return nullptr;
	MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	MID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.02f, 0.02f, 0.02f));
	MID->SetScalarParameterValue(TEXT("Metallic"), 0.f);
	MID->SetScalarParameterValue(TEXT("Roughness"), 0.9f);
	return MID;
}

// --- Room Structure ---

void AExoShowcaseRoom::BuildStructure()
{
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f, WT = WallThick;

	auto* FloorMat = PBR(FLinearColor(0.12f, 0.11f, 0.10f), 0.02f, 0.88f, 0.35f);
	auto* WallMat = PBR(FLinearColor(0.20f, 0.21f, 0.23f), 0.55f, 0.42f, 0.5f);
	auto* CeilMat = PBR(FLinearColor(0.10f, 0.10f, 0.12f), 0.15f, 0.72f, 0.4f);

	// Floor
	Box(FVector(0, 0, -WT * 0.5f), FVector(RoomW + WT * 2, RoomD + WT * 2, WT), FloorMat);

	// Polished floor center for Lumen specular reflections
	auto* PolFloor = PBR(FLinearColor(0.08f, 0.08f, 0.09f), 0.05f, 0.25f, 0.7f);
	Box(FVector(0, 0, 0.3f), FVector(300.f, 300.f, 0.5f), PolFloor, false);

	// Ceiling
	Box(FVector(0, 0, RoomH + WT * 0.5f), FVector(RoomW + WT * 2, RoomD + WT * 2, WT), CeilMat);

	// 4 walls (south split for doorway)
	Box(FVector(0, HD + WT * 0.5f, RoomH * 0.5f), FVector(RoomW + WT * 2, WT, RoomH), WallMat);
	Box(FVector(HW + WT * 0.5f, 0, RoomH * 0.5f), FVector(WT, RoomD, RoomH), WallMat);
	Box(FVector(-HW - WT * 0.5f, 0, RoomH * 0.5f), FVector(WT, RoomD, RoomH), WallMat);

	float DoorW = 120.f, DoorH = 240.f;
	float SideW = (RoomW - DoorW) * 0.5f;
	Box(FVector(-(SideW + DoorW) * 0.5f, -HD - WT * 0.5f, RoomH * 0.5f),
		FVector(SideW, WT, RoomH), WallMat);
	Box(FVector((SideW + DoorW) * 0.5f, -HD - WT * 0.5f, RoomH * 0.5f),
		FVector(SideW, WT, RoomH), WallMat);
	Box(FVector(0, -HD - WT * 0.5f, DoorH + (RoomH - DoorH) * 0.5f),
		FVector(DoorW + 10.f, WT, RoomH - DoorH), WallMat);
}

// --- Wall Panels, Trim & Beams ---

void AExoShowcaseRoom::BuildPanels()
{
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f;

	auto* PanelMat = PBR(FLinearColor(0.14f, 0.15f, 0.18f), 0.65f, 0.32f, 0.55f);
	auto* TrimMat = PBR(FLinearColor(0.40f, 0.42f, 0.45f), 0.90f, 0.15f, 0.7f);
	auto* BorderMat = PBR(FLinearColor(0.25f, 0.26f, 0.28f), 0.80f, 0.25f, 0.6f);
	auto* DoorMat = PBR(FLinearColor(0.38f, 0.40f, 0.44f), 0.88f, 0.18f, 0.65f);
	auto* BeamMat = PBR(FLinearColor(0.28f, 0.28f, 0.30f), 0.75f, 0.28f, 0.55f);

	float Inset = 3.f, PanW = 160.f, PanH = 200.f;

	// Recessed panels on all 4 walls
	for (int32 S = 0; S < 2; S++)
	{
		float YS = S == 0 ? 1.f : -1.f;
		float WY = YS * HD;
		for (int32 P = 0; P < 2; P++)
		{
			float PX = (P == 0 ? -0.25f : 0.25f) * RoomW;
			Box(FVector(PX, WY - YS * Inset, 100.f + PanH * 0.5f),
				FVector(PanW, 2.f, PanH), PanelMat, false);
		}
		float TW = (S == 0) ? RoomW - 20.f : RoomW * 0.3f;
		Box(FVector(0, WY - YS * 1.f, 120.f), FVector(TW, 4.f, 4.f), TrimMat, false);
	}
	for (int32 S = 0; S < 2; S++)
	{
		float XS = S == 0 ? 1.f : -1.f;
		float WX = XS * HW;
		for (int32 P = 0; P < 2; P++)
		{
			float PY = (P == 0 ? -0.25f : 0.25f) * RoomD;
			Box(FVector(WX - XS * Inset, PY, 100.f + PanH * 0.5f),
				FVector(2.f, PanW, PanH), PanelMat, false);
		}
		Box(FVector(WX - XS * 1.f, 0, 120.f), FVector(4.f, RoomD - 20.f, 4.f), TrimMat, false);
	}

	// Floor border inlay
	float BW = 12.f, BH = 1.5f;
	Box(FVector(0, HD - BW * 0.5f, BH * 0.5f), FVector(RoomW, BW, BH), BorderMat, false);
	Box(FVector(0, -HD + BW * 0.5f, BH * 0.5f), FVector(RoomW, BW, BH), BorderMat, false);
	Box(FVector(HW - BW * 0.5f, 0, BH * 0.5f), FVector(BW, RoomD, BH), BorderMat, false);
	Box(FVector(-HW + BW * 0.5f, 0, BH * 0.5f), FVector(BW, RoomD, BH), BorderMat, false);

	// Ceiling cross-beams
	Box(FVector(0, 0, RoomH - 8.f), FVector(RoomW - 10.f, 18.f, 18.f), BeamMat, false);
	Box(FVector(0, 0, RoomH - 8.f), FVector(18.f, RoomD - 10.f, 18.f), BeamMat, false);

	// Corner pillars — cylindrical for visual variety
	for (int32 CX = -1; CX <= 1; CX += 2)
		for (int32 CY = -1; CY <= 1; CY += 2)
			Cylinder(FVector(CX * (HW - 8.f), CY * (HD - 8.f), RoomH * 0.5f),
				8.f, RoomH, TrimMat);

	// Door frame
	float DoorW = 120.f, DoorH = 240.f;
	Box(FVector(-DoorW * 0.5f - 3.f, -HD + 1.f, DoorH * 0.5f),
		FVector(6.f, 4.f, DoorH), DoorMat, false);
	Box(FVector(DoorW * 0.5f + 3.f, -HD + 1.f, DoorH * 0.5f),
		FVector(6.f, 4.f, DoorH), DoorMat, false);
	Box(FVector(0, -HD + 1.f, DoorH + 3.f), FVector(DoorW + 12.f, 4.f, 6.f), DoorMat, false);
}

// --- Props, Emissive Accents & Conduits ---

void AExoShowcaseRoom::BuildDetails()
{
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f;

	auto* ConsoleMat = PBR(FLinearColor(0.06f, 0.06f, 0.08f), 0.35f, 0.55f, 0.45f);
	auto* PipeMat = PBR(FLinearColor(0.35f, 0.35f, 0.37f), 0.85f, 0.20f, 0.6f);
	auto* ScreenGlow = Glow(FLinearColor(0.15f, 0.3f, 0.6f));
	auto* WarmAccent = Glow(FLinearColor(0.8f, 0.5f, 0.2f));
	auto* CoolAccent = Glow(FLinearColor(0.1f, 0.2f, 0.5f));

	// Console desk against north wall
	float DkW = 140.f, DkD = 55.f, DkH = 80.f;
	float DkY = HD - DkD * 0.5f - 20.f;
	Box(FVector(0, DkY, DkH), FVector(DkW, DkD, 4.f), ConsoleMat, false);
	// Desk legs — cylinders
	Cylinder(FVector(-DkW * 0.38f, DkY, DkH * 0.5f), 4.f, DkH - 4.f, ConsoleMat);
	Cylinder(FVector(DkW * 0.38f, DkY, DkH * 0.5f), 4.f, DkH - 4.f, ConsoleMat);

	// Screen (emissive panel)
	Box(FVector(0, DkY + DkD * 0.3f, DkH + 35.f), FVector(90.f, 2.f, 55.f), ScreenGlow, false);

	// Emissive accent strips at 120cm height
	float SZ = 120.f;
	Box(FVector(0, HD - 1.f, SZ), FVector(RoomW * 0.6f, 1.f, 2.f), WarmAccent, false);
	Box(FVector(0, -HD + 1.f, SZ), FVector(RoomW * 0.3f, 1.f, 2.f), WarmAccent, false);
	Box(FVector(HW - 1.f, 0, SZ), FVector(1.f, RoomD * 0.6f, 2.f), WarmAccent, false);
	Box(FVector(-HW + 1.f, 0, SZ), FVector(1.f, RoomD * 0.6f, 2.f), WarmAccent, false);
	Box(FVector(0, -HD + 1.f, 243.f), FVector(130.f, 1.f, 2.f), CoolAccent, false);

	// Ceiling conduit pipes
	Cylinder(FVector(0, HD - 25.f, RoomH - 20.f), 6.f, RoomW * 0.8f, PipeMat);
	// Rotate horizontal: the cylinder helper places it vertical, so use a box for horizontal
	Box(FVector(0, HD - 45.f, RoomH - 15.f), FVector(RoomW * 0.6f, 8.f, 8.f), PipeMat, false);

	// Supply crate in SW corner
	auto* CrateMat = PBR(FLinearColor(0.18f, 0.16f, 0.12f), 0.10f, 0.78f, 0.35f);
	Box(FVector(-HW + 45.f, -HD + 45.f, 22.f), FVector(44.f, 44.f, 44.f), CrateMat, false);

	// Floor vent grating
	auto* VentMat = PBR(FLinearColor(0.08f, 0.08f, 0.08f), 0.70f, 0.35f, 0.5f);
	Box(FVector(90.f, -70.f, 0.5f), FVector(65.f, 45.f, 1.f), VentMat, false);
}

// --- Display Objects (curved geometry for PBR showcase) ---

void AExoShowcaseRoom::BuildDisplayObjects()
{
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f;

	// === Center pedestal with chrome sphere ===
	// Pedestal base (dark polished stone)
	auto* PedestalMat = PBR(FLinearColor(0.05f, 0.05f, 0.06f), 0.02f, 0.30f, 0.6f);
	Cylinder(FVector(0, 0, 15.f), 30.f, 30.f, PedestalMat, true);
	// Pedestal column
	Cylinder(FVector(0, 0, 50.f), 12.f, 40.f, PedestalMat);
	// Pedestal top plate
	Cylinder(FVector(0, 0, 72.f), 25.f, 4.f, PedestalMat);

	// Chrome sphere — near-perfect mirror, showcases Lumen reflections
	auto* ChromeMat = PBR(FLinearColor(0.95f, 0.93f, 0.88f), 1.0f, 0.02f, 1.0f);
	Sphere(FVector(0, 0, 100.f), 26.f, ChromeMat);

	// === East wall: material sample spheres on shelf ===
	float ShelfY = 0.f;
	float ShelfX = HW - 30.f;
	float ShelfZ = 110.f;
	auto* ShelfMat = PBR(FLinearColor(0.15f, 0.15f, 0.17f), 0.80f, 0.25f, 0.5f);
	Box(FVector(ShelfX, ShelfY, ShelfZ), FVector(50.f, 200.f, 4.f), ShelfMat, false);
	// Shelf bracket
	Box(FVector(ShelfX + 2.f, ShelfY - 70.f, ShelfZ - 15.f), FVector(4.f, 4.f, 26.f), ShelfMat, false);
	Box(FVector(ShelfX + 2.f, ShelfY + 70.f, ShelfZ - 15.f), FVector(4.f, 4.f, 26.f), ShelfMat, false);

	// 4 material sample spheres: copper, gold, rubber, ceramic
	auto* CopperMat = PBR(FLinearColor(0.95f, 0.64f, 0.54f), 1.0f, 0.25f, 0.7f);
	auto* GoldMat = PBR(FLinearColor(1.0f, 0.76f, 0.33f), 1.0f, 0.15f, 0.8f);
	auto* RubberMat = PBR(FLinearColor(0.08f, 0.08f, 0.08f), 0.0f, 0.95f, 0.3f);
	auto* CeramicMat = PBR(FLinearColor(0.85f, 0.85f, 0.80f), 0.0f, 0.45f, 0.5f);

	float SphR = 14.f;
	float SphZ = ShelfZ + 2.f + SphR;
	Sphere(FVector(ShelfX, ShelfY - 60.f, SphZ), SphR, CopperMat);
	Sphere(FVector(ShelfX, ShelfY - 20.f, SphZ), SphR, GoldMat);
	Sphere(FVector(ShelfX, ShelfY + 20.f, SphZ), SphR, RubberMat);
	Sphere(FVector(ShelfX, ShelfY + 60.f, SphZ), SphR, CeramicMat);

	// === West wall: cylindrical vases / containers ===
	auto* DarkMetalMat = PBR(FLinearColor(0.10f, 0.10f, 0.12f), 0.85f, 0.20f, 0.6f);
	auto* BrushedMat = PBR(FLinearColor(0.30f, 0.32f, 0.35f), 0.70f, 0.40f, 0.5f);
	auto* TintedMat = PBR(FLinearColor(0.15f, 0.25f, 0.35f), 0.60f, 0.30f, 0.5f);

	// Tall vase
	Cylinder(FVector(-HW + 40.f, HD - 50.f, 40.f), 12.f, 80.f, DarkMetalMat);
	// Medium container
	Cylinder(FVector(-HW + 40.f, HD - 90.f, 25.f), 16.f, 50.f, BrushedMat);
	// Cone cap on medium container
	Cone(FVector(-HW + 40.f, HD - 90.f, 55.f), 16.f, 20.f, TintedMat);

	// Short wide cylinder (canister)
	Cylinder(FVector(-HW + 40.f, 0, 18.f), 20.f, 36.f, TintedMat);

	// === NE corner: decorative cone sculpture ===
	auto* SculptureMat = PBR(FLinearColor(0.60f, 0.55f, 0.45f), 0.95f, 0.08f, 0.9f);
	// Base ring
	Cylinder(FVector(HW - 60.f, HD - 60.f, 5.f), 22.f, 10.f, SculptureMat);
	// Cone
	Cone(FVector(HW - 60.f, HD - 60.f, 45.f), 18.f, 70.f, SculptureMat);
	// Small sphere on top
	Sphere(FVector(HW - 60.f, HD - 60.f, 85.f), 8.f, SculptureMat);

	// === Floor details: scattered small objects ===
	// Cylindrical bollards near door
	auto* BollardMat = PBR(FLinearColor(0.25f, 0.25f, 0.27f), 0.80f, 0.30f, 0.5f);
	Cylinder(FVector(-70.f, -HD + 30.f, 25.f), 6.f, 50.f, BollardMat, true);
	Cylinder(FVector(70.f, -HD + 30.f, 25.f), 6.f, 50.f, BollardMat, true);
	// Emissive caps on bollards
	auto* BollardGlow = Glow(FLinearColor(0.4f, 0.6f, 1.0f));
	Sphere(FVector(-70.f, -HD + 30.f, 52.f), 6.5f, BollardGlow);
	Sphere(FVector(70.f, -HD + 30.f, 52.f), 6.5f, BollardGlow);
}

// --- Lumen-optimized multi-source lighting ---

void AExoShowcaseRoom::BuildLighting()
{
	FVector Base = GetActorLocation();

	auto MakeLight = [&](FVector Offset, float Intensity, float Radius,
		FLinearColor Color, bool bShadow, float SrcRadius = 0.f)
	{
		UPointLightComponent* L = NewObject<UPointLightComponent>(this);
		L->SetWorldLocation(Base + Offset);
		L->SetIntensity(Intensity);
		L->SetAttenuationRadius(Radius);
		L->SetLightColor(Color);
		L->CastShadows = bShadow;
		if (SrcRadius > 0.f) L->SourceRadius = SrcRadius;
		L->RegisterComponent();
	};

	// Lumen bounces aggressively in enclosed rooms — keep intensities low
	// Key: warm overhead
	MakeLight(FVector(0, 0, RoomH - 30.f), 500.f, RoomW * 2.f,
		FLinearColor(1.0f, 0.92f, 0.80f), true, 60.f);

	// Fill: cool offset east
	MakeLight(FVector(RoomW * 0.3f, -RoomD * 0.3f, RoomH * 0.7f), 180.f, RoomW * 1.5f,
		FLinearColor(0.80f, 0.88f, 1.0f), false, 60.f);

	// Accent: warm southwest
	MakeLight(FVector(-RoomW * 0.3f, RoomD * 0.2f, RoomH * 0.5f), 120.f,
		RoomW, FLinearColor(1.0f, 0.80f, 0.60f), false, 30.f);

	// Console screen bounce
	MakeLight(FVector(0, RoomD * 0.5f - 50.f, 120.f), 100.f, 400.f,
		FLinearColor(0.3f, 0.5f, 1.0f), false, 20.f);

	// Spotlight on center pedestal — highlights the chrome sphere
	MakeLight(FVector(0, 0, RoomH - 40.f), 200.f, 300.f,
		FLinearColor(1.0f, 0.95f, 0.90f), true, 10.f);
}
