#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "ExoSpectatorPawn.generated.h"

UCLASS()
class EXORIFT_API AExoSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AExoSpectatorPawn();

	virtual void Tick(float DeltaTime) override;

	void StartDeathCam(AActor* Killer, const FVector& DeathLocation);
	void StartFreeCam();
	void CycleSpectateTarget(int32 Direction);
	bool IsInDeathCam() const { return bInDeathCam; }

protected:
	UPROPERTY()
	AActor* DeathCamTarget = nullptr;

	FVector DeathLocation = FVector::ZeroVector;
	bool bInDeathCam = false;
	float DeathCamTimer = 0.f;
	float DeathCamDuration = 4.f;

	// Pullback phase: camera rises and pulls back before orbiting
	float PullbackDuration = 1.2f;
	FVector PullbackStartLoc = FVector::ZeroVector;
	FRotator PullbackStartRot = FRotator::ZeroRotator;

	// Free spectate
	UPROPERTY()
	TArray<TWeakObjectPtr<APawn>> SpectatableTargets;

	int32 CurrentSpectateIndex = -1;
	float TargetRefreshTimer = 0.f;

	// Smooth follow state
	bool bInFreeSpectate = false;
	float SpectateFollowDistance = 350.f;
	float SpectateFollowHeight = 180.f;
	float SpectateInterpSpeed = 5.f;
	float SpectateCamOrbitAngle = 0.f;
	float SpectateIdleTimer = 0.f;

	void RefreshSpectatableTargets();

public:
	FString GetSpectateTargetName() const;
	bool IsInFreeSpectate() const { return bInFreeSpectate; }
	APawn* GetCurrentSpectateTarget() const;
};
