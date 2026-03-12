// ExoLevelBuilderBuildings.cpp — SpawnBuilding with real 3D assets + cube fallback
#include "Map/ExoLevelBuilder.h"
#include "Map/ExoAutoSlidingDoor.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

void AExoLevelBuilder::SpawnBuilding(const FVector& Center, const FVector& Size, float Rotation)
{
	float HalfX = Size.X * 0.5f;
	float HalfY = Size.Y * 0.5f;
	float HalfZ = Size.Z * 0.5f;
	FRotator Rot(0.f, Rotation, 0.f);
	float DoorHalf = 600.f;
	float WallThickness = 80.f;

	// Position-based building color variation
	float BldgHash = FMath::Abs(FMath::Sin(Center.X * 0.00007f + Center.Y * 0.00011f));
	FLinearColor WallColor(
		0.14f + BldgHash * 0.06f, 0.15f + BldgHash * 0.05f, 0.17f + BldgHash * 0.05f);
	FLinearColor RoofColor(
		0.1f + BldgHash * 0.03f, 0.1f + BldgHash * 0.03f, 0.12f + BldgHash * 0.03f);
	FLinearColor TrimColor(0.22f, 0.24f, 0.3f);

	// Compound accent color
	FLinearColor AccentGlow(0.1f, 0.5f, 1.2f);
	FLinearColor WinGlow(0.06f, 0.15f, 0.35f);
	if (Center.Y > 8000.f)
	{	AccentGlow = FLinearColor(1.2f, 0.6f, 0.12f);
		WinGlow = FLinearColor(0.3f, 0.15f, 0.04f);
	}
	else if (Center.Y < -8000.f)
	{	AccentGlow = FLinearColor(0.15f, 1.0f, 0.7f);
		WinGlow = FLinearColor(0.04f, 0.25f, 0.18f);
	}
	else if (Center.X > 8000.f)
	{	AccentGlow = FLinearColor(1.2f, 0.25f, 0.15f);
		WinGlow = FLinearColor(0.3f, 0.06f, 0.04f);
	}
	else if (Center.X < -8000.f)
	{	AccentGlow = FLinearColor(0.2f, 1.0f, 0.3f);
		WinGlow = FLinearColor(0.05f, 0.25f, 0.08f);
	}

	// ─── REAL ASSET SHELL ───
	bool bUsedRealShell = false;

	if (bHasKenneyAssets)
	{
		// Pick best room module for building proportions
		UStaticMesh* RoomMesh = KN_RoomLarge;
		if (Size.X > Size.Y * 1.5f && KN_RoomWide) RoomMesh = KN_RoomWide;
		else if (Size.X < 2500.f && Size.Y < 2500.f && KN_RoomSmall) RoomMesh = KN_RoomSmall;
		if (RoomMesh)
		{
			FVector RoomScale(Size.X / 400.f, Size.Y / 400.f, Size.Z / 400.f);
			SpawnRawMesh(Center + FVector(0.f, 0.f, HalfZ), RoomScale, Rot, RoomMesh);
			bUsedRealShell = true;
		}
	}
	else if (bHasKayKitAssets && KK_BaseModule)
	{
		FVector ModScale(Size.X / 500.f, Size.Y / 500.f, Size.Z / 500.f);
		SpawnRawMesh(Center + FVector(0.f, 0.f, HalfZ), ModScale, Rot, KK_BaseModule);
		bUsedRealShell = true;
	}

	// Quaternius enhancements — columns, floor accent, roof detail
	if (bHasQuaterniusAssets)
	{
		float ColScale = Size.Z / 400.f;
		if (QT_Column1)
			for (float CX : {-HalfX, HalfX})
				for (float CY : {-HalfY, HalfY})
					SpawnRawMesh(Center + Rot.RotateVector(FVector(CX, CY, 0.f)),
						FVector(ColScale * 0.5f, ColScale * 0.5f, ColScale), Rot, QT_Column1);
		if (QT_FloorBasic && !bUsedRealShell)
			SpawnRawMesh(Center + FVector(0.f, 0.f, 5.f),
				FVector(Size.X / 500.f, Size.Y / 500.f, 0.3f), Rot, QT_FloorBasic);
		if (QT_RoofPipes)
			SpawnRawMesh(Center + FVector(0.f, 0.f, Size.Z + 10.f),
				FVector(Size.X / 600.f, Size.Y / 600.f, 0.5f), Rot, QT_RoofPipes);
		if (QT_DetailVent1 && Size.X > 3000.f)
			SpawnRawMesh(Center + Rot.RotateVector(
				FVector(HalfX * 0.3f, -HalfY * 0.3f, Size.Z + 30.f)),
				FVector(1.f), Rot, QT_DetailVent1);
	}

	// Door mesh at entrance
	UStaticMesh* DoorAsset = nullptr;
	if (bHasSciFiDoorAsset && SF_Door) DoorAsset = SF_Door;
	else if (bHasQuaterniusAssets && QT_DoorSingle) DoorAsset = QT_DoorSingle;
	else if (bHasKenneyAssets && KN_Door) DoorAsset = KN_Door;
	if (DoorAsset)
		SpawnRawMesh(Center + Rot.RotateVector(FVector(0.f, HalfY, 0.f)),
			FVector(1.5f, 1.5f, Size.Z / 400.f), Rot, DoorAsset);

	// ─── PRIMITIVE FALLBACK (only when no real shell) ───
	if (!bUsedRealShell)
	{
		// Floor
		SpawnStaticMesh(Center + FVector(0.f, 0.f, 10.f),
			FVector(Size.X / 100.f, Size.Y / 100.f, 0.2f), Rot, CubeMesh,
			FLinearColor(0.11f, 0.11f, 0.12f));
		// Back wall
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY, HalfZ)),
			FVector(Size.X / 100.f, WallThickness / 100.f, Size.Z / 100.f),
			Rot, CubeMesh, WallColor);
		// Left wall
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(-HalfX, 0.f, HalfZ)),
			FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f),
			Rot, CubeMesh, WallColor);
		// Right wall
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(HalfX, 0.f, HalfZ)),
			FVector(WallThickness / 100.f, Size.Y / 100.f, Size.Z / 100.f),
			Rot, CubeMesh, WallColor);
		// Front wall — two segments with door gap
		float SegX_L = (-HalfX + (-DoorHalf)) * 0.5f;
		float SegX_R = (DoorHalf + HalfX) * 0.5f;
		float SegW_L = (-DoorHalf) - (-HalfX);
		float SegW_R = HalfX - DoorHalf;
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_L, HalfY, HalfZ)),
			FVector(SegW_L / 100.f, WallThickness / 100.f, Size.Z / 100.f),
			Rot, CubeMesh, WallColor);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(SegX_R, HalfY, HalfZ)),
			FVector(SegW_R / 100.f, WallThickness / 100.f, Size.Z / 100.f),
			Rot, CubeMesh, WallColor);
		// Door frame header + side posts
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY, Size.Z - 200.f)),
			FVector(DoorHalf * 2.f / 100.f, WallThickness / 100.f + 0.1f, 2.f),
			Rot, CubeMesh, TrimColor);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(-DoorHalf, HalfY, HalfZ * 0.8f)),
			FVector(0.6f, WallThickness / 100.f + 0.15f, Size.Z / 100.f * 0.8f),
			Rot, CubeMesh, TrimColor);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(DoorHalf, HalfY, HalfZ * 0.8f)),
			FVector(0.6f, WallThickness / 100.f + 0.15f, Size.Z / 100.f * 0.8f),
			Rot, CubeMesh, TrimColor);
		// Corner pillars
		FLinearColor PillarColor(0.15f, 0.16f, 0.18f);
		for (float CX : {-HalfX, HalfX})
			for (float CY : {-HalfY, HalfY})
				SpawnStaticMesh(Center + Rot.RotateVector(FVector(CX, CY, HalfZ)),
					FVector(1.2f, 1.2f, Size.Z / 100.f + 0.2f), Rot, CubeMesh, PillarColor);
		// Base plinth
		FLinearColor PlinthColor(0.06f, 0.065f, 0.07f);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY, 20.f)),
			FVector(Size.X / 100.f + 0.2f, 0.3f, 0.4f), Rot, CubeMesh, PlinthColor);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY, 20.f)),
			FVector(Size.X / 100.f + 0.2f, 0.3f, 0.4f), Rot, CubeMesh, PlinthColor);
		// Front entrance step
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 60.f, 15.f)),
			FVector(DoorHalf * 2.f / 100.f + 0.5f, 1.2f, 0.3f), Rot, CubeMesh,
			FLinearColor(0.065f, 0.07f, 0.08f));
		// Roof slab + front/back parapet
		SpawnStaticMesh(Center + FVector(0.f, 0.f, Size.Z),
			FVector(Size.X / 100.f + 0.3f, Size.Y / 100.f + 0.3f, 0.3f),
			Rot, CubeMesh, RoofColor);
		float ParH = 80.f;
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 5.f, Size.Z + ParH * 0.5f)),
			FVector(Size.X / 100.f + 0.3f, 0.3f, ParH / 100.f), Rot, CubeMesh, TrimColor);
		SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 5.f, Size.Z + ParH * 0.5f)),
			FVector(Size.X / 100.f + 0.3f, 0.3f, ParH / 100.f), Rot, CubeMesh, TrimColor);

		// Windows — glass panes with frame
		int32 NumWindows = FMath::Max(1, FMath::RoundToInt32(Size.X / 1500.f));
		float WinSpacing = Size.X / (NumWindows + 1);
		float WinZ = Size.Z * 0.4f;
		UMaterialInterface* GlassMat = FExoMaterialFactory::GetGlassTranslucent();
		for (int32 w = 0; w < NumWindows; w++)
		{
			float WX = -HalfX + WinSpacing * (w + 1);
			for (float Side : {HalfY + 2.f, -HalfY - 2.f})
			{
				UStaticMeshComponent* WC = SpawnStaticMesh(
					Center + Rot.RotateVector(FVector(WX, Side, WinZ)),
					FVector(0.8f, 0.015f, 0.6f), Rot, CubeMesh,
					FLinearColor(0.02f, 0.03f, 0.05f));
				if (WC && GlassMat)
				{
					UMaterialInstanceDynamic* WM =
						UMaterialInstanceDynamic::Create(GlassMat, this);
					if (!WM) continue;
					WM->SetVectorParameterValue(TEXT("BaseColor"),
						FLinearColor(0.02f, 0.03f, 0.06f));
					WM->SetScalarParameterValue(TEXT("Metallic"), 0.1f);
					WM->SetScalarParameterValue(TEXT("Roughness"), 0.03f);
					WM->SetScalarParameterValue(TEXT("Specular"), 0.9f);
					WM->SetScalarParameterValue(TEXT("Opacity"), 0.35f);
					WM->SetVectorParameterValue(TEXT("EmissiveColor"), WinGlow);
					WC->SetMaterial(0, WM);
				}
				float YOff = (Side > 0.f) ? 3.f : -3.f;
				SpawnStaticMesh(Center + Rot.RotateVector(FVector(WX, Side + YOff, WinZ + 35.f)),
					FVector(0.85f, 0.03f, 0.08f), Rot, CubeMesh,
					FLinearColor(0.05f, 0.055f, 0.065f));
				SpawnStaticMesh(Center + Rot.RotateVector(FVector(WX, Side + YOff, WinZ - 35.f)),
					FVector(0.85f, 0.03f, 0.08f), Rot, CubeMesh,
					FLinearColor(0.05f, 0.055f, 0.065f));
			}
		}

		// Rooftop equipment on larger buildings
		if (Size.X > 3000.f && Size.Y > 3000.f)
		{
			float RoofSeed = FMath::Abs(FMath::Sin(
				Center.X * 0.00013f + Center.Y * 0.00019f));
			FVector VentPos = Center + Rot.RotateVector(FVector(
				HalfX * (RoofSeed - 0.5f) * 0.5f,
				HalfY * (FMath::Frac(RoofSeed * 7.f) - 0.5f) * 0.5f, Size.Z + 60.f));
			SpawnStaticMesh(VentPos, FVector(1.5f, 1.f, 0.6f), Rot, CubeMesh,
				FLinearColor(0.08f, 0.08f, 0.1f));
			FVector FanPos = Center + Rot.RotateVector(FVector(
				HalfX * (FMath::Frac(RoofSeed * 3.f) - 0.5f) * 0.5f,
				HalfY * (FMath::Frac(RoofSeed * 11.f) - 0.5f) * 0.5f, Size.Z + 40.f));
			SpawnStaticMesh(FanPos, FVector(0.6f, 0.6f, 0.8f), Rot, CylinderMesh,
				FLinearColor(0.07f, 0.07f, 0.09f));
		}
	}

	// ─── COMMON: Accent emissive strips ───
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

	// ─── COMMON: Lights ───
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

	// Ceiling light fixtures
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

	// ─── COMMON: Auto-sliding door ───
	FVector DoorPos = Center + Rot.RotateVector(FVector(0.f, HalfY, 0.f));
	float DoorH = FMath::Min(Size.Z - 200.f, 350.f);
	AExoAutoSlidingDoor* Door = GetWorld()->SpawnActor<AExoAutoSlidingDoor>(
		AExoAutoSlidingDoor::StaticClass(), DoorPos, Rot);
	if (Door)
		Door->InitDoor(DoorHalf * 2.f, DoorH, DoorHalf * 0.9f, AccentGlow);
}
