#include "Map/ExoDropPod.h"
#include "Map/ExoDropPodManager.h"
#include "Player/ExoCharacter.h"
#include "Visual/ExoTracerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "ExoRift.h"

AExoDropPod::AExoDropPod()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root scene
	PodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PodRoot"));
	RootComponent = PodMesh;
	PodMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PodMesh->SetVisibility(false); // Root is invisible, children are visible

	PodCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PodCamera"));
	PodCamera->SetupAttachment(PodMesh);
	PodCamera->SetRelativeLocation(FVector(-400.f, 0.f, 250.f));
	PodCamera->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));

	ThrusterLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ThrusterLight"));
	ThrusterLight->SetupAttachment(PodMesh);
	ThrusterLight->SetRelativeLocation(FVector(0.f, 0.f, -150.f));
	ThrusterLight->SetIntensity(0.f);
	ThrusterLight->SetAttenuationRadius(3000.f);
	ThrusterLight->SetLightColor(FColor(80, 160, 255)); // Blue thruster glow

	// Cache engine meshes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));
	if (CubeFind.Succeeded()) CubeMesh = CubeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFind(
		TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylFind.Succeeded()) CylinderMesh = CylFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeFind(
		TEXT("/Engine/BasicShapes/Cone"));
	if (ConeFind.Succeeded()) ConeMesh = ConeFind.Object;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFind(
		TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFind.Succeeded()) SphereMesh = SphereFind.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFind(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFind.Succeeded()) BaseMaterial = MatFind.Object;
}

void AExoDropPod::BuildPodMesh()
{
	auto MakeComp = [&](UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FLinearColor& Color, const FRotator& Rot = FRotator::ZeroRotator) -> UStaticMeshComponent*
	{
		if (!Mesh) return nullptr;
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		C->SetupAttachment(RootComponent);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeScale3D(Scale);
		C->SetRelativeRotation(Rot);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = true;
		C->RegisterComponent();
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
			C->SetMaterial(0, Mat);
		}
		return C;
	};

	FLinearColor HullColor(0.06f, 0.07f, 0.09f);    // Dark gunmetal
	FLinearColor AccentColor(0.1f, 0.3f, 0.8f);       // Blue accent
	FLinearColor ThrusterColor(0.15f, 0.15f, 0.2f);   // Dark thruster

	// Main hull body — elongated cylinder
	HullBody = MakeComp(CylinderMesh, FVector(0.f, 0.f, 0.f),
		FVector(1.2f, 1.2f, 2.f), HullColor);

	// Nose cone
	HullNose = MakeComp(ConeMesh ? ConeMesh : CylinderMesh, FVector(0.f, 0.f, 200.f),
		FVector(1.0f, 1.0f, 0.8f), HullColor);

	// Stabilizer fins (4 fins around pod)
	FinLeft = MakeComp(CubeMesh, FVector(0.f, -100.f, -80.f),
		FVector(0.05f, 0.6f, 0.3f), AccentColor, FRotator(0.f, 0.f, 15.f));

	FinRight = MakeComp(CubeMesh, FVector(0.f, 100.f, -80.f),
		FVector(0.05f, 0.6f, 0.3f), AccentColor, FRotator(0.f, 0.f, -15.f));

	// Front/back fins
	MakeComp(CubeMesh, FVector(100.f, 0.f, -80.f),
		FVector(0.6f, 0.05f, 0.3f), AccentColor, FRotator(0.f, 0.f, 0.f));
	MakeComp(CubeMesh, FVector(-100.f, 0.f, -80.f),
		FVector(0.6f, 0.05f, 0.3f), AccentColor, FRotator(0.f, 0.f, 0.f));

	// Thruster cone at bottom
	ThrusterCone = MakeComp(CylinderMesh, FVector(0.f, 0.f, -160.f),
		FVector(0.6f, 0.6f, 0.4f), ThrusterColor);

	// Blue accent stripes on hull
	MakeComp(CubeMesh, FVector(0.f, 60.f, 40.f),
		FVector(0.02f, 0.02f, 1.2f), AccentColor);
	MakeComp(CubeMesh, FVector(0.f, -60.f, 40.f),
		FVector(0.02f, 0.02f, 1.2f), AccentColor);

	// --- VFX: Thruster flame (elongated bright cylinder below thruster) ---
	if (CylinderMesh && BaseMaterial)
	{
		ThrusterFlame = NewObject<UStaticMeshComponent>(this);
		ThrusterFlame->SetupAttachment(RootComponent);
		ThrusterFlame->SetStaticMesh(CylinderMesh);
		ThrusterFlame->SetRelativeLocation(FVector(0.f, 0.f, -250.f));
		ThrusterFlame->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.01f));
		ThrusterFlame->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ThrusterFlame->CastShadow = false;
		ThrusterFlame->RegisterComponent();
		FlameMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		FlameMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(2.f, 4.f, 10.f));
		FlameMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(2.f, 4.f, 10.f));
		ThrusterFlame->SetMaterial(0, FlameMat);
	}

	// --- VFX: Heat shield glow (sphere around pod during freefall) ---
	if (SphereMesh && BaseMaterial)
	{
		HeatShield = NewObject<UStaticMeshComponent>(this);
		HeatShield->SetupAttachment(RootComponent);
		HeatShield->SetStaticMesh(SphereMesh);
		HeatShield->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
		HeatShield->SetRelativeScale3D(FVector(2.5f, 2.5f, 3.5f));
		HeatShield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HeatShield->CastShadow = false;
		HeatShield->RegisterComponent();
		HeatMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		HeatMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(1.f, 0.3f, 0.05f));
		HeatMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(3.f, 0.8f, 0.15f));
		HeatShield->SetMaterial(0, HeatMat);

		// Heat glow light
		HeatLight = NewObject<UPointLightComponent>(this);
		HeatLight->SetupAttachment(RootComponent);
		HeatLight->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
		HeatLight->SetIntensity(0.f);
		HeatLight->SetAttenuationRadius(2000.f);
		HeatLight->SetLightColor(FLinearColor(1.f, 0.4f, 0.1f));
		HeatLight->CastShadows = false;
		HeatLight->RegisterComponent();
	}
}

void AExoDropPod::InitPod(AController* InPassenger, AExoDropPodManager* InManager)
{
	Passenger = InPassenger;
	Manager = InManager;

	BuildPodMesh();

	// Possess the pod so the player sees through PodCamera
	if (Passenger)
	{
		if (APawn* OldPawn = Passenger->GetPawn())
		{
			OldPawn->SetActorHiddenInGame(true);
			OldPawn->SetActorEnableCollision(false);
		}
	}

	Phase = EDropPodPhase::FreeFall;
}

void AExoDropPod::ApplySteerInput(FVector2D Input)
{
	SteerInput = Input;
}

void AExoDropPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Phase == EDropPodPhase::Landed)
	{
		LandedTimer += DeltaTime;
		if (LandedTimer > 1.5f)
		{
			OnLanded();
		}
		return;
	}

	// Detect ground
	if (!bGroundDetected)
	{
		FHitResult Hit;
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0.f, 0.f, 200000.f);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			GroundZ = Hit.ImpactPoint.Z + 200.f;
			bGroundDetected = true;
		}
	}

	float CurrentZ = GetActorLocation().Z;
	float AltAboveGround = bGroundDetected ? (CurrentZ - GroundZ) : 50000.f;

	// Phase transitions
	if (Phase == EDropPodPhase::FreeFall && AltAboveGround < BrakeAltitude)
	{
		Phase = EDropPodPhase::Braking;
	}
	if (Phase == EDropPodPhase::Braking && AltAboveGround < 500.f)
	{
		Phase = EDropPodPhase::Landing;
	}

	// Calculate descent speed based on phase
	float DescentSpeed;
	float BrakeAlpha = 0.f;
	switch (Phase)
	{
	case EDropPodPhase::FreeFall:
		DescentSpeed = MaxDescentSpeed;
		break;
	case EDropPodPhase::Braking:
		BrakeAlpha = 1.f - (AltAboveGround / BrakeAltitude);
		DescentSpeed = FMath::Lerp(MaxDescentSpeed, LandingSpeed, BrakeAlpha);
		break;
	case EDropPodPhase::Landing:
	default:
		BrakeAlpha = 1.f;
		DescentSpeed = LandingSpeed;
		break;
	}

	// Horizontal steering during freefall/braking
	FVector HorizontalMove = FVector::ZeroVector;
	if (Phase != EDropPodPhase::Landing)
	{
		float SteerMult = (Phase == EDropPodPhase::FreeFall) ? SteerSpeed : SteerSpeed * 0.3f;
		HorizontalMove = FVector(SteerInput.Y, SteerInput.X, 0.f) * SteerMult * DeltaTime;

		// Tilt pod based on steering
		float TargetTilt = SteerInput.X * 15.f;
		PodTilt = FMath::FInterpTo(PodTilt, TargetTilt, DeltaTime, 5.f);
		SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, PodTilt));
	}
	else
	{
		// Level out on landing
		PodTilt = FMath::FInterpTo(PodTilt, 0.f, DeltaTime, 8.f);
		SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, PodTilt));
	}

	// Move
	FVector NewLoc = GetActorLocation() + HorizontalMove - FVector(0.f, 0.f, DescentSpeed * DeltaTime);

	if (bGroundDetected && NewLoc.Z <= GroundZ)
	{
		NewLoc.Z = GroundZ;
		SetActorLocation(NewLoc);

		// Spawn landing impact
		FExoTracerManager::SpawnExplosionEffect(GetWorld(), GetActorLocation(), 800.f);

		Phase = EDropPodPhase::Landed;
		LandedTimer = 0.f;

		UE_LOG(LogExoRift, Log, TEXT("Drop pod touched down at %s"), *GetActorLocation().ToString());
	}
	else
	{
		SetActorLocation(NewLoc);
	}

	// Thruster VFX
	UpdateThrusterVFX(DeltaTime, BrakeAlpha);
}

void AExoDropPod::UpdateThrusterVFX(float DeltaTime, float BrakeAlpha)
{
	if (!ThrusterLight) return;

	float Time = GetWorld()->GetTimeSeconds();

	// Thruster light intensity based on braking
	float TargetIntensity = 0.f;
	if (Phase == EDropPodPhase::FreeFall)
		TargetIntensity = 5000.f;
	else if (Phase == EDropPodPhase::Braking)
		TargetIntensity = FMath::Lerp(5000.f, 80000.f, BrakeAlpha);
	else if (Phase == EDropPodPhase::Landing)
		TargetIntensity = 80000.f;

	float CurrentIntensity = ThrusterLight->Intensity;
	ThrusterLight->SetIntensity(FMath::FInterpTo(CurrentIntensity, TargetIntensity, DeltaTime, 4.f));

	FLinearColor ThrusterColor = FMath::Lerp(
		FLinearColor(0.3f, 0.5f, 1.f),
		FLinearColor(0.8f, 0.9f, 1.f),
		BrakeAlpha);
	ThrusterLight->SetLightColor(ThrusterColor);

	// --- Thruster flame: scales up with braking, flickers ---
	if (ThrusterFlame)
	{
		float FlameLen = 0.f;
		float FlameWidth = 0.3f;
		if (Phase == EDropPodPhase::FreeFall)
		{
			FlameLen = 0.5f;
			FlameWidth = 0.15f;
		}
		else if (Phase == EDropPodPhase::Braking)
		{
			FlameLen = FMath::Lerp(0.5f, 4.f, BrakeAlpha);
			FlameWidth = FMath::Lerp(0.15f, 0.5f, BrakeAlpha);
		}
		else if (Phase == EDropPodPhase::Landing)
		{
			FlameLen = 4.f;
			FlameWidth = 0.5f;
		}

		// Flicker
		float Flicker = 1.f + 0.15f * FMath::Sin(Time * 25.f) + 0.1f * FMath::Sin(Time * 37.f);
		ThrusterFlame->SetRelativeScale3D(FVector(FlameWidth * Flicker, FlameWidth * Flicker, FlameLen));
		ThrusterFlame->SetRelativeLocation(FVector(0.f, 0.f, -200.f - FlameLen * 50.f));

		// Color: blue → white-hot
		if (FlameMat)
		{
			FLinearColor FlameCol = FMath::Lerp(
				FLinearColor(1.f, 3.f, 8.f),
				FLinearColor(8.f, 8.f, 10.f),
				BrakeAlpha);
			FlameMat->SetVectorParameterValue(TEXT("EmissiveColor"), FlameCol);
		}
	}

	// --- Heat shield: visible during freefall, fades during braking ---
	if (HeatShield)
	{
		float HeatAlpha = 0.f;
		if (Phase == EDropPodPhase::FreeFall)
			HeatAlpha = 0.6f + 0.2f * FMath::Sin(Time * 3.f);
		else if (Phase == EDropPodPhase::Braking)
			HeatAlpha = FMath::Max(0.f, 0.6f * (1.f - BrakeAlpha));

		HeatShield->SetVisibility(HeatAlpha > 0.01f);
		if (HeatMat && HeatAlpha > 0.01f)
		{
			FLinearColor HeatCol(3.f * HeatAlpha, 0.8f * HeatAlpha, 0.15f * HeatAlpha);
			HeatMat->SetVectorParameterValue(TEXT("EmissiveColor"), HeatCol);
		}

		if (HeatLight)
		{
			HeatLight->SetIntensity(HeatAlpha * 30000.f);
		}
	}
}

void AExoDropPod::OnLanded()
{
	if (Phase != EDropPodPhase::Landed) return;

	// Spawn/reposition character
	if (Passenger)
	{
		FVector SpawnLoc = GetActorLocation() + FVector(200.f, 0.f, 100.f);
		FRotator SpawnRot = FRotator(0.f, GetActorRotation().Yaw, 0.f);

		if (APawn* ExistingPawn = Passenger->GetPawn())
		{
			ExistingPawn->SetActorLocation(SpawnLoc);
			ExistingPawn->SetActorRotation(SpawnRot);
			ExistingPawn->SetActorHiddenInGame(false);
			ExistingPawn->SetActorEnableCollision(true);
		}
		else
		{
			AExoCharacter* NewChar = GetWorld()->SpawnActor<AExoCharacter>(
				AExoCharacter::StaticClass(), SpawnLoc, SpawnRot);
			if (NewChar) Passenger->Possess(NewChar);
		}
	}

	if (Manager)
	{
		Manager->OnPodLanded(this);
	}

	SetLifeSpan(3.f);
}
