// ExoLevelBuilderTunnels.cpp — Underground tunnel network between compounds
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildTunnels()
{
	// === Underground tunnel entrances at each compound ===

	// Hub tunnel entrances (4 directions)
	SpawnTunnelEntrance(FVector(0.f, 10000.f, 0.f), 0.f);   // North exit
	SpawnTunnelEntrance(FVector(0.f, -10000.f, 0.f), 180.f); // South exit
	SpawnTunnelEntrance(FVector(10000.f, 0.f, 0.f), 90.f);   // East exit
	SpawnTunnelEntrance(FVector(-10000.f, 0.f, 0.f), 270.f);  // West exit

	// Compound tunnel entrances
	SpawnTunnelEntrance(FVector(0.f, 80000.f - 10000.f, 0.f), 180.f); // North compound
	SpawnTunnelEntrance(FVector(0.f, -80000.f + 10000.f, 0.f), 0.f);   // South compound
	SpawnTunnelEntrance(FVector(80000.f - 8000.f, 0.f, 0.f), 270.f);  // East compound
	SpawnTunnelEntrance(FVector(-80000.f + 6000.f, 0.f, 0.f), 90.f);  // West compound

	// === Tunnel segments connecting compounds to hub ===

	// Hub to North (along Y axis)
	SpawnTunnelSegment(FVector(0.f, 10000.f, -300.f), FVector(0.f, 70000.f, -300.f));

	// Hub to South
	SpawnTunnelSegment(FVector(0.f, -10000.f, -300.f), FVector(0.f, -70000.f, -300.f));

	// Hub to East
	SpawnTunnelSegment(FVector(10000.f, 0.f, -300.f), FVector(72000.f, 0.f, -300.f));

	// Hub to West
	SpawnTunnelSegment(FVector(-10000.f, 0.f, -300.f), FVector(-74000.f, 0.f, -300.f));

	UE_LOG(LogExoRift, Log, TEXT("LevelBuilder: Tunnel network placed"));
}

void AExoLevelBuilder::SpawnTunnelEntrance(const FVector& Pos, float Yaw)
{
	FRotator Rot(0.f, Yaw, 0.f);
	FVector Forward = Rot.RotateVector(FVector(1.f, 0.f, 0.f));
	FVector Right = Rot.RotateVector(FVector(0.f, 1.f, 0.f));

	float EntranceW = 800.f;
	float EntranceH = 500.f;
	float RampLen = 2000.f;

	// Ramp going down (5 step approximation)
	int32 Steps = 8;
	float StepLen = RampLen / Steps;
	float StepDrop = 400.f / Steps; // Goes 400 units below ground
	for (int32 i = 0; i < Steps; i++)
	{
		FVector StepPos = Pos + Forward * (StepLen * (i + 0.5f))
			- FVector(0.f, 0.f, StepDrop * (i + 0.5f));
		SpawnStaticMesh(StepPos,
			FVector(StepLen / 100.f, EntranceW / 100.f, 0.15f), Rot, CubeMesh,
			FLinearColor(0.05f, 0.055f, 0.065f));
	}

	// Side walls along the ramp
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		FVector WallBase = Pos + Right * (EntranceW * 0.5f * Side);
		FVector WallEnd = Pos + Forward * RampLen + Right * (EntranceW * 0.5f * Side)
			- FVector(0.f, 0.f, 400.f);
		FVector WallMid = (WallBase + WallEnd) * 0.5f + FVector(0.f, 0.f, EntranceH * 0.5f);
		float WallLen = FVector::Dist(WallBase, WallEnd);
		FRotator WallRot = (WallEnd - WallBase).Rotation();
		SpawnStaticMesh(WallMid,
			FVector(WallLen / 100.f, 0.3f, EntranceH / 100.f), WallRot, CubeMesh,
			FLinearColor(0.06f, 0.065f, 0.08f));
	}

	// Archway frame over the entrance
	SpawnStaticMesh(Pos + FVector(0.f, 0.f, EntranceH),
		FVector(0.5f, EntranceW / 100.f, 0.3f), Rot, CubeMesh,
		FLinearColor(0.08f, 0.08f, 0.1f));

	// Entrance sign — red warning light
	UPointLightComponent* EntranceLight = NewObject<UPointLightComponent>(this);
	EntranceLight->SetupAttachment(RootComponent);
	EntranceLight->SetWorldLocation(Pos + FVector(0.f, 0.f, EntranceH - 50.f));
	EntranceLight->SetIntensity(3000.f);
	EntranceLight->SetAttenuationRadius(800.f);
	EntranceLight->SetLightColor(FLinearColor(1.f, 0.4f, 0.1f));
	EntranceLight->CastShadows = false;
	EntranceLight->RegisterComponent();

	// Warning stripe on archway
	UStaticMeshComponent* Stripe = SpawnStaticMesh(
		Pos + FVector(0.f, 0.f, EntranceH + 10.f),
		FVector(0.6f, EntranceW / 100.f, 0.08f), Rot, CubeMesh,
		FLinearColor(0.8f, 0.4f, 0.05f));
	if (Stripe)
	{
		UMaterialInstanceDynamic* StMat = Cast<UMaterialInstanceDynamic>(
			Stripe->GetMaterial(0));
		if (StMat) StMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(2.f, 0.8f, 0.1f));
	}
}

void AExoLevelBuilder::SpawnTunnelSegment(const FVector& Start, const FVector& End)
{
	FVector Dir = End - Start;
	float Length = Dir.Size();
	FRotator Rot = Dir.Rotation();
	FVector Mid = (Start + End) * 0.5f;

	float TunnelW = 800.f;
	float TunnelH = 400.f;

	// Floor
	SpawnStaticMesh(Mid,
		FVector(Length / 100.f, TunnelW / 100.f, 0.15f), Rot, CubeMesh,
		FLinearColor(0.04f, 0.045f, 0.055f));

	// Ceiling
	SpawnStaticMesh(Mid + FVector(0.f, 0.f, TunnelH),
		FVector(Length / 100.f, TunnelW / 100.f, 0.15f), Rot, CubeMesh,
		FLinearColor(0.05f, 0.055f, 0.06f));

	// Side walls
	FVector Right = FRotationMatrix(Rot).GetUnitAxis(EAxis::Y);
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		SpawnStaticMesh(Mid + Right * (TunnelW * 0.5f * Side) + FVector(0.f, 0.f, TunnelH * 0.5f),
			FVector(Length / 100.f, 0.15f, TunnelH / 100.f), Rot, CubeMesh,
			FLinearColor(0.055f, 0.06f, 0.07f));
	}

	// Overhead lights every 4000 units
	FVector Forward = Dir.GetSafeNormal();
	int32 LightCount = FMath::Max(2, FMath::RoundToInt32(Length / 4000.f));
	for (int32 i = 0; i < LightCount; i++)
	{
		float T = (float)(i + 0.5f) / LightCount;
		FVector LightPos = FMath::Lerp(Start, End, T) + FVector(0.f, 0.f, TunnelH - 30.f);

		// Light fixture (small box)
		SpawnStaticMesh(LightPos,
			FVector(0.3f, 0.8f, 0.05f), Rot, CubeMesh,
			FLinearColor(0.12f, 0.12f, 0.15f));

		// Emissive light panel
		UStaticMeshComponent* LightPanel = SpawnStaticMesh(
			LightPos - FVector(0.f, 0.f, 5.f),
			FVector(0.2f, 0.6f, 0.02f), Rot, CubeMesh,
			FLinearColor(0.7f, 0.8f, 1.f));
		if (LightPanel)
		{
			UMaterialInstanceDynamic* LPMat = Cast<UMaterialInstanceDynamic>(
				LightPanel->GetMaterial(0));
			if (LPMat) LPMat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(3.f, 3.5f, 5.f));
		}

		// Point light
		UPointLightComponent* TunnelLight = NewObject<UPointLightComponent>(this);
		TunnelLight->SetupAttachment(RootComponent);
		TunnelLight->SetWorldLocation(LightPos - FVector(0.f, 0.f, 20.f));
		TunnelLight->SetIntensity(3000.f);
		TunnelLight->SetAttenuationRadius(2000.f);
		TunnelLight->SetLightColor(FLinearColor(0.6f, 0.7f, 1.f));
		TunnelLight->CastShadows = false;
		TunnelLight->RegisterComponent();
	}

	// Support ribs — structural cross-beams every 3000 units
	int32 RibCount = FMath::Max(2, FMath::RoundToInt32(Length / 3000.f));
	for (int32 i = 0; i < RibCount; i++)
	{
		float T = (float)(i + 0.5f) / RibCount;
		FVector RibPos = FMath::Lerp(Start, End, T);

		// Vertical pillars on each side
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			FVector PillarPos = RibPos + Right * (TunnelW * 0.45f * Side)
				+ FVector(0.f, 0.f, TunnelH * 0.5f);
			SpawnStaticMesh(PillarPos,
				FVector(0.12f, 0.12f, TunnelH / 100.f), FRotator::ZeroRotator, CylinderMesh,
				FLinearColor(0.08f, 0.085f, 0.1f));
		}

		// Ceiling cross-beam
		SpawnStaticMesh(RibPos + FVector(0.f, 0.f, TunnelH - 10.f),
			FVector(0.12f, TunnelW / 100.f, 0.12f), Rot, CubeMesh,
			FLinearColor(0.08f, 0.085f, 0.1f));
	}
}
