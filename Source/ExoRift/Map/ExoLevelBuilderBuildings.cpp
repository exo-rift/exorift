// ExoLevelBuilderBuildings.cpp — SpawnBuilding with walls, emissives, windows
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoAutoSlidingDoor.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::SpawnBuilding(const FVector& Center, const FVector& Size, float Rotation)
{
	// Position-based wall color variation — each building slightly different
	float BldgHash = FMath::Abs(FMath::Sin(Center.X * 0.00007f + Center.Y * 0.00011f));
	FLinearColor WallColor(
		0.14f + BldgHash * 0.06f,
		0.15f + BldgHash * 0.05f,
		0.17f + BldgHash * 0.05f);
	FLinearColor RoofColor(
		0.1f + BldgHash * 0.03f,
		0.1f + BldgHash * 0.03f,
		0.12f + BldgHash * 0.03f);
	FLinearColor TrimColor(0.22f, 0.24f, 0.3f);   // Brighter metal trim
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
		FLinearColor(0.11f, 0.11f, 0.12f));

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
	FLinearColor PillarColor(0.15f, 0.16f, 0.18f);
	float PillarW = 120.f;
	for (float CX : {-HalfX, HalfX})
		for (float CY : {-HalfY, HalfY})
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(CX, CY, HalfZ)),
				FVector(PillarW / 100.f, PillarW / 100.f, Size.Z / 100.f + 0.2f),
				Rot, CubeMesh, PillarColor);

	// Base plinth — dark strip around building perimeter
	FLinearColor PlinthColor(0.06f, 0.065f, 0.07f);
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

	// Roof slab
	SpawnStaticMesh(Center + FVector(0.f, 0.f, Size.Z),
		FVector(Size.X / 100.f + 0.3f, Size.Y / 100.f + 0.3f, 0.3f), Rot, CubeMesh, RoofColor);
	// Parapet walls — short walls along roof edge for realistic silhouette
	float ParH = 80.f; // Parapet height
	float ParT = 30.f; // Parapet thickness
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 5.f, Size.Z + ParH * 0.5f)),
		FVector(Size.X / 100.f + 0.3f, ParT / 100.f, ParH / 100.f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 5.f, Size.Z + ParH * 0.5f)),
		FVector(Size.X / 100.f + 0.3f, ParT / 100.f, ParH / 100.f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX - 5.f, 0.f, Size.Z + ParH * 0.5f)),
		FVector(ParT / 100.f, Size.Y / 100.f + 0.3f, ParH / 100.f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX + 5.f, 0.f, Size.Z + ParH * 0.5f)),
		FVector(ParT / 100.f, Size.Y / 100.f + 0.3f, ParH / 100.f), Rot, CubeMesh, TrimColor);
	// Parapet cap — thin metal capping on top
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 5.f, Size.Z + ParH + 5.f)),
		FVector(Size.X / 100.f + 0.35f, ParT / 100.f + 0.1f, 0.1f), Rot, CubeMesh,
		FLinearColor(0.14f, 0.15f, 0.18f));

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
			if (!SM) return;
			SM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(AccentGlow.R * 1.2f, AccentGlow.G * 1.2f, AccentGlow.B * 1.2f));
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
	DoorLight->SetIntensity(5000.f);
	DoorLight->SetAttenuationRadius(1200.f);
	DoorLight->SetLightColor(FLinearColor(0.8f, 0.6f, 0.3f));
	DoorLight->CastShadows = false;
	DoorLight->RegisterComponent();

	UPointLightComponent* IntLight = NewObject<UPointLightComponent>(this);
	IntLight->SetupAttachment(RootComponent);
	IntLight->SetWorldLocation(Center + FVector(0.f, 0.f, Size.Z - 100.f));
	IntLight->SetIntensity(4000.f);
	IntLight->SetAttenuationRadius(FMath::Max(Size.X, Size.Y) * 0.6f);
	IntLight->SetLightColor(FLinearColor(0.5f, 0.6f, 0.8f));
	IntLight->CastShadows = false;
	IntLight->RegisterComponent();

	// Ceiling light fixtures — visible through glass windows
	int32 NumCeilLights = FMath::Max(1, FMath::RoundToInt32(Size.X / 2500.f));
	UMaterialInterface* CeilMat = FExoMaterialFactory::GetEmissiveOpaque();
	for (int32 cl = 0; cl < NumCeilLights; cl++)
	{
		float CLX = -HalfX * 0.6f + (HalfX * 1.2f * cl / FMath::Max(1, NumCeilLights - 1));
		UStaticMeshComponent* CeilPanel = SpawnStaticMesh(
			Center + Rot.RotateVector(FVector(CLX, 0.f, Size.Z - 20.f)),
			FVector(1.2f, 0.3f, 0.03f), Rot, CubeMesh,
			FLinearColor(0.3f, 0.35f, 0.4f));
		if (CeilPanel && CeilMat)
		{
			UMaterialInstanceDynamic* CM = UMaterialInstanceDynamic::Create(CeilMat, this);
			if (!CM) continue;
			CM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.6f, 0.7f, 0.9f));
			CeilPanel->SetMaterial(0, CM);
		}
	}

	// Recessed wall panels — inset darker sections create shadow depth
	FLinearColor PanelDark(0.06f, 0.065f, 0.08f);
	FLinearColor PanelLine(0.065f, 0.075f, 0.09f);
	int32 NumPanelsX = FMath::Max(2, FMath::RoundToInt32(Size.X / 2000.f));
	int32 NumPanelsZ = FMath::Max(1, FMath::RoundToInt32(Size.Z / 1200.f));
	float PanelGap = 60.f; // Gap between panels
	float PanelInset = 8.f; // How far panels are recessed into wall
	float PanelW = (Size.X - PanelGap * (NumPanelsX + 1)) / NumPanelsX;
	float PanelH = (Size.Z - PanelGap * (NumPanelsZ + 1)) / NumPanelsZ;
	for (int32 px = 0; px < NumPanelsX; px++)
	{
		float PX = -HalfX + PanelGap + PanelW * 0.5f + px * (PanelW + PanelGap);
		for (int32 pz = 0; pz < NumPanelsZ; pz++)
		{
			float PZ = PanelGap + PanelH * 0.5f + pz * (PanelH + PanelGap);
			// Front wall inset panel
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(PX, HalfY - PanelInset, PZ)),
				FVector(PanelW / 100.f, 0.02f, PanelH / 100.f), Rot, CubeMesh, PanelDark);
			// Back wall inset panel
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(PX, -HalfY + PanelInset, PZ)),
				FVector(PanelW / 100.f, 0.02f, PanelH / 100.f), Rot, CubeMesh, PanelDark);
		}
	}

	// Side wall recessed panels (left/right walls)
	int32 NumSidePanels = FMath::Max(1, FMath::RoundToInt32(Size.Y / 2000.f));
	float SidePanelW = (Size.Y - PanelGap * (NumSidePanels + 1)) / FMath::Max(1, NumSidePanels);
	for (int32 py = 0; py < NumSidePanels; py++)
	{
		float PY = -HalfY + PanelGap + SidePanelW * 0.5f + py * (SidePanelW + PanelGap);
		for (int32 pz = 0; pz < NumPanelsZ; pz++)
		{
			float PZ = PanelGap + PanelH * 0.5f + pz * (PanelH + PanelGap);
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX - PanelInset, PY, PZ)),
				FVector(0.02f, SidePanelW / 100.f, PanelH / 100.f), Rot, CubeMesh, PanelDark);
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX + PanelInset, PY, PZ)),
				FVector(0.02f, SidePanelW / 100.f, PanelH / 100.f), Rot, CubeMesh, PanelDark);
		}
	}

	// Horizontal panel lines at division heights
	for (float PZ : {Size.Z * 0.3f, Size.Z * 0.7f})
	{
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 1.f, PZ)),
			FVector(Size.X / 100.f * 0.95f, 0.015f, 0.04f), Rot, CubeMesh, PanelLine);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 1.f, PZ)),
			FVector(Size.X / 100.f * 0.95f, 0.015f, 0.04f), Rot, CubeMesh, PanelLine);
		// Side wall panel lines
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX + 1.f, 0.f, PZ)),
			FVector(0.015f, Size.Y / 100.f * 0.95f, 0.04f), Rot, CubeMesh, PanelLine);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX - 1.f, 0.f, PZ)),
			FVector(0.015f, Size.Y / 100.f * 0.95f, 0.04f), Rot, CubeMesh, PanelLine);
	}

	// Vertical wall ribs — structural mullions between panels
	float RibSpacing = Size.X / (NumPanelsX + 1);
	FLinearColor RibColor(0.1f, 0.11f, 0.13f);
	for (int32 r = 0; r <= NumPanelsX; r++)
	{
		float RX = -HalfX + PanelGap * 0.5f + r * (PanelW + PanelGap);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(RX, HalfY + 2.f, HalfZ)),
			FVector(PanelGap / 100.f, 0.06f, Size.Z / 100.f * 0.95f), Rot, CubeMesh, RibColor);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(RX, -HalfY - 2.f, HalfZ)),
			FVector(PanelGap / 100.f, 0.06f, Size.Z / 100.f * 0.95f), Rot, CubeMesh, RibColor);
	}

	// Rooftop equipment on larger buildings
	if (Size.X > 3000.f && Size.Y > 3000.f)
	{
		// Deterministic placement via position hash (no RandRange)
		float RoofSeed = FMath::Abs(FMath::Sin(Center.X * 0.00013f + Center.Y * 0.00019f));

		// HVAC unit — large box with exhaust grate
		FVector VentPos = Center + Rot.RotateVector(FVector(
			HalfX * (RoofSeed - 0.5f) * 0.5f,
			HalfY * (FMath::Frac(RoofSeed * 7.f) - 0.5f) * 0.5f, Size.Z + 60.f));
		SpawnStaticMesh(VentPos, FVector(1.5f, 1.f, 0.6f), Rot, CubeMesh,
			FLinearColor(0.08f, 0.08f, 0.1f));
		SpawnStaticMesh(VentPos + FVector(0.f, 0.f, 35.f),
			FVector(1.3f, 0.8f, 0.05f), Rot, CubeMesh, FLinearColor(0.06f, 0.06f, 0.07f));

		// Cylindrical exhaust fan with rim
		FVector FanPos = Center + Rot.RotateVector(FVector(
			HalfX * (FMath::Frac(RoofSeed * 3.f) - 0.5f) * 0.5f,
			HalfY * (FMath::Frac(RoofSeed * 11.f) - 0.5f) * 0.5f, Size.Z + 40.f));
		SpawnStaticMesh(FanPos, FVector(0.6f, 0.6f, 0.8f), Rot, CylinderMesh,
			FLinearColor(0.07f, 0.07f, 0.09f));
		SpawnStaticMesh(FanPos + FVector(0.f, 0.f, 42.f),
			FVector(0.7f, 0.7f, 0.04f), Rot, CylinderMesh,
			FLinearColor(0.1f, 0.1f, 0.12f));

		// Pipe stub protruding from roof (varies per building)
		if (RoofSeed > 0.4f)
		{
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(
				HalfX * 0.3f, -HalfY * 0.2f, Size.Z + 50.f)),
				FVector(0.15f, 0.15f, 1.f), Rot, CylinderMesh,
				FLinearColor(0.08f, 0.09f, 0.1f));
		}
	}

	// Exterior conduit — rectangular duct along back wall
	if (Size.X > 2500.f)
	{
		SpawnStaticMesh(
			Center + Rot.RotateVector(FVector(0.f, -HalfY - 20.f, Size.Z * 0.35f)),
			FVector(Size.X / 100.f * 0.7f, 0.15f, 0.12f), Rot, CubeMesh,
			FLinearColor(0.07f, 0.08f, 0.09f));
	}

	// Utility junction box on side wall
	SpawnStaticMesh(
		Center + Rot.RotateVector(FVector(HalfX + 15.f, HalfY * 0.3f, Size.Z * 0.25f)),
		FVector(0.12f, 0.5f, 0.4f), Rot, CubeMesh,
		FLinearColor(0.06f, 0.06f, 0.07f));

	// Window panels with frames — translucent glass with interior glow
	int32 NumWindows = FMath::Max(1, FMath::RoundToInt32(Size.X / 1500.f));
	float WinSpacing = Size.X / (NumWindows + 1);
	float WinZ = Size.Z * 0.4f;
	float FrameW = 8.f;
	FLinearColor FrameCol(0.05f, 0.055f, 0.065f);
	UMaterialInterface* GlassMat = FExoMaterialFactory::GetGlassTranslucent();
	for (int32 w = 0; w < NumWindows; w++)
	{
		float WX = -HalfX + WinSpacing * (w + 1);
		for (float Side : {HalfY + 2.f, -HalfY - 2.f})
		{
			// Translucent glass pane
			UStaticMeshComponent* WC = SpawnStaticMesh(
				Center + Rot.RotateVector(FVector(WX, Side, WinZ)),
				FVector(0.8f, 0.015f, 0.6f), Rot, CubeMesh,
				FLinearColor(0.02f, 0.03f, 0.05f));
			if (WC && GlassMat)
			{
				UMaterialInstanceDynamic* WM = UMaterialInstanceDynamic::Create(GlassMat, this);
				if (!WM) continue;
				WM->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.02f, 0.03f, 0.06f));
				WM->SetScalarParameterValue(TEXT("Metallic"), 0.1f);
				WM->SetScalarParameterValue(TEXT("Roughness"), 0.03f);
				WM->SetScalarParameterValue(TEXT("Specular"), 0.9f);
				WM->SetScalarParameterValue(TEXT("Opacity"), 0.35f);
				WM->SetVectorParameterValue(TEXT("EmissiveColor"), WinGlow);
				WC->SetMaterial(0, WM);
			}
			// Window frame — top and bottom bars
			float YOff = (Side > 0.f) ? 3.f : -3.f;
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(WX, Side + YOff, WinZ + 35.f)),
				FVector(0.85f, 0.03f, FrameW / 100.f), Rot, CubeMesh, FrameCol);
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(WX, Side + YOff, WinZ - 35.f)),
				FVector(0.85f, 0.03f, FrameW / 100.f), Rot, CubeMesh, FrameCol);
			// Window sill — slightly wider ledge below window
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(WX, Side + YOff * 2.f, WinZ - 38.f)),
				FVector(0.9f, 0.06f, 0.03f), Rot, CubeMesh,
				FLinearColor(0.08f, 0.085f, 0.1f));
			// Weathering streak below sill — rain staining
			SpawnStaticMesh(Center + Rot.RotateVector(FVector(WX, Side + YOff, WinZ - 60.f)),
				FVector(0.15f, 0.01f, 0.4f), Rot, CubeMesh,
				FLinearColor(0.035f, 0.035f, 0.04f));
		}
	}

	// Wall base grime — dark strip simulating dirt/water staining
	FLinearColor GrimeColor(0.03f, 0.03f, 0.035f);
	float GrimeH = 50.f;
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 2.f, GrimeH * 0.5f)),
		FVector(Size.X / 100.f * 0.95f, 0.01f, GrimeH / 100.f), Rot, CubeMesh, GrimeColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 2.f, GrimeH * 0.5f)),
		FVector(Size.X / 100.f * 0.95f, 0.01f, GrimeH / 100.f), Rot, CubeMesh, GrimeColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX + 2.f, 0.f, GrimeH * 0.5f)),
		FVector(0.01f, Size.Y / 100.f * 0.95f, GrimeH / 100.f), Rot, CubeMesh, GrimeColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX - 2.f, 0.f, GrimeH * 0.5f)),
		FVector(0.01f, Size.Y / 100.f * 0.95f, GrimeH / 100.f), Rot, CubeMesh, GrimeColor);

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
