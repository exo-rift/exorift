// ExoMuzzleSparks.cpp — Energy discharge sparks from weapon muzzle
#include "Visual/ExoMuzzleSparks.h"
#include "Visual/ExoMaterialFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoMuzzleSparks::AExoMuzzleSparks()
{
	PrimaryActorTick.bCanEverTick = true;
	InitialLifeSpan = 0.4f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFind(
		TEXT("/Engine/BasicShapes/Cube"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	for (int32 i = 0; i < NUM_MUZZLE_SPARKS; i++)
	{
		FName Name = *FString::Printf(TEXT("MuzzleSpark_%d"), i);
		UStaticMeshComponent* S = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		S->SetupAttachment(Root);
		S->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		S->CastShadow = false;
		S->SetGenerateOverlapEvents(false);
		if (CubeFind.Succeeded()) S->SetStaticMesh(CubeFind.Object);
		SparkMeshes.Add(S);
	}

	SparkLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SparkLight"));
	SparkLight->SetupAttachment(Root);
	SparkLight->SetIntensity(40000.f);
	SparkLight->SetAttenuationRadius(800.f);
	SparkLight->CastShadows = false;
}

void AExoMuzzleSparks::InitSparks(const FRotator& FireDir, const FLinearColor& Color,
	EWeaponType WeaponType)
{
	FVector Forward = FireDir.Vector();
	FVector Right, Up;
	Forward.FindBestAxisVectors(Right, Up);

	// Weapon-specific spark behavior
	float SparkSpeed = 400.f;
	float SparkSpread = 200.f;
	float SparkSize = 0.05f;
	float LightPower = 60000.f;

	switch (WeaponType)
	{
	case EWeaponType::Shotgun:
		SparkSpeed = 600.f; SparkSpread = 350.f; SparkSize = 0.08f;
		LightPower = 100000.f; Lifetime = 0.18f;
		break;
	case EWeaponType::Sniper:
		SparkSpeed = 500.f; SparkSpread = 150.f; SparkSize = 0.06f;
		LightPower = 80000.f; Lifetime = 0.20f;
		break;
	case EWeaponType::SMG:
		SparkSpeed = 350.f; SparkSpread = 250.f; SparkSize = 0.04f;
		LightPower = 40000.f; Lifetime = 0.10f;
		break;
	default:
		break;
	}

	UMaterialInterface* BaseMat = FExoMaterialFactory::GetEmissiveAdditive();
	if (!BaseMat) return;

	FLinearColor SparkCol(
		Color.R * 60.f + 10.f,
		Color.G * 60.f + 10.f,
		Color.B * 60.f + 10.f);

	SparkVelocities.SetNum(NUM_MUZZLE_SPARKS);
	for (int32 i = 0; i < NUM_MUZZLE_SPARKS; i++)
	{
		// Sparks fly forward and to the sides
		FVector Vel = Forward * SparkSpeed * FMath::RandRange(0.3f, 1.f);
		Vel += Right * FMath::RandRange(-SparkSpread, SparkSpread);
		Vel += Up * FMath::RandRange(-SparkSpread, SparkSpread);
		SparkVelocities[i] = Vel;

		float S = SparkSize * FMath::RandRange(0.6f, 1.4f);
		SparkMeshes[i]->SetWorldScale3D(FVector(S * 3.f, S * 0.5f, S * 0.5f));

		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMat, this);
		FLinearColor SC = SparkCol * FMath::RandRange(0.6f, 1.2f);
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"), SC);
		SparkMeshes[i]->SetMaterial(0, Mat);
	}

	SparkLight->SetLightColor(Color);
	SparkLight->SetIntensity(LightPower);
	BaseLightIntensity = LightPower;
}

void AExoMuzzleSparks::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	float T = FMath::Clamp(Age / Lifetime, 0.f, 1.f);
	float Alpha = 1.f - T;

	SparkLight->SetIntensity(BaseLightIntensity * Alpha * Alpha);

	for (int32 i = 0; i < SparkMeshes.Num(); i++)
	{
		if (!SparkMeshes[i]) continue;
		FVector Pos = SparkVelocities[i] * Age;
		Pos.Z -= 200.f * Age * Age;
		SparkMeshes[i]->SetRelativeLocation(Pos);

		// Tumble and shrink
		float S = 0.05f * Alpha;
		SparkMeshes[i]->SetWorldScale3D(FVector(S * 3.f, S * 0.5f, S * 0.5f));
		float R = Age * (600.f + i * 200.f);
		SparkMeshes[i]->SetRelativeRotation(FRotator(R, R * 0.6f, R * 0.3f));
	}

	if (Age >= Lifetime) Destroy();
}

void AExoMuzzleSparks::SpawnSparks(UWorld* World, const FVector& Location,
	const FRotator& FireDir, const FLinearColor& Color, EWeaponType WeaponType)
{
	if (!World) return;
	// Skip for melee
	if (WeaponType == EWeaponType::Melee) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AExoMuzzleSparks* Sparks = World->SpawnActor<AExoMuzzleSparks>(
		AExoMuzzleSparks::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (Sparks)
	{
		Sparks->InitSparks(FireDir, Color, WeaponType);
	}
}
