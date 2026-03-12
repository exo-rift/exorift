// ExoShowcaseRoom.h — Photo-realistic PBR showcase room
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShowcaseRoom.generated.h"

class UStaticMesh;
class UMaterialInstanceDynamic;

/**
 * A small enclosed sci-fi room built from primitive meshes with calibrated PBR materials
 * and multi-source Lumen lighting. Designed to showcase photo-realistic rendering quality.
 */
UCLASS()
class AExoShowcaseRoom : public AActor
{
	GENERATED_BODY()

public:
	AExoShowcaseRoom();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY() UStaticMesh* CubeMesh = nullptr;
	UPROPERTY() UStaticMesh* SphereMesh = nullptr;
	UPROPERTY() UStaticMesh* CylinderMesh = nullptr;
	UPROPERTY() UStaticMesh* ConeMesh = nullptr;
	bool bPlayerTeleported = false;
	float TeleportDelay = 0.f;

	// Room interior dimensions (cm) — 5m × 5m × 3.5m
	float RoomW = 500.f;
	float RoomD = 500.f;
	float RoomH = 350.f;
	float WallThick = 15.f;

	void BuildStructure();
	void BuildPanels();
	void BuildDetails();
	void BuildDisplayObjects();
	void BuildLighting();

	UStaticMeshComponent* Box(FVector Pos, FVector Size,
		UMaterialInstanceDynamic* Mat, bool bColl = true);
	UStaticMeshComponent* Sphere(FVector Pos, float Radius,
		UMaterialInstanceDynamic* Mat);
	UStaticMeshComponent* Cylinder(FVector Pos, float Radius, float Height,
		UMaterialInstanceDynamic* Mat, bool bColl = false);
	UStaticMeshComponent* Cone(FVector Pos, float Radius, float Height,
		UMaterialInstanceDynamic* Mat);
	UMaterialInstanceDynamic* PBR(FLinearColor Color, float Met, float Rough,
		float Spec = 0.5f, bool bMetal = false);
	UMaterialInstanceDynamic* Glow(FLinearColor Color);
};
