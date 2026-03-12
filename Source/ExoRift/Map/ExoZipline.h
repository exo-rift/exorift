#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoZipline.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Traversal zipline connecting two anchor points.
 * Players interact at either end to ride the cable.
 */
UCLASS()
class EXORIFT_API AExoZipline : public AActor
{
	GENERATED_BODY()

public:
	AExoZipline();

	virtual void Tick(float DeltaTime) override;

	/** Build the cable visual between two world-space endpoints. */
	void InitZipline(const FVector& InStart, const FVector& InEnd);

	/** Get world position at parameter T (0=start, 1=end). */
	FVector GetPositionAtT(float T) const;

	/** Get normalized direction from start to end. */
	FVector GetDirection() const;

	float GetCableLength() const { return CableLength; }
	FVector GetStartPoint() const { return StartPoint; }
	FVector GetEndPoint() const { return EndPoint; }

	UPROPERTY(EditDefaultsOnly, Category = "Zipline")
	float RideSpeed = 2500.f;

	/** Spawn a zipline between two points. */
	static AExoZipline* SpawnZipline(UWorld* World, const FVector& Start, const FVector& End);

private:
	static constexpr int32 CABLE_SEGMENTS = 12;
	static constexpr int32 ENERGY_NODES = 4;

	FVector StartPoint = FVector::ZeroVector;
	FVector EndPoint = FVector::ZeroVector;
	float CableLength = 0.f;

	UPROPERTY()
	TArray<UStaticMeshComponent*> CableSegments;

	UPROPERTY()
	TArray<UStaticMeshComponent*> EnergyNodes;

	UPROPERTY()
	TArray<UPointLightComponent*> NodeLights;

	// Endpoint glowing spheres for interaction hint
	UPROPERTY()
	UStaticMeshComponent* StartSphere = nullptr;

	UPROPERTY()
	UStaticMeshComponent* EndSphere = nullptr;

	UPROPERTY()
	UPointLightComponent* StartLight = nullptr;

	UPROPERTY()
	UPointLightComponent* EndLight = nullptr;

	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
	UPROPERTY()
	UStaticMesh* SphereMesh = nullptr;
};
