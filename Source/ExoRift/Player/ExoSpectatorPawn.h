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
	float DeathCamDuration = 3.f;

	// Free spectate
	UPROPERTY()
	TArray<TWeakObjectPtr<APawn>> SpectatableTargets;

	int32 CurrentSpectateIndex = -1;
	float TargetRefreshTimer = 0.f;

	void RefreshSpectatableTargets();
};
