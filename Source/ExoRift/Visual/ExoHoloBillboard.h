#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoHoloBillboard.generated.h"

/**
 * Animated holographic billboard with scrolling text bars.
 * Placed at compounds for sci-fi atmosphere.
 */
UCLASS()
class AExoHoloBillboard : public AActor
{
	GENERATED_BODY()

public:
	AExoHoloBillboard();

	void InitBillboard(const FLinearColor& Color, float Width = 4000.f, float Height = 2000.f);
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	UStaticMeshComponent* ScreenMesh;

	UPROPERTY()
	UStaticMeshComponent* FrameTop;

	UPROPERTY()
	UStaticMeshComponent* FrameBottom;

	// Scrolling text bars — horizontal strips that slide across
	static constexpr int32 NUM_BARS = 5;
	UPROPERTY()
	TArray<UStaticMeshComponent*> TextBars;

	UPROPERTY()
	TArray<class UMaterialInstanceDynamic*> BarMats;

	TArray<float> BarSpeeds;
	TArray<float> BarPositions;

	UPROPERTY()
	class UPointLightComponent* ScreenGlow;

	UPROPERTY()
	class UMaterialInstanceDynamic* ScreenMat;

	FLinearColor BaseColor;
	float BoardWidth = 4000.f;
	float BoardHeight = 2000.f;
};
