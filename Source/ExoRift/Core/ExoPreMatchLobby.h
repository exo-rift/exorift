#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoPreMatchLobby.generated.h"

/** Warmup lobby with pop-up targets and holographic barriers. */
UCLASS()
class EXORIFT_API AExoPreMatchLobby : public AActor
{
	GENERATED_BODY()

public:
	AExoPreMatchLobby();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	static AExoPreMatchLobby* SpawnLobby(UWorld* World, const FVector& Center);
	void Shutdown();

private:
	void BuildPlatform();
	void BuildBarriers();
	void SpawnTargets();
	void ShowTarget(int32 I);
	void HideTarget(int32 I);

	UPROPERTY() USceneComponent* LobbyRoot;
	UPROPERTY() UStaticMeshComponent* PlatformMesh;
	UPROPERTY() TArray<UStaticMeshComponent*> BarrierMeshes;
	UPROPERTY() TArray<UStaticMeshComponent*> TargetMeshes;
	UPROPERTY() TArray<class UPointLightComponent*> TargetLights;

	TArray<bool> TgtVisible;
	TArray<float> TgtTimers;

	static constexpr int32 TargetCount = 6;
	static constexpr float PlatformR = 1200.f;
	static constexpr float BarrierH = 500.f;
	bool bActive = true;
};
