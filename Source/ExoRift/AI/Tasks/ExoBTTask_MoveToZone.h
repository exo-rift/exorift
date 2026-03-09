#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ExoBTTask_MoveToZone.generated.h"

UCLASS()
class EXORIFT_API UExoBTTask_MoveToZone : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UExoBTTask_MoveToZone();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
