#include "AI/Tasks/ExoBTTask_FindCover.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UExoBTTask_FindCover::UExoBTTask_FindCover()
{
	NodeName = TEXT("Find Cover");
}

EBTNodeResult::Type UExoBTTask_FindCover::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn()) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	FVector Origin = AIC->GetPawn()->GetActorLocation();

	// Get threat from blackboard (target enemy position)
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	FVector ThreatLoc = Target ? Target->GetActorLocation() : Origin + FVector(1000.f, 0.f, 0.f);

	FVector CoverPos;
	if (FindCoverPosition(Origin, ThreatLoc, CoverPos))
	{
		BB->SetValueAsVector(CoverLocationKey.SelectedKeyName, CoverPos);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

bool UExoBTTask_FindCover::FindCoverPosition(const FVector& Origin, const FVector& ThreatLoc, FVector& OutCoverPos) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	FVector ThreatDir = (ThreatLoc - Origin).GetSafeNormal();
	float BestScore = -1.f;
	FVector BestPos = Origin;
	bool bFound = false;

	float AngleStep = 2.f * PI / NumTraces;

	for (int32 i = 0; i < NumTraces; i++)
	{
		float Angle = i * AngleStep;
		FVector Dir(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		FVector TestPos = Origin + Dir * SearchRadius;

		// Trace from test position toward threat — if blocked, it's cover
		FHitResult Hit;
		FCollisionQueryParams Params;
		if (World->LineTraceSingleByChannel(Hit, TestPos, ThreatLoc, ECC_Visibility, Params))
		{
			// Position is behind cover relative to threat
			// Score: prefer positions farther from threat direction (flanking/behind cover)
			float DotWithThreat = FVector::DotProduct(Dir, -ThreatDir);
			float Score = DotWithThreat + 0.5f; // Prefer positions opposite to threat

			if (Score > BestScore)
			{
				BestScore = Score;
				BestPos = TestPos;
				bFound = true;
			}
		}
	}

	OutCoverPos = BestPos;
	return bFound;
}
