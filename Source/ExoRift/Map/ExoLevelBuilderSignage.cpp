// ExoLevelBuilderSignage.cpp — Compound signage, landing pads, direction markers
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildSignage()
{
	FLinearColor CyanSign(0.1f, 0.5f, 1.f);
	FLinearColor GreenSign(0.1f, 0.8f, 0.3f);
	FLinearColor AmberSign(1.f, 0.6f, 0.1f);
	FLinearColor RedSign(1.f, 0.2f, 0.1f);
	FLinearColor WhiteSign(0.7f, 0.7f, 0.8f);

	// === COMPOUND SIGNS ===
	// Central Hub — "COMMAND" sign at entrance
	SpawnCompoundSign(FVector(4500.f, 0.f, 1500.f), 0.f, CyanSign);
	// North — "INDUSTRIAL" sign
	SpawnCompoundSign(FVector(0.f, 73000.f, 1500.f), 180.f, GreenSign);
	// South — "RESEARCH" sign
	SpawnCompoundSign(FVector(0.f, -73000.f, 1500.f), 0.f, CyanSign);
	// East — "POWER" sign
	SpawnCompoundSign(FVector(73000.f, 0.f, 1500.f), -90.f, AmberSign);
	// West — "BARRACKS" sign
	SpawnCompoundSign(FVector(-73000.f, 0.f, 1500.f), 90.f, RedSign);

	// === LANDING PADS ===
	// Hub helipad already exists — add ground markings
	SpawnLandingPad(FVector(0.f, 0.f, GroundZ + 3.f), 3000.f);
	// North compound landing pad
	SpawnLandingPad(FVector(5000.f, 85000.f, GroundZ + 3.f), 2500.f);
	// South research LZ
	SpawnLandingPad(FVector(-6000.f, -85000.f, GroundZ + 3.f), 2000.f);
	// East power station pad
	SpawnLandingPad(FVector(85000.f, 5000.f, GroundZ + 3.f), 2200.f);

	// === DIRECTION MARKERS ===
	// Road signs pointing between compounds
	FLinearColor ArrowColor(0.15f, 0.4f, 0.8f);

	// From hub, pointing to North
	SpawnDirectionMarker(FVector(0.f, 20000.f, GroundZ + 4.f), 0.f, ArrowColor);
	// From hub, pointing to South
	SpawnDirectionMarker(FVector(0.f, -20000.f, GroundZ + 4.f), 180.f, ArrowColor);
	// From hub, pointing to East
	SpawnDirectionMarker(FVector(20000.f, 0.f, GroundZ + 4.f), -90.f, ArrowColor);
	// From hub, pointing to West
	SpawnDirectionMarker(FVector(-20000.f, 0.f, GroundZ + 4.f), 90.f, ArrowColor);

	// Intersection markers along diagonals
	SpawnDirectionMarker(FVector(40000.f, 40000.f, GroundZ + 4.f), -45.f, ArrowColor);
	SpawnDirectionMarker(FVector(-40000.f, -40000.f, GroundZ + 4.f), 135.f, ArrowColor);
	SpawnDirectionMarker(FVector(-40000.f, 40000.f, GroundZ + 4.f), 45.f, ArrowColor);
	SpawnDirectionMarker(FVector(40000.f, -40000.f, GroundZ + 4.f), -135.f, ArrowColor);

	// === PERIMETER WARNING MARKERS ===
	// Large red warning stripes near map edge
	FLinearColor WarnRed(0.6f, 0.08f, 0.05f);
	float EdgeDist = MapHalfSize * 0.85f;
	int32 NumEdgeMarkers = 8;
	for (int32 i = 0; i < NumEdgeMarkers; i++)
	{
		float Angle = (2.f * PI * i) / NumEdgeMarkers;
		FVector Pos(FMath::Cos(Angle) * EdgeDist, FMath::Sin(Angle) * EdgeDist,
			GroundZ + 3.f);
		float Yaw = FMath::RadiansToDegrees(Angle) + 90.f;

		// Red warning stripe pair
		FVector Right = FRotationMatrix(FRotator(0.f, Yaw, 0.f)).GetScaledAxis(EAxis::Y);
		SpawnStaticMesh(Pos + Right * 400.f,
			FVector(0.5f, 30.f, 0.05f), FRotator(0.f, Yaw, 0.f),
			CubeMesh, WarnRed);
		SpawnStaticMesh(Pos - Right * 400.f,
			FVector(0.5f, 30.f, 0.05f), FRotator(0.f, Yaw, 0.f),
			CubeMesh, WarnRed);
	}

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Signage and markings placed"));
}

void AExoLevelBuilder::SpawnCompoundSign(const FVector& Pos, float Yaw,
	const FLinearColor& Color)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FLinearColor DarkMetal(0.05f, 0.05f, 0.07f);

	// Sign backing plate — dark rectangle
	SpawnStaticMesh(Pos, FVector(0.1f, 4.f, 1.2f), Rot, CubeMesh, DarkMetal);

	// Emissive text strip — glowing bar across the sign face
	FVector Forward = Rot.RotateVector(FVector(8.f, 0.f, 0.f));
	UStaticMeshComponent* TextBar = SpawnStaticMesh(
		Pos + Forward,
		FVector(0.02f, 3.2f, 0.3f), Rot, CubeMesh, Color);
	if (TextBar)
	{
		UMaterialInterface* SignEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* TBMat = UMaterialInstanceDynamic::Create(SignEmissiveMat, this);
		TBMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 4.f, Color.G * 4.f, Color.B * 4.f));
		TextBar->SetMaterial(0, TBMat);
	}

	// Top accent line
	UStaticMeshComponent* TopLine = SpawnStaticMesh(
		Pos + FVector(0.f, 0.f, 65.f) + Forward * 0.5f,
		FVector(0.02f, 3.8f, 0.04f), Rot, CubeMesh, Color);
	if (TopLine)
	{
		UMaterialInterface* LineEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
		UMaterialInstanceDynamic* TLMat = UMaterialInstanceDynamic::Create(LineEmissiveMat, this);
		TLMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
		TopLine->SetMaterial(0, TLMat);
	}

	// Support posts on each side
	SpawnStaticMesh(Pos + Rot.RotateVector(FVector(0.f, 180.f, -750.f)),
		FVector(0.15f, 0.15f, 15.f), FRotator::ZeroRotator, CylinderMesh, DarkMetal);
	SpawnStaticMesh(Pos + Rot.RotateVector(FVector(0.f, -180.f, -750.f)),
		FVector(0.15f, 0.15f, 15.f), FRotator::ZeroRotator, CylinderMesh, DarkMetal);

	// Glow light in front of sign
	UPointLightComponent* SignLight = NewObject<UPointLightComponent>(this);
	SignLight->SetupAttachment(RootComponent);
	SignLight->SetWorldLocation(Pos + Forward * 3.f);
	SignLight->SetIntensity(3000.f);
	SignLight->SetAttenuationRadius(800.f);
	SignLight->SetLightColor(Color);
	SignLight->CastShadows = false;
	SignLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnLandingPad(const FVector& Center, float Radius)
{
	FLinearColor PadColor(0.06f, 0.065f, 0.07f);
	FLinearColor LineColor(0.15f, 0.4f, 0.7f);
	FLinearColor CornerColor(0.8f, 0.5f, 0.1f);

	// Circular pad base
	float PadScale = Radius / 50.f;
	SpawnStaticMesh(Center, FVector(PadScale, PadScale, 0.04f),
		FRotator::ZeroRotator, CylinderMesh, PadColor);

	// Center H-marking — two vertical bars and crossbar
	float HSize = Radius * 0.3f;
	float BarW = HSize * 0.15f;

	UMaterialInterface* PadEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();

	// Left vertical bar of H
	UStaticMeshComponent* HL = SpawnStaticMesh(
		Center + FVector(-HSize * 0.3f, 0.f, 2.f),
		FVector(BarW / 100.f, 0.08f, HSize / 100.f),
		FRotator(90.f, 0.f, 0.f), CubeMesh, LineColor);
	if (HL)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(PadEmissiveMat, this);
		M->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(LineColor.R * 2.f, LineColor.G * 2.f, LineColor.B * 2.f));
		HL->SetMaterial(0, M);
	}

	// Right vertical bar of H
	UStaticMeshComponent* HR = SpawnStaticMesh(
		Center + FVector(HSize * 0.3f, 0.f, 2.f),
		FVector(BarW / 100.f, 0.08f, HSize / 100.f),
		FRotator(90.f, 0.f, 0.f), CubeMesh, LineColor);
	if (HR)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(PadEmissiveMat, this);
		M->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(LineColor.R * 2.f, LineColor.G * 2.f, LineColor.B * 2.f));
		HR->SetMaterial(0, M);
	}

	// Crossbar of H
	UStaticMeshComponent* HX = SpawnStaticMesh(
		Center + FVector(0.f, 0.f, 2.f),
		FVector(HSize * 0.5f / 100.f, 0.08f, BarW / 100.f),
		FRotator(90.f, 0.f, 0.f), CubeMesh, LineColor);
	if (HX)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(PadEmissiveMat, this);
		M->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(LineColor.R * 2.f, LineColor.G * 2.f, LineColor.B * 2.f));
		HX->SetMaterial(0, M);
	}

	// Corner markers (4 L-shaped brackets at cardinal points)
	for (int32 i = 0; i < 4; i++)
	{
		float Angle = i * PI * 0.5f + PI * 0.25f;
		float Dist = Radius * 0.75f;
		FVector Pos = Center + FVector(
			FMath::Cos(Angle) * Dist,
			FMath::Sin(Angle) * Dist, 2.f);

		float MarkYaw = FMath::RadiansToDegrees(Angle);
		SpawnStaticMesh(Pos,
			FVector(Radius * 0.1f / 100.f, 0.05f, 0.05f),
			FRotator(0.f, MarkYaw, 0.f), CubeMesh, CornerColor);
		SpawnStaticMesh(Pos,
			FVector(0.05f, Radius * 0.1f / 100.f, 0.05f),
			FRotator(0.f, MarkYaw, 0.f), CubeMesh, CornerColor);
	}

	// Pad edge light
	UPointLightComponent* PadLight = NewObject<UPointLightComponent>(this);
	PadLight->SetupAttachment(RootComponent);
	PadLight->SetWorldLocation(Center + FVector(0.f, 0.f, 50.f));
	PadLight->SetIntensity(2000.f);
	PadLight->SetAttenuationRadius(Radius * 1.5f);
	PadLight->SetLightColor(FLinearColor(0.15f, 0.4f, 0.7f));
	PadLight->CastShadows = false;
	PadLight->RegisterComponent();
}

void AExoLevelBuilder::SpawnDirectionMarker(const FVector& Pos, float Yaw,
	const FLinearColor& Color)
{
	FRotator Rot(0.f, Yaw, 0.f);

	// Arrow shaft — elongated rectangle on ground
	SpawnStaticMesh(Pos,
		FVector(8.f, 1.5f, 0.04f), Rot, CubeMesh, Color);

	// Arrow head — two angled bars forming a chevron
	FVector Forward = Rot.RotateVector(FVector(500.f, 0.f, 0.f));
	FVector Right = Rot.RotateVector(FVector(0.f, 200.f, 0.f));

	UStaticMeshComponent* ChevL = SpawnStaticMesh(
		Pos + Forward + Right * 0.5f,
		FVector(3.f, 0.4f, 0.04f),
		FRotator(0.f, Yaw + 30.f, 0.f), CubeMesh, Color);

	UStaticMeshComponent* ChevR = SpawnStaticMesh(
		Pos + Forward - Right * 0.5f,
		FVector(3.f, 0.4f, 0.04f),
		FRotator(0.f, Yaw - 30.f, 0.f), CubeMesh, Color);

	// Emissive on chevron tips
	UMaterialInterface* ChevEmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
	for (UStaticMeshComponent* Chev : {ChevL, ChevR})
	{
		if (Chev)
		{
			UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(ChevEmissiveMat, this);
			M->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 2.f, Color.G * 2.f, Color.B * 2.f));
			Chev->SetMaterial(0, M);
		}
	}
}
