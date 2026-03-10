#include "Visual/ExoFlickerLight.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoFlickerLight::AExoFlickerLight()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylFinder(
		TEXT("/Engine/BasicShapes/Cylinder"));

	FixtureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FixtureMesh"));
	RootComponent = FixtureMesh;
	FixtureMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FixtureMesh->CastShadow = false;
	FixtureMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.05f));
	if (CylFinder.Succeeded()) FixtureMesh->SetStaticMesh(CylFinder.Object);

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(FixtureMesh);
	Light->SetRelativeLocation(FVector(0.f, 0.f, -30.f));
	Light->SetIntensity(8000.f);
	Light->SetAttenuationRadius(2000.f);
	Light->CastShadows = false;

	NextFlickerTime = FMath::RandRange(0.5f, 3.f);
}

void AExoFlickerLight::InitLight(const FLinearColor& Color, float InBaseIntensity)
{
	BaseIntensity = InBaseIntensity;
	Light->SetLightColor(Color);
	Light->SetIntensity(BaseIntensity);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (MatFinder.Succeeded())
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
			MatFinder.Object, this);
		Mat->SetVectorParameterValue(TEXT("BaseColor"),
			FLinearColor(0.15f, 0.15f, 0.18f, 1.f));
		Mat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
		FixtureMesh->SetMaterial(0, Mat);
	}
}

void AExoFlickerLight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsOff)
	{
		OffTimer += DeltaTime;
		if (OffTimer >= OffDuration)
		{
			bIsOff = false;
			Light->SetIntensity(BaseIntensity);
			NextFlickerTime = FMath::RandRange(1.f, 5.f);
			FlickerTimer = 0.f;
		}
		else
		{
			// Brief flashes during off period
			float Flash = FMath::RandRange(0.f, 1.f) > 0.85f
				? BaseIntensity * 0.3f : 0.f;
			Light->SetIntensity(Flash);
		}
		return;
	}

	FlickerTimer += DeltaTime;

	// Normal operation: slight intensity variation
	float Time = GetWorld()->GetTimeSeconds();
	float Wobble = 1.f + 0.05f * FMath::Sin(Time * 8.f + GetActorLocation().X);
	Light->SetIntensity(BaseIntensity * Wobble);

	// Random flicker event
	if (FlickerTimer >= NextFlickerTime)
	{
		bIsOff = true;
		OffTimer = 0.f;
		OffDuration = FMath::RandRange(0.05f, 0.4f);
		Light->SetIntensity(0.f);
	}
}
