// ExoLevelBuilderBuildings.cpp — SpawnBuilding with walls, emissives, windows
#include "Map/ExoLevelBuilder.h"
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

	// Roof + trim
	SpawnStaticMesh(Center + FVector(0.f, 0.f, Size.Z),
		FVector(Size.X / 100.f + 0.3f, Size.Y / 100.f + 0.3f, 0.3f), Rot, CubeMesh, RoofColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, HalfY + 15.f, Size.Z + 20.f)),
		FVector(Size.X / 100.f + 0.4f, 0.1f, 0.4f), Rot, CubeMesh, TrimColor);
	SpawnStaticMesh(Center + Rot.RotateVector(FVector(0.f, -HalfY - 15.f, Size.Z + 20.f)),
		FVector(Size.X / 100.f + 0.4f, 0.1f, 0.4f), Rot, CubeMesh, TrimColor);

	// Emissive accent strips on front/back walls
	float StripZ = Size.Z * 0.6f;
	FLinearColor StripCol(0.05f, 0.15f, 0.3f);
	auto MakeStrip = [&](const FVector& Offset, const FVector& Scale)
	{
		UStaticMeshComponent* S = SpawnStaticMesh(
			Center + Rot.RotateVector(Offset), Scale, Rot, CubeMesh, StripCol);
		if (S && BaseMaterial)
		{
			UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			SM->SetVectorParameterValue(TEXT("BaseColor"), StripCol);
			SM->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.1f, 0.5f, 1.2f));
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
	}

	// Window panels along walls
	int32 NumWindows = FMath::Max(1, FMath::RoundToInt32(Size.X / 1500.f));
	float WinSpacing = Size.X / (NumWindows + 1);
	float WinZ = Size.Z * 0.4f;
	FLinearColor WinGlow(0.03f, 0.06f, 0.12f);
	for (int32 w = 0; w < NumWindows; w++)
	{
		float WX = -HalfX + WinSpacing * (w + 1);
		for (float Side : {HalfY + 2.f, -HalfY - 2.f})
		{
			UStaticMeshComponent* WC = SpawnStaticMesh(
				Center + Rot.RotateVector(FVector(WX, Side, WinZ)),
				FVector(0.8f, 0.015f, 0.6f), Rot, CubeMesh, WinGlow);
			if (WC && BaseMaterial)
			{
				UMaterialInstanceDynamic* WM = Cast<UMaterialInstanceDynamic>(WC->GetMaterial(0));
				if (WM) WM->SetVectorParameterValue(TEXT("EmissiveColor"),
					FLinearColor(0.06f, 0.15f, 0.35f));
			}
		}
	}
}
