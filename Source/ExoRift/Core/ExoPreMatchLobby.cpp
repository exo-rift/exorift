// ExoPreMatchLobby.cpp -- Warmup lobby with pop-up targets and barriers
#include "Core/ExoPreMatchLobby.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

static const FLinearColor TgtColors[] = {
	{0.f, 0.8f, 1.f}, {1.f, 0.3f, 0.1f}, {0.2f, 1.f, 0.3f},
	{1.f, 0.1f, 0.5f}, {0.9f, 0.9f, 0.2f}, {0.5f, 0.2f, 1.f}
};

AExoPreMatchLobby::AExoPreMatchLobby()
{
	PrimaryActorTick.bCanEverTick = true;
	LobbyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LobbyRoot"));
	RootComponent = LobbyRoot;
}

void AExoPreMatchLobby::BeginPlay()
{
	Super::BeginPlay();
	BuildPlatform();
	BuildBarriers();
	SpawnTargets();
	UE_LOG(LogExoRift, Log, TEXT("PreMatchLobby active at %s"), *GetActorLocation().ToString());
}

AExoPreMatchLobby* AExoPreMatchLobby::SpawnLobby(UWorld* World, const FVector& Center)
{
	if (!World) return nullptr;
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return World->SpawnActor<AExoPreMatchLobby>(
		AExoPreMatchLobby::StaticClass(), Center, FRotator::ZeroRotator, P);
}

void AExoPreMatchLobby::Shutdown()
{
	bActive = false;
	for (auto* M : TargetMeshes) if (M) M->SetVisibility(false);
	for (auto* L : TargetLights) if (L) L->SetIntensity(0.f);
	for (auto* B : BarrierMeshes) if (B) B->SetVisibility(false);
	if (PlatformMesh) PlatformMesh->SetVisibility(false);
	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
}

void AExoPreMatchLobby::BuildPlatform()
{
	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
	if (!Cyl) return;

	PlatformMesh = NewObject<UStaticMeshComponent>(this, TEXT("Platform"));
	PlatformMesh->SetStaticMesh(Cyl);
	PlatformMesh->SetupAttachment(LobbyRoot);
	PlatformMesh->SetRelativeScale3D(FVector(PlatformR / 50.f, PlatformR / 50.f, 0.04f));
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlatformMesh->CastShadow = false;
	PlatformMesh->RegisterComponent();

	auto* Mat = UMaterialInstanceDynamic::Create(FExoMaterialFactory::GetLitEmissive(), this);
	if (!Mat) { return; }
	Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.03f, 0.04f, 0.05f));
	Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.f, 0.3f, 0.5f));
	Mat->SetScalarParameterValue(TEXT("Metallic"), 0.9f);
	Mat->SetScalarParameterValue(TEXT("Roughness"), 0.15f);
	PlatformMesh->SetMaterial(0, Mat);
}

void AExoPreMatchLobby::BuildBarriers()
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (!Cube) return;

	UMaterialInterface* AddMat = FExoMaterialFactory::GetEmissiveAdditive();
	const float D = PlatformR - 50.f;
	const float WL = PlatformR * 2.f / 100.f;
	const float WH = BarrierH / 100.f;

	struct WD { FVector Off; FVector Sc; };
	const WD Walls[] = {
		{{D, 0, BarrierH * .5f},  {0.02f, WL, WH}}, {{-D, 0, BarrierH * .5f}, {0.02f, WL, WH}},
		{{0, D, BarrierH * .5f},  {WL, 0.02f, WH}}, {{0, -D, BarrierH * .5f}, {WL, 0.02f, WH}},
	};

	for (int32 i = 0; i < 4; ++i)
	{
		auto* W = NewObject<UStaticMeshComponent>(this, *FString::Printf(TEXT("Barrier_%d"), i));
		W->SetStaticMesh(Cube);
		W->SetupAttachment(LobbyRoot);
		W->SetRelativeLocation(Walls[i].Off);
		W->SetRelativeScale3D(Walls[i].Sc);
		W->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		W->SetCollisionResponseToAllChannels(ECR_Block);
		W->CastShadow = false;
		W->RegisterComponent();

		auto* WMat = UMaterialInstanceDynamic::Create(AddMat, this);
		if (!WMat) { continue; }
		WMat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.f, 1.8f, 3.f));
		WMat->SetScalarParameterValue(TEXT("Opacity"), 0.25f);
		W->SetMaterial(0, WMat);
		BarrierMeshes.Add(W);
	}
}

void AExoPreMatchLobby::SpawnTargets()
{
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere"));
	if (!Sphere) return;

	UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();
	const float ArenaR = PlatformR * 0.7f;

	for (int32 i = 0; i < TargetCount; ++i)
	{
		float Ang = (2.f * PI * i) / TargetCount;
		FVector Pos(FMath::Cos(Ang) * ArenaR, FMath::Sin(Ang) * ArenaR, 80.f);

		auto* M = NewObject<UStaticMeshComponent>(this, *FString::Printf(TEXT("Tgt_%d"), i));
		M->SetStaticMesh(Sphere);
		M->SetupAttachment(LobbyRoot);
		M->SetRelativeLocation(Pos);
		M->SetRelativeScale3D(FVector(0.6f));
		M->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		M->SetCollisionResponseToAllChannels(ECR_Ignore);
		M->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		M->CastShadow = false;
		M->SetVisibility(false);
		M->RegisterComponent();

		const auto& C = TgtColors[i % UE_ARRAY_COUNT(TgtColors)];
		auto* Mat = UMaterialInstanceDynamic::Create(EmMat, this);
		if (!Mat) { continue; }
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(C.R*8, C.G*8, C.B*8));
		M->SetMaterial(0, Mat);

		auto* Lt = NewObject<UPointLightComponent>(this, *FString::Printf(TEXT("TgtL_%d"), i));
		Lt->SetupAttachment(M);
		Lt->SetIntensity(0.f);
		Lt->SetAttenuationRadius(400.f);
		Lt->SetLightColor(C);
		Lt->CastShadows = false;
		Lt->RegisterComponent();

		TargetMeshes.Add(M);
		TargetLights.Add(Lt);
		TgtVisible.Add(false);
		TgtTimers.Add(FMath::FRandRange(0.5f, 3.f)); // staggered start
	}
}

void AExoPreMatchLobby::ShowTarget(int32 I)
{
	TgtVisible[I] = true;
	TgtTimers[I] = FMath::FRandRange(2.f, 3.f);
	TargetMeshes[I]->SetVisibility(true);
	TargetLights[I]->SetIntensity(5000.f);
}

void AExoPreMatchLobby::HideTarget(int32 I)
{
	TgtVisible[I] = false;
	TgtTimers[I] = FMath::FRandRange(1.5f, 3.f);
	TargetMeshes[I]->SetVisibility(false);
	TargetLights[I]->SetIntensity(0.f);
}

void AExoPreMatchLobby::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bActive) return;

	// Pop-up target cycling
	for (int32 i = 0; i < TargetCount; ++i)
	{
		TgtTimers[i] -= DeltaTime;
		if (TgtTimers[i] <= 0.f)
			TgtVisible[i] ? HideTarget(i) : ShowTarget(i);

		if (TgtVisible[i])
		{
			float Pulse = 0.8f + 0.2f * FMath::Sin(GetWorld()->GetTimeSeconds() * 3.f + i);
			TargetLights[i]->SetIntensity(5000.f * Pulse);
		}
	}

	// Barrier shimmer
	float T = GetWorld()->GetTimeSeconds();
	for (int32 i = 0; i < BarrierMeshes.Num(); ++i)
	{
		float W = 0.7f + 0.3f * FMath::Sin(T * 1.5f + i * 1.57f);
		if (auto* M = Cast<UMaterialInstanceDynamic>(BarrierMeshes[i]->GetMaterial(0)))
			M->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0, 1.8f*W, 3.f*W));
	}
}
