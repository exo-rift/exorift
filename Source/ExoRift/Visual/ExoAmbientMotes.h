#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoAmbientMotes.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Floating energy motes that drift through the air, creating a sci-fi atmosphere.
 * Spawns a cluster of glowing particles that bob and drift organically.
 * Place one per area or let ExoWorldSetup scatter them.
 */
UCLASS()
class AExoAmbientMotes : public AActor
{
	GENERATED_BODY()

public:
	AExoAmbientMotes();
	virtual void Tick(float DeltaTime) override;

	void InitMotes(const FLinearColor& Color, float InRadius, int32 Count);

	/** Scatter mote clusters across the map. Called by ExoWorldSetup. */
	static void SpawnClusters(UWorld* World);

private:
	struct FMote
	{
		UStaticMeshComponent* Mesh = nullptr; // Kept alive by component attachment
		FVector BaseOffset;
		FVector DriftVelocity;
		float BobPhase = 0.f;
		float BobFreq = 0.5f;
		float BobAmp = 40.f;
		float PulsePhase = 0.f;
		float BaseSize = 0.05f;
	};

	TArray<FMote> Motes;

	UPROPERTY()
	UPointLightComponent* ClusterLight;

	float ClusterRadius = 2000.f;
	FLinearColor MoteColor = FLinearColor(0.2f, 0.6f, 1.f);
	UPROPERTY()
	UStaticMesh* SphereMesh = nullptr;
};
