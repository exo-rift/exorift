// ExoZipline.cpp — Traversal zipline with glowing energy cable
#include "Map/ExoZipline.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoZipline::AExoZipline()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // 20Hz for energy pulse

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFind.Succeeded()) SphereMesh = SphereFind.Object;
}

void AExoZipline::InitZipline(const FVector& InStart, const FVector& InEnd)
{
	StartPoint = InStart;
	EndPoint = InEnd;
	CableLength = FVector::Distance(StartPoint, EndPoint);
	if (CableLength < 100.f) return;

	SetActorLocation(StartPoint);

	FVector Dir = (EndPoint - StartPoint).GetSafeNormal();
	FRotator CableRot = Dir.Rotation();
	CableRot.Pitch += 90.f; // Cylinder default is vertical

	UMaterialInterface* LitMat = FExoMaterialFactory::GetLitEmissive();
	UMaterialInterface* EmissiveMat = FExoMaterialFactory::GetEmissiveAdditive();

	// Cable segments — thin metallic cylinders
	float SegLen = CableLength / CABLE_SEGMENTS;
	for (int32 i = 0; i < CABLE_SEGMENTS; i++)
	{
		float T0 = (float)i / CABLE_SEGMENTS;
		float T1 = (float)(i + 1) / CABLE_SEGMENTS;
		FVector SegStart = FMath::Lerp(StartPoint, EndPoint, T0);
		FVector SegEnd = FMath::Lerp(StartPoint, EndPoint, T1);

		// Slight catenary sag
		float MidT = (T0 + T1) * 0.5f;
		float Sag = FMath::Sin(MidT * PI) * CableLength * 0.02f;
		FVector Center = (SegStart + SegEnd) * 0.5f - FVector(0.f, 0.f, Sag);

		UStaticMeshComponent* Seg = NewObject<UStaticMeshComponent>(this);
		Seg->SetupAttachment(RootComponent);
		Seg->SetStaticMesh(CubeMesh);
		Seg->SetWorldLocation(Center);
		Seg->SetWorldRotation(CableRot);
		float LenScale = SegLen / 100.f;
		Seg->SetWorldScale3D(FVector(0.04f, 0.04f, LenScale * 0.52f));
		Seg->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Seg->CastShadow = false;
		Seg->RegisterComponent();

		if (LitMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(LitMat, this);
			if (!Mat) { return; }
			Mat->SetVectorParameterValue(TEXT("BaseColor"),
				FLinearColor(0.15f, 0.15f, 0.18f));
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(1.f, 4.f, 2.f)); // Green energy glow
			Mat->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
			Mat->SetScalarParameterValue(TEXT("Roughness"), 0.15f);
			Seg->SetMaterial(0, Mat);
		}

		CableSegments.Add(Seg);
	}

	// Energy nodes — pulsing spheres along the cable
	for (int32 i = 0; i < ENERGY_NODES; i++)
	{
		float T = (float)(i + 1) / (ENERGY_NODES + 1);
		FVector Pos = GetPositionAtT(T);

		UStaticMeshComponent* Node = NewObject<UStaticMeshComponent>(this);
		Node->SetupAttachment(RootComponent);
		Node->SetStaticMesh(SphereMesh);
		Node->SetWorldLocation(Pos);
		Node->SetWorldScale3D(FVector(0.12f));
		Node->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Node->CastShadow = false;
		Node->RegisterComponent();

		if (EmissiveMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			if (!Mat) { return; }
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(4.f, 16.f, 6.f));
			Node->SetMaterial(0, Mat);
		}

		EnergyNodes.Add(Node);

		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetWorldLocation(Pos);
		Light->SetIntensity(4000.f);
		Light->SetAttenuationRadius(600.f);
		Light->SetLightColor(FLinearColor(0.2f, 1.f, 0.4f));
		Light->CastShadows = false;
		Light->RegisterComponent();
		NodeLights.Add(Light);
	}

	// Endpoint interaction spheres — bright green "mount here" hint
	auto MakeEndpoint = [&](const FVector& Pos, UStaticMeshComponent*& OutSphere,
		UPointLightComponent*& OutLight)
	{
		OutSphere = NewObject<UStaticMeshComponent>(this);
		OutSphere->SetupAttachment(RootComponent);
		OutSphere->SetStaticMesh(SphereMesh);
		OutSphere->SetWorldLocation(Pos);
		OutSphere->SetWorldScale3D(FVector(0.2f));
		OutSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		OutSphere->CastShadow = false;
		OutSphere->RegisterComponent();

		if (EmissiveMat)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(EmissiveMat, this);
			if (!Mat) { return; }
			Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(6.f, 20.f, 8.f));
			OutSphere->SetMaterial(0, Mat);
		}

		OutLight = NewObject<UPointLightComponent>(this);
		OutLight->SetupAttachment(RootComponent);
		OutLight->SetWorldLocation(Pos);
		OutLight->SetIntensity(8000.f);
		OutLight->SetAttenuationRadius(800.f);
		OutLight->SetLightColor(FLinearColor(0.2f, 1.f, 0.4f));
		OutLight->CastShadows = false;
		OutLight->RegisterComponent();
	};

	MakeEndpoint(StartPoint, StartSphere, StartLight);
	MakeEndpoint(EndPoint, EndSphere, EndLight);
}

FVector AExoZipline::GetPositionAtT(float T) const
{
	FVector Pos = FMath::Lerp(StartPoint, EndPoint, T);
	// Catenary sag
	float Sag = FMath::Sin(T * PI) * CableLength * 0.02f;
	Pos.Z -= Sag;
	return Pos;
}

FVector AExoZipline::GetDirection() const
{
	return (EndPoint - StartPoint).GetSafeNormal();
}

void AExoZipline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	// Pulse energy nodes
	for (int32 i = 0; i < EnergyNodes.Num(); i++)
	{
		if (!EnergyNodes[i]) continue;
		float Phase = Time * 2.f + i * 1.5f;
		float Pulse = 0.7f + 0.3f * FMath::Sin(Phase);
		float S = 0.12f * Pulse;
		EnergyNodes[i]->SetWorldScale3D(FVector(S));

		if (i < NodeLights.Num() && NodeLights[i])
		{
			NodeLights[i]->SetIntensity(4000.f * Pulse);
		}
	}

	// Pulse endpoint spheres
	float EndPulse = 0.8f + 0.2f * FMath::Sin(Time * 3.f);
	if (StartSphere)
		StartSphere->SetWorldScale3D(FVector(0.2f * EndPulse));
	if (EndSphere)
		EndSphere->SetWorldScale3D(FVector(0.2f * EndPulse));
	if (StartLight)
		StartLight->SetIntensity(8000.f * EndPulse);
	if (EndLight)
		EndLight->SetIntensity(8000.f * EndPulse);
}

AExoZipline* AExoZipline::SpawnZipline(UWorld* World, const FVector& Start,
	const FVector& End)
{
	if (!World) return nullptr;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AExoZipline* Zip = World->SpawnActor<AExoZipline>(
		AExoZipline::StaticClass(), Start, FRotator::ZeroRotator, Params);
	if (Zip) Zip->InitZipline(Start, End);
	return Zip;
}
