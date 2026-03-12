#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoZoneVisualizer.generated.h"

class AExoZoneSystem;
class UPointLightComponent;

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
	void UpdateTargetRing();
	void UpdateGroundGlow();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ZoneWallMesh;

	/** Thinner ring showing where the zone is shrinking TO. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* TargetRingMesh;

	/** Ground-level glow ring at the zone boundary. */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GroundGlowMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* ZoneMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* TargetMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* GlowMaterial;

	UPROPERTY()
	AExoZoneSystem* ZoneSystem = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float WallHeight = 50000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor ZoneColor = FLinearColor(0.1f, 0.3f, 1.f, 0.3f);

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor TargetColor = FLinearColor(1.f, 1.f, 1.f, 0.15f);

	/** Lightning arcs along zone edge — pulsing energy segments. */
	void UpdateEdgeLightning();

	static constexpr int32 NumLightningArcs = 12;

	UPROPERTY()
	TArray<UStaticMeshComponent*> LightningArcs;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> LightningMats;

	UPROPERTY()
	TArray<UPointLightComponent*> EdgeLights;

	float CachedRadius = -1.f;
	float CachedTargetRadius = -1.f;

	// Cached engine meshes
	UPROPERTY()
	UStaticMesh* CylinderMesh = nullptr;
	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
};
