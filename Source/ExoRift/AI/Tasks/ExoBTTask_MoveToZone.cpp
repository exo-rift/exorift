#include "AI/Tasks/ExoBTTask_MoveToZone.h"
#include "AIController.h"
#include "Map/ExoZoneSystem.h"
#include "EngineUtils.h"

UExoBTTask_MoveToZone::UExoBTTask_MoveToZone()
{
	NodeName = TEXT("Move To Zone");
}

EBTNodeResult::Type UExoBTTask_MoveToZone::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn()) return EBTNodeResult::Failed;

	// Find zone system
	AExoZoneSystem* Zone = nullptr;
	for (TActorIterator<AExoZoneSystem> It(AIC->GetWorld()); It; ++It)
	{
		Zone = *It;
		break;
	}

	if (!Zone) return EBTNodeResult::Failed;

	// Already inside?
	if (Zone->IsInsideZone(AIC->GetPawn()->GetActorLocation()))
	{
		return EBTNodeResult::Succeeded;
	}

	// Move toward zone center
	FVector2D Center = Zone->GetCurrentCenter();
	FVector Target(Center.X, Center.Y, AIC->GetPawn()->GetActorLocation().Z);
	AIC->MoveToLocation(Target, 500.f);

	return EBTNodeResult::InProgress;
}
