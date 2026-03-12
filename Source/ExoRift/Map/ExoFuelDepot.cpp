// ExoFuelDepot.cpp — Industrial energy storage depot
#include "Map/ExoFuelDepot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Visual/ExoMaterialFactory.h"

AExoFuelDepot::AExoFuelDepot()
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

	// Materials created at runtime via FExoMaterialFactory

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
}

UStaticMeshComponent* AExoFuelDepot::AddPart(
	const FVector& Pos, const FVector& Scale, const FRotator& Rot,
	UStaticMesh* Mesh, const FLinearColor& Color)
{
	if (!Mesh) return nullptr;

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

	float Lum = Color.R * 0.3f + Color.G * 0.6f + Color.B * 0.1f;
	if (Lum > 0.15f)
	{
		UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();
		if (EmMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmMat, this);
			if (!Mat) { return nullptr; }
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Color.R * 2.f, Color.G * 2.f, Color.B * 2.f));
			Part->SetMaterial(0, Mat);
		}
	}
	else
	{
		UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
		if (LitMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, this);
			if (!Mat) { return nullptr; }
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Black);
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.85f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.3f);
			Part->SetMaterial(0, Mat);
		}
	}
	DepotParts.Add(Part);
	return Part;
}

void AExoFuelDepot::BuildDepot()
{
	FLinearColor Metal(0.06f, 0.06f, 0.08f);
	FLinearColor DarkMetal(0.04f, 0.04f, 0.05f);
	FLinearColor WarnYellow(0.6f, 0.5f, 0.05f);
	FLinearColor TankBlue(0.05f, 0.07f, 0.1f);

	// === GROUND PAD — concrete platform ===
	AddPart(FVector(0.f, 0.f, 10.f), FVector(30.f, 20.f, 0.2f),
		FRotator::ZeroRotator, CubeMesh, FLinearColor(0.05f, 0.05f, 0.055f));

	// === MAIN STORAGE TANKS — 3 large horizontal cylinders ===
	struct FTankDef { FVector Pos; float Radius; float Length; };
	FTankDef Tanks[] = {
		{{-800.f, 0.f, 500.f}, 4.f, 12.f},
		{{800.f, 0.f, 500.f}, 4.f, 12.f},
		{{0.f, 0.f, 1200.f}, 3.f, 14.f}, // Top tank (smaller)
	};

	for (int32 i = 0; i < 3; i++)
	{
		const FTankDef& T = Tanks[i];
		// Tank body
		UStaticMeshComponent* Body = AddPart(T.Pos,
			FVector(T.Radius, T.Radius, T.Length),
			FRotator(0.f, 0.f, 90.f), CylinderMesh, TankBlue);

		// End caps (spheres)
		float HalfLen = T.Length * 50.f;
		AddPart(T.Pos + FVector(0.f, HalfLen, 0.f),
			FVector(T.Radius * 0.9f), FRotator::ZeroRotator, SphereMesh, TankBlue);
		AddPart(T.Pos + FVector(0.f, -HalfLen, 0.f),
			FVector(T.Radius * 0.9f), FRotator::ZeroRotator, SphereMesh, TankBlue);

		// Tank support cradles (2 per tank)
		for (float Offset : {-300.f, 300.f})
		{
			AddPart(FVector(T.Pos.X, T.Pos.Y + Offset, T.Pos.Z * 0.4f),
				FVector(T.Radius * 1.2f, 0.3f, T.Pos.Z / 100.f),
				FRotator::ZeroRotator, CubeMesh, Metal);
		}

		// Glowing level indicator strip
		if (Body)
		{
			UStaticMeshComponent* Strip = AddPart(
				T.Pos + FVector(T.Radius * 50.f + 5.f, 0.f, 0.f),
				FVector(0.05f, T.Length * 0.8f, 0.15f),
				FRotator::ZeroRotator, CubeMesh,
				FLinearColor(0.1f, 0.6f, 0.3f));
			if (Strip)
			{
				UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
				UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(EmissiveMat, this);
				if (!SM) { return; }
				SM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(1.2f, 10.f, 3.5f));
				Strip->SetMaterial(0, SM);
				TankGlowMats.Add(SM);
			}
		}
	}

	// === PIPE NETWORK — connecting tanks to loading bay ===
	auto AddPipe = [&](const FVector& From, const FVector& To, float Radius)
	{
		FVector Dir = To - From;
		float Len = Dir.Size();
		FVector Mid = (From + To) * 0.5f;
		FRotator Rot = Dir.Rotation();
		AddPart(Mid, FVector(Radius, Radius, Len / 100.f),
			FRotator(0.f, Rot.Yaw, 90.f), CylinderMesh, Metal);
	};

	// Tank-to-tank connectors
	AddPipe(FVector(-800.f, 0.f, 500.f), FVector(800.f, 0.f, 500.f), 0.15f);
	// Vertical risers to top tank
	AddPipe(FVector(-400.f, 0.f, 500.f), FVector(-200.f, 0.f, 1200.f), 0.12f);
	AddPipe(FVector(400.f, 0.f, 500.f), FVector(200.f, 0.f, 1200.f), 0.12f);

	// === LOADING BAY — covered area with overhead structure ===
	// Roof
	AddPart(FVector(0.f, 1200.f, 600.f), FVector(10.f, 6.f, 0.15f),
		FRotator::ZeroRotator, CubeMesh, DarkMetal);
	// Support columns (4)
	for (float X : {-400.f, 400.f})
		for (float Y : {900.f, 1500.f})
			AddPart(FVector(X, Y, 300.f), FVector(0.3f, 0.3f, 6.f),
				FRotator::ZeroRotator, CylinderMesh, Metal);

	// === WARNING LIGHTS — amber rotating beacons at corners ===
	FLinearColor Amber(1.f, 0.6f, 0.1f);
	FVector LightPositions[] = {
		{1400.f, 900.f, 250.f}, {-1400.f, 900.f, 250.f},
		{1400.f, -900.f, 250.f}, {-1400.f, -900.f, 250.f},
	};
	for (const FVector& LP : LightPositions)
	{
		// Light post
		AddPart(LP, FVector(0.15f, 0.15f, 2.5f),
			FRotator::ZeroRotator, CylinderMesh, Metal);
		// Beacon bulb
		UStaticMeshComponent* Bulb = AddPart(LP + FVector(0.f, 0.f, 260.f),
			FVector(0.2f, 0.2f, 0.2f), FRotator::ZeroRotator, SphereMesh, Amber);
		if (Bulb)
		{
			UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* BM = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			if (!BM) { return; }
			BM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(Amber.R * 8.f, Amber.G * 8.f, Amber.B * 8.f));
			Bulb->SetMaterial(0, BM);
		}

		UPointLightComponent* WL = NewObject<UPointLightComponent>(this);
		WL->SetupAttachment(RootComponent);
		WL->SetRelativeLocation(LP + FVector(0.f, 0.f, 260.f));
		WL->SetIntensity(18000.f);
		WL->SetAttenuationRadius(2500.f);
		WL->SetLightColor(Amber);
		WL->CastShadows = false;
		WL->RegisterComponent();
		WarnLights.Add(WL);
	}

	// === HAZARD MARKINGS — yellow-black striped ground strips ===
	for (float Y = -800.f; Y <= 800.f; Y += 400.f)
	{
		AddPart(FVector(0.f, Y, 12.f), FVector(30.f, 0.3f, 0.02f),
			FRotator::ZeroRotator, CubeMesh, WarnYellow);
	}
}

void AExoFuelDepot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Pulse warning lights with staggered timing
	for (int32 i = 0; i < WarnLights.Num(); i++)
	{
		if (!WarnLights[i]) continue;
		float Phase = Time * 2.f + i * 1.57f; // Quarter-cycle offset each
		float Pulse = 0.3f + 0.7f * FMath::Max(0.f, FMath::Sin(Phase));
		WarnLights[i]->SetIntensity(18000.f * Pulse);
	}

	// Animate tank level indicators — slow breathe
	for (int32 i = 0; i < TankGlowMats.Num(); i++)
	{
		if (!TankGlowMats[i]) continue;
		float Phase = Time * 0.8f + i * 2.1f;
		float Level = 0.5f + 0.5f * FMath::Sin(Phase);
		TankGlowMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(1.2f * Level, 10.f * Level, 3.5f * Level));
	}
}
