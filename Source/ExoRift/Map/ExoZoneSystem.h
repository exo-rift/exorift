#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ExoTypes.h"
#include "ExoZoneSystem.generated.h"

UCLASS()
class EXORIFT_API AExoZoneSystem : public AActor
{
	GENERATED_BODY()

public:
	AExoZoneSystem();

	virtual void Tick(float DeltaTime) override;

	void StartZoneSequence();
	bool IsInsideZone(const FVector& Location) const;
	float GetDamagePerSecond() const;
	bool IsShrinking() const { return bIsShrinking; }

	float GetCurrentRadius() const { return CurrentRadius; }
	FVector2D GetCurrentCenter() const { return CurrentCenter; }
	float GetTargetRadius() const { return TargetRadius; }
	FVector2D GetTargetCenter() const { return TargetCenter; }
	int32 GetCurrentStage() const { return CurrentStage; }
	float GetStageTimer() const { return StageTimer; }
	int32 GetNumStages() const { return Stages.Num(); }
	const FZoneStage* GetStage(int32 Index) const { return Stages.IsValidIndex(Index) ? &Stages[Index] : nullptr; }

protected:
	void AdvanceStage();
	void TickShrink(float DeltaTime);
	void TickHold(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Zone")
	TArray<FZoneStage> Stages;

	UPROPERTY(Replicated)
	int32 CurrentStage = -1;

	UPROPERTY(Replicated)
	float CurrentRadius = 200000.f;

	UPROPERTY(Replicated)
	FVector2D CurrentCenter = FVector2D::ZeroVector;

	UPROPERTY(Replicated)
	float TargetRadius = 200000.f;

	UPROPERTY(Replicated)
	FVector2D TargetCenter = FVector2D::ZeroVector;

	bool bIsActive = false;
	bool bIsShrinking = false;
	float StageTimer = 0.f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
