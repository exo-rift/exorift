#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoZoneVisualizer.generated.h"

class AExoZoneSystem;
class UProceduralMeshComponent;

UCLASS()
class EXORIFT_API AExoZoneVisualizer : public AActor
{
	GENERATED_BODY()

public:
	AExoZoneVisualizer();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	void UpdateZoneWall();
	void GenerateCircleMesh(float Radius, FVector2D Center, float WallHeight,
		TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UVs);

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ZoneWallMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* ZoneMaterial;

	UPROPERTY()
	AExoZoneSystem* ZoneSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float WallHeight = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	int32 CircleSegments = 64;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor ZoneColor = FLinearColor(0.1f, 0.3f, 1.f, 0.3f);

	float CachedRadius = -1.f;
};
