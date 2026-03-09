#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ExoBTTask_EngageTarget.generated.h"

UCLASS()
class EXORIFT_API UExoBTTask_EngageTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UExoBTTask_EngageTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	float HeatThresholdToStopFiring = 0.85f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float HeatThresholdToResumeFiring = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FBlackboardKeySelector TargetActorKey;

	bool bIsFiring = false;
};
