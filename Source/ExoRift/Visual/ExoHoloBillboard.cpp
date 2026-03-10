// ExoHoloBillboard.cpp — Animated holographic billboard with scrolling text bars
#include "Visual/ExoHoloBillboard.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AExoHoloBillboard::AExoHoloBillboard()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
		TEXT("/Engine/BasicShapes/Cube"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;

	auto MakePart = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		if (Cube) C->SetStaticMesh(Cube);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->CastShadow = false;
		return C;
	};

	ScreenMesh = MakePart(TEXT("Screen"));
	RootComponent = ScreenMesh;

	FrameTop = MakePart(TEXT("FrameTop"));
	FrameTop->SetupAttachment(ScreenMesh);

	FrameBottom = MakePart(TEXT("FrameBottom"));
	FrameBottom->SetupAttachment(ScreenMesh);

	for (int32 i = 0; i < NUM_BARS; i++)
	{
		FName BarName = *FString::Printf(TEXT("Bar_%d"), i);
		UStaticMeshComponent* Bar = MakePart(*BarName.ToString());
		Bar->SetupAttachment(ScreenMesh);
		TextBars.Add(Bar);
	}

	ScreenGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	ScreenGlow->SetupAttachment(ScreenMesh);
	ScreenGlow->SetIntensity(15000.f);
	ScreenGlow->SetAttenuationRadius(3000.f);
	ScreenGlow->CastShadows = false;

	BaseColor = FLinearColor(0.2f, 0.6f, 1.f);
}

void AExoHoloBillboard::InitBillboard(const FLinearColor& Color, float Width, float Height)
{
	BaseColor = Color;
	BoardWidth = Width;
	BoardHeight = Height;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (!MatFinder.Succeeded()) return;
	UMaterialInterface* BaseMat = MatFinder.Object;

	// Main screen — dark with subtle glow
	ScreenMesh->SetRelativeScale3D(FVector(Width / 100.f, 5.f, Height / 100.f));
	ScreenMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	ScreenMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.02f, 0.02f, 0.03f));
	ScreenMat->SetVectorParameterValue(TEXT("EmissiveColor"),
		FLinearColor(Color.R * 0.2f, Color.G * 0.2f, Color.B * 0.2f));
	ScreenMesh->SetMaterial(0, ScreenMat);

	// Frame accents
	float FrameW = Width / 100.f + 0.5f;
	auto SetupFrame = [&](UStaticMeshComponent* F, float ZOff)
	{
		F->SetRelativeLocation(FVector(0.f, 0.f, ZOff));
		F->SetRelativeScale3D(FVector(FrameW, 0.12f, 0.08f));
		UMaterialInstanceDynamic* FM = UMaterialInstanceDynamic::Create(BaseMat, this);
		FM->SetVectorParameterValue(TEXT("BaseColor"), Color * 0.3f);
		FM->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * 3.f, Color.G * 3.f, Color.B * 3.f));
		F->SetMaterial(0, FM);
	};
	SetupFrame(FrameTop, Height / 200.f + 2.f);
	SetupFrame(FrameBottom, -(Height / 200.f + 2.f));

	// Scrolling text bars — horizontal strips at different heights
	BarSpeeds.SetNum(NUM_BARS);
	BarPositions.SetNum(NUM_BARS);
	BarMats.SetNum(NUM_BARS);
	for (int32 i = 0; i < NUM_BARS; i++)
	{
		float BarH = FMath::RandRange(Height * 0.015f, Height * 0.04f);
		float ZPos = FMath::RandRange(-Height / 220.f, Height / 220.f);
		float BarWidth = FMath::RandRange(Width * 0.2f, Width * 0.6f);

		TextBars[i]->SetRelativeScale3D(
			FVector(BarWidth / 100.f, 0.06f, BarH / 100.f));
		TextBars[i]->SetRelativeLocation(FVector(0.f, -5.f, ZPos));

		BarSpeeds[i] = FMath::RandRange(300.f, 800.f) * (FMath::RandBool() ? 1.f : -1.f);
		BarPositions[i] = FMath::RandRange(-Width * 0.5f, Width * 0.5f);

		BarMats[i] = UMaterialInstanceDynamic::Create(BaseMat, this);
		float Em = FMath::RandRange(2.f, 6.f);
		BarMats[i]->SetVectorParameterValue(TEXT("BaseColor"), Color);
		BarMats[i]->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(Color.R * Em, Color.G * Em, Color.B * Em));
		TextBars[i]->SetMaterial(0, BarMats[i]);
	}

	ScreenGlow->SetRelativeLocation(FVector(0.f, -100.f, 0.f));
	ScreenGlow->SetLightColor(Color);
}

void AExoHoloBillboard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float HalfW = BoardWidth * 0.5f;

	// Scroll text bars
	for (int32 i = 0; i < NUM_BARS; i++)
	{
		BarPositions[i] += BarSpeeds[i] * DeltaTime;

		// Wrap around when off-screen
		if (BarPositions[i] > HalfW + 500.f)
			BarPositions[i] = -HalfW - 500.f;
		else if (BarPositions[i] < -HalfW - 500.f)
			BarPositions[i] = HalfW + 500.f;

		FVector Loc = TextBars[i]->GetRelativeLocation();
		Loc.X = BarPositions[i] / 100.f;
		TextBars[i]->SetRelativeLocation(Loc);
	}

	// Subtle screen flicker
	float Time = GetWorld()->GetTimeSeconds();
	float Flicker = 0.85f + 0.15f * FMath::Sin(Time * 40.f)
		* FMath::Sin(Time * 17.f);
	if (ScreenMat)
	{
		ScreenMat->SetVectorParameterValue(TEXT("EmissiveColor"),
			FLinearColor(BaseColor.R * 0.2f * Flicker,
				BaseColor.G * 0.2f * Flicker,
				BaseColor.B * 0.2f * Flicker));
	}
	ScreenGlow->SetIntensity(15000.f * Flicker);
}
