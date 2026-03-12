// ExoShowcaseRoom.cpp — Photo-realistic PBR showcase room with real imported assets
#include "Map/ExoShowcaseRoom.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

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

	ScanAssets();
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

// --- Asset Scanning ---

void AExoShowcaseRoom::ScanAssets()
{
	// Kenney room shell — replaces floor+walls+ceiling with one mesh
	KN_RoomLarge = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Kenney_SpaceKit/room-large"));
	bHasRoomMesh = (KN_RoomLarge != nullptr);

	// Quaternius floor/wall tiles for fallback structure dressing
	QT_FloorBasic = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Quaternius_SciFi/FloorTile_Basic"));
	QT_Wall1 = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Quaternius_SciFi/Walls/Wall_1"));

	// Quaternius props — replace display cubes with real sci-fi objects
	QT_PropsShelf = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Quaternius_SciFi/Props_Shelf"));
	QT_PropsComputer = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Quaternius_SciFi/Props_Computer"));
	QT_PropsCrate = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Meshes/Quaternius_SciFi/Props_Crate"));
	bHasQTProps = (QT_PropsShelf || QT_PropsComputer || QT_PropsCrate);

	UE_LOG(LogExoRift, Log, TEXT("ShowcaseRoom: RoomMesh=%s, QTProps=%s"),
		bHasRoomMesh ? TEXT("Kenney") : TEXT("fallback"),
		bHasQTProps ? TEXT("Quaternius") : TEXT("fallback"));
}

UStaticMeshComponent* AExoShowcaseRoom::SpawnMesh(UStaticMesh* Mesh, FVector Pos,
	FVector Scale, FRotator Rot, bool bColl)
{
	if (!Mesh) return nullptr;
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(Mesh);
	C->SetWorldLocation(GetActorLocation() + Pos);
	C->SetWorldScale3D(Scale);
	C->SetWorldRotation(Rot);
	C->SetCollisionEnabled(bColl ? ECollisionEnabled::QueryAndPhysics
		: ECollisionEnabled::NoCollision);
	if (bColl) C->SetCollisionResponseToAllChannels(ECR_Block);
	C->CastShadow = true;
	C->RegisterComponent();
	return C;
}

// --- Mesh Helpers (shared setup extracted to reduce duplication) ---

static UStaticMeshComponent* SetupComp(AExoShowcaseRoom* Owner, UStaticMesh* Mesh,
	FVector WorldPos, FVector Scale, UMaterialInstanceDynamic* Mat, bool bColl)
{
	if (!Mesh) return nullptr;
	auto* C = NewObject<UStaticMeshComponent>(Owner);
	C->SetStaticMesh(Mesh);
	C->SetWorldLocation(WorldPos);
	C->SetWorldScale3D(Scale);
	if (Mat) C->SetMaterial(0, Mat);
	C->SetCollisionEnabled(bColl ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bColl) C->SetCollisionResponseToAllChannels(ECR_Block);
	C->CastShadow = true;
	C->RegisterComponent();
	return C;
}

UStaticMeshComponent* AExoShowcaseRoom::Box(FVector Pos, FVector Size,
	UMaterialInstanceDynamic* Mat, bool bColl)
{
	return SetupComp(this, CubeMesh, GetActorLocation() + Pos, Size / 100.f, Mat, bColl);
}

UStaticMeshComponent* AExoShowcaseRoom::Sphere(FVector Pos, float Radius,
	UMaterialInstanceDynamic* Mat)
{
	return SetupComp(this, SphereMesh, GetActorLocation() + Pos, FVector(Radius / 50.f), Mat, false);
}

UStaticMeshComponent* AExoShowcaseRoom::Cylinder(FVector Pos, float Radius,
	float Height, UMaterialInstanceDynamic* Mat, bool bColl)
{
	return SetupComp(this, CylinderMesh, GetActorLocation() + Pos,
		FVector(Radius / 50.f, Radius / 50.f, Height / 100.f), Mat, bColl);
}

UStaticMeshComponent* AExoShowcaseRoom::Cone(FVector Pos, float Radius,
	float Height, UMaterialInstanceDynamic* Mat)
{
	return SetupComp(this, ConeMesh, GetActorLocation() + Pos,
		FVector(Radius / 50.f, Radius / 50.f, Height / 100.f), Mat, false);
}

UMaterialInstanceDynamic* AExoShowcaseRoom::PBR(FLinearColor Color, float Met,
	float Rough, float Spec, bool /*bMetal*/)
{
	auto* Base = FExoMaterialFactory::GetLitEmissive();
	auto* M = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (!M) return nullptr;
	M->SetVectorParameterValue(TEXT("BaseColor"), Color);
	M->SetScalarParameterValue(TEXT("Metallic"), Met);
	M->SetScalarParameterValue(TEXT("Roughness"), Rough);
	M->SetScalarParameterValue(TEXT("Specular"), Spec);
	M->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
	return M;
}

UMaterialInstanceDynamic* AExoShowcaseRoom::Glow(FLinearColor Color)
{
	auto* Base = FExoMaterialFactory::GetLitEmissive();
	auto* M = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (!M) return nullptr;
	M->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	M->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.02f, 0.02f, 0.02f));
	M->SetScalarParameterValue(TEXT("Metallic"), 0.f);
	M->SetScalarParameterValue(TEXT("Roughness"), 0.9f);
	return M;
}

// --- Room Structure ---

void AExoShowcaseRoom::BuildStructure()
{
	if (bHasRoomMesh)
	{
		// Kenney room-large is a single mesh with floor, walls, and ceiling
		// Scale to match our 5m x 5m x 3.5m room dimensions
		SpawnMesh(KN_RoomLarge, FVector(0, 0, 0), FVector(2.5f, 2.5f, 1.75f));

		// Polished floor center for Lumen specular reflections (on top of room mesh)
		auto* PolFloor = PBR(FLinearColor(0.08f, 0.08f, 0.09f), 0.05f, 0.25f, 0.7f);
		Box(FVector(0, 0, 0.3f), FVector(300.f, 300.f, 0.5f), PolFloor, false);
		return;
	}

	// --- Fallback: primitive geometry ---
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f, WT = WallThick;
	auto* FM = PBR(FLinearColor(0.12f, 0.11f, 0.10f), 0.02f, 0.88f, 0.35f);
	auto* WM = PBR(FLinearColor(0.20f, 0.21f, 0.23f), 0.55f, 0.42f, 0.5f);
	auto* CM = PBR(FLinearColor(0.10f, 0.10f, 0.12f), 0.15f, 0.72f, 0.4f);
	float FW = RoomW + WT * 2;

	Box(FVector(0, 0, -WT * 0.5f), FVector(FW, RoomD + WT * 2, WT), FM);         // Floor
	Box(FVector(0, 0, 0.3f), FVector(300.f, 300.f, 0.5f),                         // Polished center
		PBR(FLinearColor(0.08f, 0.08f, 0.09f), 0.05f, 0.25f, 0.7f), false);
	Box(FVector(0, 0, RoomH + WT * 0.5f), FVector(FW, RoomD + WT * 2, WT), CM);   // Ceiling
	Box(FVector(0, HD + WT * 0.5f, RoomH * 0.5f), FVector(FW, WT, RoomH), WM);    // North
	Box(FVector(HW + WT * 0.5f, 0, RoomH * 0.5f), FVector(WT, RoomD, RoomH), WM); // East
	Box(FVector(-HW - WT * 0.5f, 0, RoomH * 0.5f), FVector(WT, RoomD, RoomH), WM);// West
	// South wall split for doorway
	float SW = (RoomW - 120.f) * 0.5f;
	Box(FVector(-155.f, -HD - WT * 0.5f, RoomH * 0.5f), FVector(SW, WT, RoomH), WM);
	Box(FVector(155.f, -HD - WT * 0.5f, RoomH * 0.5f), FVector(SW, WT, RoomH), WM);
	Box(FVector(0, -HD - WT * 0.5f, 295.f), FVector(130.f, WT, 110.f), WM);
}

// --- Wall Panels, Trim & Beams ---

void AExoShowcaseRoom::BuildPanels()
{
	if (bHasRoomMesh) return; // Kenney room mesh includes wall detail

	float HW = RoomW * 0.5f, HD = RoomD * 0.5f;
	auto* PanMat = PBR(FLinearColor(0.14f, 0.15f, 0.18f), 0.65f, 0.32f, 0.55f);
	auto* TrimMat = PBR(FLinearColor(0.40f, 0.42f, 0.45f), 0.90f, 0.15f, 0.7f);
	auto* BordMat = PBR(FLinearColor(0.25f, 0.26f, 0.28f), 0.80f, 0.25f, 0.6f);
	auto* DoorMat = PBR(FLinearColor(0.38f, 0.40f, 0.44f), 0.88f, 0.18f, 0.65f);
	auto* BeamMat = PBR(FLinearColor(0.28f, 0.28f, 0.30f), 0.75f, 0.28f, 0.55f);

	// Recessed panels on N/S walls
	for (float YS : {1.f, -1.f})
	{
		float WY = YS * HD;
		for (float PX : {-0.25f * RoomW, 0.25f * RoomW})
			Box(FVector(PX, WY - YS * 3.f, 200.f), FVector(160.f, 2.f, 200.f), PanMat, false);
		float TW = (YS > 0) ? RoomW - 20.f : RoomW * 0.3f;
		Box(FVector(0, WY - YS, 120.f), FVector(TW, 4.f, 4.f), TrimMat, false);
	}
	// Recessed panels on E/W walls
	for (float XS : {1.f, -1.f})
	{
		float WX = XS * HW;
		for (float PY : {-0.25f * RoomD, 0.25f * RoomD})
			Box(FVector(WX - XS * 3.f, PY, 200.f), FVector(2.f, 160.f, 200.f), PanMat, false);
		Box(FVector(WX - XS, 0, 120.f), FVector(4.f, RoomD - 20.f, 4.f), TrimMat, false);
	}

	// Floor border inlay
	Box(FVector(0, HD - 6.f, 0.75f), FVector(RoomW, 12.f, 1.5f), BordMat, false);
	Box(FVector(0, -HD + 6.f, 0.75f), FVector(RoomW, 12.f, 1.5f), BordMat, false);
	Box(FVector(HW - 6.f, 0, 0.75f), FVector(12.f, RoomD, 1.5f), BordMat, false);
	Box(FVector(-HW + 6.f, 0, 0.75f), FVector(12.f, RoomD, 1.5f), BordMat, false);

	// Ceiling cross-beams + corner pillars
	Box(FVector(0, 0, RoomH - 8.f), FVector(RoomW - 10.f, 18.f, 18.f), BeamMat, false);
	Box(FVector(0, 0, RoomH - 8.f), FVector(18.f, RoomD - 10.f, 18.f), BeamMat, false);
	for (int32 CX : {-1, 1})
		for (int32 CY : {-1, 1})
			Cylinder(FVector(CX * (HW - 8.f), CY * (HD - 8.f), RoomH * 0.5f), 8.f, RoomH, TrimMat);

	// Door frame
	Box(FVector(-63.f, -HD + 1.f, 120.f), FVector(6.f, 4.f, 240.f), DoorMat, false);
	Box(FVector(63.f, -HD + 1.f, 120.f), FVector(6.f, 4.f, 240.f), DoorMat, false);
	Box(FVector(0, -HD + 1.f, 243.f), FVector(132.f, 4.f, 6.f), DoorMat, false);
}

// --- Props, Emissive Accents & Conduits ---

void AExoShowcaseRoom::BuildDetails()
{
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f;
	auto* PipeMat = PBR(FLinearColor(0.35f, 0.35f, 0.37f), 0.85f, 0.20f, 0.6f);
	auto* WarmGlow = Glow(FLinearColor(0.8f, 0.5f, 0.2f));
	auto* CoolGlow = Glow(FLinearColor(0.1f, 0.2f, 0.5f));

	// Console desk against north wall (skip if real computer prop placed)
	if (!QT_PropsComputer)
	{
		auto* ConMat = PBR(FLinearColor(0.06f, 0.06f, 0.08f), 0.35f, 0.55f, 0.45f);
		float DkY = HD - 47.5f;
		Box(FVector(0, DkY, 80.f), FVector(140.f, 55.f, 4.f), ConMat, false);
		Cylinder(FVector(-53.2f, DkY, 40.f), 4.f, 76.f, ConMat);
		Cylinder(FVector(53.2f, DkY, 40.f), 4.f, 76.f, ConMat);
		Box(FVector(0, DkY + 16.5f, 115.f), FVector(90.f, 2.f, 55.f),
			Glow(FLinearColor(0.15f, 0.3f, 0.6f)), false);
	}

	// Emissive accent strips at 120cm height
	Box(FVector(0, HD - 1.f, 120.f), FVector(RoomW * 0.6f, 1.f, 2.f), WarmGlow, false);
	Box(FVector(0, -HD + 1.f, 120.f), FVector(RoomW * 0.3f, 1.f, 2.f), WarmGlow, false);
	Box(FVector(HW - 1.f, 0, 120.f), FVector(1.f, RoomD * 0.6f, 2.f), WarmGlow, false);
	Box(FVector(-HW + 1.f, 0, 120.f), FVector(1.f, RoomD * 0.6f, 2.f), WarmGlow, false);
	Box(FVector(0, -HD + 1.f, 243.f), FVector(130.f, 1.f, 2.f), CoolGlow, false);

	// Ceiling conduit pipes
	Cylinder(FVector(0, HD - 25.f, RoomH - 20.f), 6.f, RoomW * 0.8f, PipeMat);
	Box(FVector(0, HD - 45.f, RoomH - 15.f), FVector(RoomW * 0.6f, 8.f, 8.f), PipeMat, false);

	// Floor vent grating
	auto* VentMat = PBR(FLinearColor(0.08f, 0.08f, 0.08f), 0.70f, 0.35f, 0.5f);
	Box(FVector(90.f, -70.f, 0.5f), FVector(65.f, 45.f, 1.f), VentMat, false);
}

// --- Display Objects (real props when available, primitives as fallback) ---

void AExoShowcaseRoom::BuildDisplayObjects()
{
	float HW = RoomW * 0.5f, HD = RoomD * 0.5f;

	// Center pedestal + chrome sphere (always primitives for PBR showcase)
	auto* PM = PBR(FLinearColor(0.05f, 0.05f, 0.06f), 0.02f, 0.30f, 0.6f);
	Cylinder(FVector(0, 0, 15.f), 30.f, 30.f, PM, true);
	Cylinder(FVector(0, 0, 50.f), 12.f, 40.f, PM);
	Cylinder(FVector(0, 0, 72.f), 25.f, 4.f, PM);
	Sphere(FVector(0, 0, 100.f), 26.f, PBR(FLinearColor(0.95f, 0.93f, 0.88f), 1.0f, 0.02f, 1.0f));

	// East wall: real shelf or primitive bracket
	float SX = HW - 30.f;
	if (QT_PropsShelf)
		SpawnMesh(QT_PropsShelf, FVector(SX - 10.f, 0, 0), FVector(1.2f), FRotator(0, -90, 0));
	else
	{
		auto* SM = PBR(FLinearColor(0.15f, 0.15f, 0.17f), 0.80f, 0.25f, 0.5f);
		Box(FVector(SX, 0, 110.f), FVector(50.f, 200.f, 4.f), SM, false);
		Box(FVector(SX + 2.f, -70.f, 95.f), FVector(4.f, 4.f, 26.f), SM, false);
		Box(FVector(SX + 2.f, 70.f, 95.f), FVector(4.f, 4.f, 26.f), SM, false);
	}

	// Material sample spheres (always shown for PBR demo)
	Sphere(FVector(SX, -60.f, 126.f), 14.f, PBR(FLinearColor(0.95f, 0.64f, 0.54f), 1.f, 0.25f, 0.7f));
	Sphere(FVector(SX, -20.f, 126.f), 14.f, PBR(FLinearColor(1.0f, 0.76f, 0.33f), 1.f, 0.15f, 0.8f));
	Sphere(FVector(SX, 20.f, 126.f), 14.f, PBR(FLinearColor(0.08f, 0.08f, 0.08f), 0.f, 0.95f, 0.3f));
	Sphere(FVector(SX, 60.f, 126.f), 14.f, PBR(FLinearColor(0.85f, 0.85f, 0.80f), 0.f, 0.45f, 0.5f));

	// North wall: real computer or primitive vases
	if (QT_PropsComputer)
		SpawnMesh(QT_PropsComputer, FVector(0, HD - 40.f, 0), FVector(1.5f), FRotator(0, 180, 0));
	else
	{
		auto* DM = PBR(FLinearColor(0.10f, 0.10f, 0.12f), 0.85f, 0.20f, 0.6f);
		auto* TM = PBR(FLinearColor(0.15f, 0.25f, 0.35f), 0.60f, 0.30f, 0.5f);
		Cylinder(FVector(-HW + 40.f, HD - 50.f, 40.f), 12.f, 80.f, DM);
		Cylinder(FVector(-HW + 40.f, HD - 90.f, 25.f), 16.f, 50.f,
			PBR(FLinearColor(0.30f, 0.32f, 0.35f), 0.70f, 0.40f, 0.5f));
		Cone(FVector(-HW + 40.f, HD - 90.f, 55.f), 16.f, 20.f, TM);
		Cylinder(FVector(-HW + 40.f, 0, 18.f), 20.f, 36.f, TM);
	}

	// SW corner: real crates or cone sculpture
	if (QT_PropsCrate)
	{
		SpawnMesh(QT_PropsCrate, FVector(-HW + 50.f, -HD + 50.f, 0), FVector(1.5f));
		SpawnMesh(QT_PropsCrate, FVector(-HW + 55.f, -HD + 45.f, 55.f),
			FVector(1.2f), FRotator(0, 25, 0), false);
	}
	else
	{
		auto* SC = PBR(FLinearColor(0.60f, 0.55f, 0.45f), 0.95f, 0.08f, 0.9f);
		Cylinder(FVector(HW - 60.f, HD - 60.f, 5.f), 22.f, 10.f, SC);
		Cone(FVector(HW - 60.f, HD - 60.f, 45.f), 18.f, 70.f, SC);
		Sphere(FVector(HW - 60.f, HD - 60.f, 85.f), 8.f, SC);
	}

	// Bollards near door
	auto* BM = PBR(FLinearColor(0.25f, 0.25f, 0.27f), 0.80f, 0.30f, 0.5f);
	auto* BG = Glow(FLinearColor(0.4f, 0.6f, 1.0f));
	for (float X : {-70.f, 70.f})
	{
		Cylinder(FVector(X, -HD + 30.f, 25.f), 6.f, 50.f, BM, true);
		Sphere(FVector(X, -HD + 30.f, 52.f), 6.5f, BG);
	}
}

// --- Lumen-optimized multi-source lighting ---

void AExoShowcaseRoom::BuildLighting()
{
	FVector Base = GetActorLocation();
	auto ML = [&](FVector Off, float I, float R, FLinearColor C, bool Sh, float Sr = 0.f) {
		auto* L = NewObject<UPointLightComponent>(this);
		L->SetWorldLocation(Base + Off);
		L->SetIntensity(I); L->SetAttenuationRadius(R); L->SetLightColor(C);
		L->CastShadows = Sh;
		if (Sr > 0.f) L->SourceRadius = Sr;
		L->RegisterComponent();
	};
	ML(FVector(0, 0, RoomH - 30.f), 500.f, RoomW * 2.f,
		FLinearColor(1.0f, 0.92f, 0.80f), true, 60.f);               // Key: warm overhead
	ML(FVector(RoomW * 0.3f, -RoomD * 0.3f, RoomH * 0.7f), 180.f,
		RoomW * 1.5f, FLinearColor(0.80f, 0.88f, 1.0f), false, 60.f); // Fill: cool east
	ML(FVector(-RoomW * 0.3f, RoomD * 0.2f, RoomH * 0.5f), 120.f,
		RoomW, FLinearColor(1.0f, 0.80f, 0.60f), false, 30.f);        // Accent: warm SW
	ML(FVector(0, RoomD * 0.5f - 50.f, 120.f), 100.f, 400.f,
		FLinearColor(0.3f, 0.5f, 1.0f), false, 20.f);                 // Console bounce
	ML(FVector(0, 0, RoomH - 40.f), 200.f, 300.f,
		FLinearColor(1.0f, 0.95f, 0.90f), true, 10.f);                // Pedestal spot
}
