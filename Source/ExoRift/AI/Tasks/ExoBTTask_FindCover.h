#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ExoBTTask_FindCover.generated.h"

UCLASS()
class EXORIFT_API UExoBTTask_FindCover : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UExoBTTask_FindCover();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	bool FindCoverPosition(const FVector& Origin, const FVector& ThreatLoc, FVector& OutCoverPos) const;

	UPROPERTY(EditAnywhere, Category = "Cover")
	float SearchRadius = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Cover")
	int32 NumTraces = 12;

	UPROPERTY(EditAnywhere, Category = "Cover")
	FBlackboardKeySelector CoverLocationKey;
};
