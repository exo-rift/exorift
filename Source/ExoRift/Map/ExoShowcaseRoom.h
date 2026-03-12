// ExoShowcaseRoom.h — Photo-realistic PBR showcase room with real imported assets
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoShowcaseRoom.generated.h"

class UStaticMesh;
class UMaterialInstanceDynamic;

/**
 * A small enclosed sci-fi room using imported Kenney/Quaternius meshes when available,
 * with calibrated PBR materials and multi-source Lumen lighting.
 * Falls back to BasicShapes primitives if assets are not found.
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
	// --- BasicShapes fallback meshes ---
	UPROPERTY() UStaticMesh* CubeMesh = nullptr;
	UPROPERTY() UStaticMesh* SphereMesh = nullptr;
	UPROPERTY() UStaticMesh* CylinderMesh = nullptr;
	UPROPERTY() UStaticMesh* ConeMesh = nullptr;

	// --- Imported asset meshes (loaded at runtime via LoadObject) ---
	UPROPERTY() UStaticMesh* KN_RoomLarge = nullptr;
	UPROPERTY() UStaticMesh* QT_FloorBasic = nullptr;
	UPROPERTY() UStaticMesh* QT_Wall1 = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsShelf = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsComputer = nullptr;
	UPROPERTY() UStaticMesh* QT_PropsCrate = nullptr;
	bool bHasRoomMesh = false;
	bool bHasQTProps = false;

	bool bPlayerTeleported = false;
	float TeleportDelay = 0.f;

	// Room interior dimensions (cm) — 5m × 5m × 3.5m
	float RoomW = 500.f;
	float RoomD = 500.f;
	float RoomH = 350.f;
	float WallThick = 15.f;

	void ScanAssets();
	void BuildStructure();
	void BuildPanels();
	void BuildDetails();
	void BuildDisplayObjects();
	void BuildLighting();

	/** Spawn an imported mesh with its own materials (no PBR override). */
	UStaticMeshComponent* SpawnMesh(UStaticMesh* Mesh, FVector Pos,
		FVector Scale = FVector(1.f), FRotator Rot = FRotator::ZeroRotator, bool bColl = true);

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
