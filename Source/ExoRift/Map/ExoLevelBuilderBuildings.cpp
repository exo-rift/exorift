// ExoLevelBuilderBuildings.cpp — SpawnBuilding with walls, emissives, windows
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoAutoSlidingDoor.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::SpawnBuilding(const FVector& Center, const FVector& Size, float Rotation)
{
	FLinearColor WallColor(0.08f, 0.09f, 0.11f);
	FLinearColor RoofColor(0.06f, 0.07f, 0.09f);
	FLinearColor TrimColor(0.12f, 0.14f, 0.18f);
	float WallThickness = 80.f;
	FRotator Rot(0.f, Rotation, 0.f);

	float HalfX = Size.X * 0.5f;
	float HalfY = Size.Y * 0.5f;
	float HalfZ = Size.Z * 0.5f;

	// Derive accent color from building position for compound-specific identity
	FLinearColor AccentGlow(0.1f, 0.5f, 1.2f); // Default: blue
	FLinearColor WinGlow(0.06f, 0.15f, 0.35f);
	if (Center.Y > 40000.f)
	{	// North compound — amber industrial
		AccentGlow = FLinearColor(1.2f, 0.6f, 0.12f);
		WinGlow = FLinearColor(0.3f, 0.15f, 0.04f);
	}
	else if (Center.Y < -40000.f)
	{	// South compound — teal research
		AccentGlow = FLinearColor(0.15f, 1.0f, 0.7f);
		WinGlow = FLinearColor(0.04f, 0.25f, 0.18f);
	}
	else if (Center.X > 40000.f)
	{	// East compound — red power
		AccentGlow = FLinearColor(1.2f, 0.25f, 0.15f);
		WinGlow = FLinearColor(0.3f, 0.06f, 0.04f);
	}
	else if (Center.X < -40000.f)
	{	// West compound — green military
		AccentGlow = FLinearColor(0.2f, 1.0f, 0.3f);
		WinGlow = FLinearColor(0.05f, 0.25f, 0.08f);
	}

	// Floor
	SpawnStaticMesh(Center + FVector(0.f, 0.f, 10.f),
		FVector(Size.X / 100.f, Size.Y / 100.f, 0.2f), Rot, CubeMesh,
		FLinearColor(0.07f, 0.07f, 0.08f));

	// Back wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY, HalfZ)),
		FVector(Size.X / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);
	// Left wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX, 0.f, HalfZ)),
		FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);
	// Right wall
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX, 0.f, HalfZ)),
		FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Front wall — two segments with door gap
	float DoorHalf = 600.f;
	float SegX_Left = (-HalfX + (-DoorHalf)) * 0.5f;
	float SegX_Right = (DoorHalf + HalfX) * 0.5f;
	float SegW_Left = (-DoorHalf) - (-HalfX);
	float SegW_Right = HalfX - DoorHalf;
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_Left, HalfY, HalfZ)),
		FVector(SegW_Left / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_Right, HalfY, HalfZ)),
		FVector(SegW_Right / 100.f, WallThickness / 100.f, Size.Z / 100.f), Rot, CubeMesh, WallColor);

	// Door frame header
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY, Size.Z - 200.f)),
		FVector(DoorHalf * 2.f / 100.f, WallThickness / 100.f + 0.1f, 2.f), Rot, CubeMesh, TrimColor);

	// Corner pillars — structural columns at each building corner
	FLinearColor PillarColor(0.1f, 0.11f, 0.13f);
	float PillarW = 120.f;
	for (float CX : {-HalfX, HalfX})
		for (float CY : {-HalfY, HalfY})
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(CX, CY, HalfZ)),
				FVector(PillarW / 100.f, PillarW / 100.f, Size.Z / 100.f + 0.2f),
				Rot, CubeMesh, PillarColor);

	// Base plinth — dark strip around building perimeter
	FLinearColor PlinthColor(0.04f, 0.045f, 0.05f);
	float PlinthH = 40.f;
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY, PlinthH * 0.5f)),
		FVector(Size.X / 100.f + 0.2f, 0.3f, PlinthH / 100.f), Rot, CubeMesh, PlinthColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY, PlinthH * 0.5f)),
		FVector(Size.X / 100.f + 0.2f, 0.3f, PlinthH / 100.f), Rot, CubeMesh, PlinthColor);

	// Door frame side posts
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-DoorHalf, HalfY, HalfZ * 0.8f)),
		FVector(0.6f, WallThickness / 100.f + 0.15f, Size.Z / 100.f * 0.8f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(DoorHalf, HalfY, HalfZ * 0.8f)),
		FVector(0.6f, WallThickness / 100.f + 0.15f, Size.Z / 100.f * 0.8f), Rot, CubeMesh, TrimColor);

	// Front entrance step
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 60.f, 15.f)),
		FVector(DoorHalf * 2.f / 100.f + 0.5f, 1.2f, 0.3f), Rot, CubeMesh,
		FLinearColor(0.065f, 0.07f, 0.08f));

	// Roof + trim (all four sides)
	SpawnStaticMesh(Center + FVector(0.f, 0.f, Size.Z),
		FVector(Size.X / 100.f + 0.3f, Size.Y / 100.f + 0.3f, 0.3f), Rot, CubeMesh, RoofColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 15.f, Size.Z + 20.f)),
		FVector(Size.X / 100.f + 0.4f, 0.1f, 0.4f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 15.f, Size.Z + 20.f)),
		FVector(Size.X / 100.f + 0.4f, 0.1f, 0.4f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX - 15.f, 0.f, Size.Z + 20.f)),
		FVector(0.1f, Size.Y / 100.f + 0.4f, 0.4f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX + 15.f, 0.f, Size.Z + 20.f)),
		FVector(0.1f, Size.Y / 100.f + 0.4f, 0.4f), Rot, CubeMesh, TrimColor);

	// Emissive accent strips on front/back walls
	float StripZ = Size.Z * 0.6f;
	UMaterialInterface* StripMat = FExoMaterialFactory::GetEmissiveOpaque();
	auto MakeStrip = [&](const FVector& Offset, const FVector& Scale)
	{
		UStaticMeshComponent* S = SpawnStaticMesh(
			Center + Rot.RotateVector(Offset), Scale, Rot, CubeMesh,
			FLinearColor(AccentGlow.R * 0.1f, AccentGlow.G * 0.1f, AccentGlow.B * 0.1f));
		if (S && StripMat)
		{
			UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(StripMat, this);
			SM->SetVectorParameterValue(TEXT("EmissiveColor"), AccentGlow);
			S->SetMaterial(0, SM);
		}
	};
	MakeStrip(FVector(0.f, HalfY + 1.f, StripZ), FVector(Size.X / 100.f * 0.8f, 0.02f, 0.15f));
	MakeStrip(FVector(0.f, -HalfY - 1.f, StripZ), FVector(Size.X / 100.f * 0.8f, 0.02f, 0.15f));
	MakeStrip(FVector(-HalfX - 1.f, 0.f, StripZ), FVector(0.02f, Size.Y / 100.f * 0.7f, 0.12f));
	MakeStrip(FVector(HalfX + 1.f, 0.f, StripZ), FVector(0.02f, Size.Y / 100.f * 0.7f, 0.12f));

	// Doorway + interior lights
	UPointLightComponent* DoorLight = NewObject<UPointLightComponent>(this);
	DoorLight->SetupAttachment(RootComponent);
	DoorLight->SetWorldLocation(
		Center + Rot.RotateVector(FVector(0.f, HalfY + 50.f, Size.Z - 300.f)));
	DoorLight->SetIntensity(2500.f);
	DoorLight->SetAttenuationRadius(800.f);
	DoorLight->SetLightColor(FLinearColor(0.8f, 0.6f, 0.3f));
	DoorLight->CastShadows = false;
	DoorLight->RegisterComponent();

	UPointLightComponent* IntLight = NewObject<UPointLightComponent>(this);
	IntLight->SetupAttachment(RootComponent);
	IntLight->SetWorldLocation(Center + FVector(0.f, 0.f, Size.Z - 100.f));
	IntLight->SetIntensity(2000.f);
	IntLight->SetAttenuationRadius(FMath::Max(Size.X, Size.Y) * 0.6f);
	IntLight->SetLightColor(FLinearColor(0.5f, 0.6f, 0.8f));
	IntLight->CastShadows = false;
	IntLight->RegisterComponent();

	// Horizontal wall panel line (mid-height detail break)
	float PanelZ = Size.Z * 0.3f;
	FLinearColor PanelLine(0.065f, 0.075f, 0.09f);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 1.f, PanelZ)),
		FVector(Size.X / 100.f * 0.95f, 0.015f, 0.06f), Rot, CubeMesh, PanelLine);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 1.f, PanelZ)),
		FVector(Size.X / 100.f * 0.95f, 0.015f, 0.06f), Rot, CubeMesh, PanelLine);

	// Rooftop equipment on larger buildings
	if (Size.X > 3000.f && Size.Y > 3000.f)
	{
		FVector VentPos = Center + FVector(
			FMath::RandRange(-HalfX * 0.3f, HalfX * 0.3f),
			FMath::RandRange(-HalfY * 0.3f, HalfY * 0.3f), Size.Z + 60.f);
		SpawnStaticMesh(VentPos, FVector(1.5f, 1.f, 0.6f), Rot, CubeMesh,
			FLinearColor(0.08f, 0.08f, 0.1f));
		SpawnStaticMesh(VentPos + FVector(0.f, 0.f, 35.f),
			FVector(1.3f, 0.8f, 0.05f), Rot, CubeMesh, FLinearColor(0.06f, 0.06f, 0.07f));
		// Second rooftop unit offset from the first
		FVector Vent2 = Center + FVector(
			FMath::RandRange(-HalfX * 0.3f, HalfX * 0.3f),
			FMath::RandRange(-HalfY * 0.3f, HalfY * 0.3f), Size.Z + 40.f);
		SpawnStaticMesh(Vent2, FVector(0.6f, 0.6f, 0.8f), Rot, CylinderMesh,
			FLinearColor(0.07f, 0.07f, 0.09f));
	}

	// Window panels along walls — emissive glow from interior lighting
	int32 NumWindows = FMath::Max(1, FMath::RoundToInt32(Size.X / 1500.f));
	float WinSpacing = Size.X / (NumWindows + 1);
	float WinZ = Size.Z * 0.4f;
	UMaterialInterface* WinMat = FExoMaterialFactory::GetEmissiveOpaque();
	for (int32 w = 0; w < NumWindows; w++)
	{
		float WX = -HalfX + WinSpacing * (w + 1);
		for (float Side : {HalfY + 2.f, -HalfY - 2.f})
		{
			UStaticMeshComponent* WC = SpawnStaticMesh(
				Center + Rot.RotateVector(FVector(WX, Side, WinZ)),
				FVector(0.8f, 0.015f, 0.6f), Rot, CubeMesh,
				FLinearColor(WinGlow.R * 0.5f, WinGlow.G * 0.5f, WinGlow.B * 0.5f));
			if (WC && WinMat)
			{
				UMaterialInstanceDynamic* WM = UMaterialInstanceDynamic::Create(WinMat, this);
				WM->SetVectorParameterValue(TEXT("EmissiveColor"), WinGlow);
				WC->SetMaterial(0, WM);
			}
		}
	}

	// Auto-sliding door at the front entrance
	FVector DoorPos = Center + Rot.RotateVector(FVector(0.f, HalfY, 0.f));
	float DoorH = FMath::Min(Size.Z - 200.f, 350.f);
	AExoAutoSlidingDoor* Door = GetWorld()->SpawnActor<AExoAutoSlidingDoor>(
		AExoAutoSlidingDoor::StaticClass(), DoorPos, Rot);
	if (Door)
	{
		Door->InitDoor(DoorHalf * 2.f, DoorH, DoorHalf * 0.9f, AccentGlow);
	}
}
