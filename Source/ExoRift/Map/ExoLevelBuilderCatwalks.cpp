// ExoLevelBuilderCatwalks.cpp — Elevated walkways, catwalks, and vertical gameplay elements
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildCatwalks()
{
	// === CENTRAL HUB — catwalks from roof to surrounding platforms ===
	// Hub main roof is at Z=4000, second floor at 2500
	SpawnCatwalk(FVector(4000.f, 0.f, 2500.f), FVector(5000.f, 0.f, 1500.f), 200.f);
	SpawnCatwalk(FVector(-4000.f, 0.f, 2500.f), FVector(-4000.f, 3000.f, 1200.f), 200.f);

	// Observation platform overlooking landing pad
	SpawnObservationDeck(FVector(5000.f, 3000.f, 2500.f), 1500.f, 0.f);

	// === NORTH COMPOUND — industrial catwalks between warehouse roofs ===
	float NY = 80000.f;
	// Warehouse-to-warehouse catwalk at gantry height
	SpawnCatwalk(FVector(-5000.f, NY, 3000.f), FVector(5000.f, NY, 2000.f), 250.f);
	// Gantry walkway extensions
	SpawnCatwalk(FVector(-5500.f, NY, 4000.f), FVector(-5500.f, NY + 5000.f, 4000.f), 200.f);
	SpawnCatwalk(FVector(5500.f, NY, 4000.f), FVector(5500.f, NY + 5000.f, 4000.f), 200.f);
	// Perimeter wall to tower catwalk
	SpawnCatwalk(FVector(0.f, NY - 4000.f, 1580.f), FVector(0.f, NY + 5000.f, 5000.f), 180.f);

	// === SOUTH COMPOUND — research lab connectors ===
	float SY = -80000.f;
	// Lab-to-lab elevated bridge
	SpawnCatwalk(FVector(3000.f, SY + 3500.f, 2200.f),
		FVector(-6000.f, SY + 2000.f, 1800.f), 300.f);
	SpawnCatwalk(FVector(-6000.f, SY - 3000.f, 2000.f),
		FVector(-6000.f, SY + 2000.f, 1800.f), 200.f);
	// Research dome observation ring
	SpawnObservationDeck(FVector(3000.f, SY, 2800.f), 2000.f, 0.f);

	// === EAST COMPOUND — power station catwalks ===
	float EX = 80000.f;
	// Tower-to-tower catwalk
	SpawnCatwalk(FVector(EX + 5000.f, 3000.f, 6000.f),
		FVector(EX + 5000.f, -5000.f, 6000.f), 200.f);
	// Building roof to guard tower
	SpawnCatwalk(FVector(EX + 2500.f, -3000.f, 3500.f),
		FVector(EX + 5000.f, -5000.f, 6000.f), 200.f);
	// Pylon inspection walkway
	SpawnCatwalk(FVector(EX + 8000.f, -6000.f, 4000.f),
		FVector(EX + 8000.f, 6000.f, 4000.f), 150.f);

	// === WEST COMPOUND — barracks connectors ===
	float WX = -80000.f;
	// Connect barracks rooftops via walkways
	for (int32 i = 0; i < 3; i++)
	{
		float Y1 = (i - 1.5f) * 5000.f;
		float Y2 = (i - 0.5f) * 5000.f;
		SpawnCatwalk(FVector(WX, Y1, 1800.f), FVector(WX, Y2, 1800.f), 180.f);
	}
	// Guard tower to barracks line
	SpawnCatwalk(FVector(WX - 3000.f, -12000.f, 4000.f),
		FVector(WX, -7500.f, 1800.f), 180.f);

	// === CORNER OUTPOST elevated connectors ===
	float CornerDist = 120000.f;
	FVector Corners[] = {
		{CornerDist, CornerDist, 0.f}, {-CornerDist, CornerDist, 0.f},
		{CornerDist, -CornerDist, 0.f}, {-CornerDist, -CornerDist, 0.f}
	};
	for (const FVector& C : Corners)
	{
		// Building to tower catwalk
		SpawnCatwalk(C + FVector(0.f, 0.f, 2000.f),
			C + FVector(3000.f, 3000.f, 3500.f), 150.f);
		// Bunker to building catwalk
		SpawnCatwalk(C + FVector(-2500.f, 1500.f, 1200.f),
			C + FVector(0.f, 0.f, 2000.f), 150.f);
	}

	// === ZIPLINE ANCHORS — tall poles at key points for future zipline mechanic ===
	SpawnZiplineAnchor(FVector(0.f, 0.f, 6000.f));       // Hub comm tower top
	SpawnZiplineAnchor(FVector(0.f, 80000.f + 5000.f, 5000.f)); // North tower
	SpawnZiplineAnchor(FVector(80000.f + 5000.f, 3000.f, 6000.f)); // East tower N
	SpawnZiplineAnchor(FVector(80000.f + 5000.f, -5000.f, 6000.f)); // East tower S
	SpawnZiplineAnchor(FVector(-80000.f - 3000.f, -12000.f, 4000.f)); // West tower S
	SpawnZiplineAnchor(FVector(-80000.f - 3000.f, 12000.f, 4000.f)); // West tower N

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Catwalks and vertical elements placed"));
}

void AExoLevelBuilder::SpawnCatwalk(const FVector& Start, const FVector& End, float Width)
{
	FVector Dir = End - Start;
	float Length = Dir.Size();
	if (Length < 10.f) return;

	FVector Mid = (Start + End) * 0.5f;
	FRotator Rot = Dir.Rotation();

	// Main walkway surface — grated metal look
	SpawnStaticMesh(Mid,
		FVector(Length / 100.f, Width / 100.f, 0.15f), Rot, CubeMesh,
		FLinearColor(0.065f, 0.07f, 0.08f));

	// Railings on both sides
	FVector Right = FRotationMatrix(Rot).GetUnitAxis(EAxis::Y);
	float RailHeight = 100.f;
	float RailOffset = Width * 0.5f;

	// Left railing
	SpawnStaticMesh(Mid + Right * RailOffset + FVector(0.f, 0.f, RailHeight * 0.5f),
		FVector(Length / 100.f, 0.03f, RailHeight / 100.f), Rot, CubeMesh,
		FLinearColor(0.1f, 0.1f, 0.12f));
	// Right railing
	SpawnStaticMesh(Mid - Right * RailOffset + FVector(0.f, 0.f, RailHeight * 0.5f),
		FVector(Length / 100.f, 0.03f, RailHeight / 100.f), Rot, CubeMesh,
		FLinearColor(0.1f, 0.1f, 0.12f));

	// Support struts underneath — every 1500 units along length
	FVector Forward = Dir.GetSafeNormal();
	int32 StrutCount = FMath::Max(2, FMath::RoundToInt32(Length / 1500.f));
	for (int32 i = 0; i < StrutCount; i++)
	{
		float T = (float)(i + 1) / (float)(StrutCount + 1);
		FVector StrutPos = FMath::Lerp(Start, End, T);
		float FloorZ = FMath::Min(Start.Z, End.Z);
		float StrutH = StrutPos.Z - FloorZ;
		if (StrutH < 100.f) continue;

		// Vertical support column
		SpawnStaticMesh(
			FVector(StrutPos.X, StrutPos.Y, FloorZ + StrutH * 0.5f),
			FVector(0.3f, 0.3f, StrutH / 100.f), FRotator::ZeroRotator, CylinderMesh,
			FLinearColor(0.08f, 0.08f, 0.1f));
	}

	// Accent lights along the walkway — dim blue strip lights
	int32 LightCount = FMath::Max(1, FMath::RoundToInt32(Length / 3000.f));
	for (int32 i = 0; i < LightCount; i++)
	{
		float T = (float)(i + 0.5f) / (float)LightCount;
		FVector LightPos = FMath::Lerp(Start, End, T) + FVector(0.f, 0.f, -5.f);

		UPointLightComponent* WalkLight = NewObject<UPointLightComponent>(this);
		WalkLight->SetupAttachment(RootComponent);
		WalkLight->SetWorldLocation(LightPos);
		WalkLight->SetIntensity(800.f);
		WalkLight->SetAttenuationRadius(500.f);
		WalkLight->SetLightColor(FLinearColor(0.2f, 0.5f, 1.f));
		WalkLight->CastShadows = false;
		WalkLight->RegisterComponent();
	}
}

void AExoLevelBuilder::SpawnObservationDeck(const FVector& Center, float Radius,
	float Yaw)
{
	FRotator Rot(0.f, Yaw, 0.f);

	// Circular platform
	float ScaleXY = Radius / 50.f;
	SpawnStaticMesh(Center,
		FVector(ScaleXY, ScaleXY, 0.15f), Rot, CylinderMesh,
		FLinearColor(0.06f, 0.07f, 0.085f));

	// Perimeter railing — 4 straight railing segments approximating circle
	float RailH = 100.f;
	for (int32 i = 0; i < 8; i++)
	{
		float Angle = i * 45.f;
		float Rad = FMath::DegreesToRadians(Angle);
		FVector Pos = Center + FVector(
			FMath::Cos(Rad) * Radius * 0.95f,
			FMath::Sin(Rad) * Radius * 0.95f,
			RailH * 0.5f);
		float SegLen = Radius * 0.75f;
		SpawnStaticMesh(Pos,
			FVector(SegLen / 100.f, 0.03f, RailH / 100.f),
			FRotator(0.f, Angle + 90.f, 0.f), CubeMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));
	}

	// Central bollard with light
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 75.f),
		FVector(0.15f, 0.15f, 1.5f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.08f, 0.08f, 0.1f));

	UPointLightComponent* DeckLight = NewObject<UPointLightComponent>(this);
	DeckLight->SetupAttachment(RootComponent);
	DeckLight->SetWorldLocation(Center + FVector(0.f, 0.f, 200.f));
	DeckLight->SetIntensity(3000.f);
	DeckLight->SetAttenuationRadius(Radius * 1.5f);
	DeckLight->SetLightColor(FLinearColor(0.3f, 0.6f, 1.f));
	DeckLight->CastShadows = false;
	DeckLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnZiplineAnchor(const FVector& Top)
{
	// Tall anchor pole with glowing attachment point
	float PoleHeight = 300.f;
	SpawnStaticMesh(Top + FVector(0.f, 0.f, PoleHeight * 0.5f),
		FVector(0.15f, 0.15f, PoleHeight / 100.f), FRotator::ZeroRotator, CylinderMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));

	// Cross arm for cable attachment
	SpawnStaticMesh(Top + FVector(0.f, 0.f, PoleHeight),
		FVector(1.5f, 0.1f, 0.1f), FRotator::ZeroRotator, CubeMesh,
		FLinearColor(0.12f, 0.12f, 0.14f));

	// Glowing attachment sphere
	UStaticMeshComponent* Anchor = SpawnStaticMesh(
		Top + FVector(0.f, 0.f, PoleHeight + 20.f),
		FVector(0.15f, 0.15f, 0.15f), FRotator::ZeroRotator, SphereMesh,
		FLinearColor(0.1f, 0.8f, 0.3f));
	if (Anchor)
	{
		UMaterialInstanceDynamic* Mat = Cast<UMaterialInstanceDynamic>(
			Anchor->GetMaterial(0));
		if (Mat) Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(1.f, 4.f, 1.5f));
	}

	// Anchor light
	UPointLightComponent* AnchorLight = NewObject<UPointLightComponent>(this);
	AnchorLight->SetupAttachment(RootComponent);
	AnchorLight->SetWorldLocation(Top + FVector(0.f, 0.f, PoleHeight + 30.f));
	AnchorLight->SetIntensity(2000.f);
	AnchorLight->SetAttenuationRadius(400.f);
	AnchorLight->SetLightColor(FLinearColor(0.2f, 1.f, 0.4f));
	AnchorLight->CastShadows = false;
	AnchorLight->RegisterComponent();
}
