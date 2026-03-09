#include "AI/Tasks/ExoBTTask_EngageTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Player/ExoCharacter.h"
#include "Weapons/ExoWeaponBase.h"

UExoBTTask_EngageTarget::UExoBTTask_EngageTarget()
{
	NodeName = TEXT("Engage Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UExoBTTask_EngageTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->GetPawn()) return EBTNodeResult::Failed;

	AExoCharacter* Bot = Cast<AExoCharacter>(AIC->GetPawn());
	if (!Bot || !Bot->IsAlive()) return EBTNodeResult::Failed;

	return EBTNodeResult::InProgress;
}

void UExoBTTask_EngageTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AExoCharacter* Bot = Cast<AExoCharacter>(AIC->GetPawn());
	if (!Bot || !Bot->IsAlive()) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!Target)
	{
		Bot->StopFire();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check target still alive
	AExoCharacter* TargetChar = Cast<AExoCharacter>(Target);
	if (TargetChar && !TargetChar->IsAlive())
	{
		Bot->StopFire();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Heat management — smart bots manage their heat
	AExoWeaponBase* Weapon = Bot->GetCurrentWeapon();
	if (Weapon)
	{
		float Heat = Weapon->GetCurrentHeat();
		if (bIsFiring && Heat >= HeatThresholdToStopFiring)
		{
			Bot->StopFire();
			bIsFiring = false;
		}
		else if (!bIsFiring && Heat <= HeatThresholdToResumeFiring && !Weapon->IsOverheated())
		{
			Bot->StartFire();
			bIsFiring = true;
		}
	}

	// Face target
	FVector Dir = (Target->GetActorLocation() - Bot->GetActorLocation()).GetSafeNormal();
	FRotator DesiredRot = Dir.Rotation();
	FRotator CurrentRot = AIC->GetControlRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaSeconds, 8.f);
	AIC->SetControlRotation(NewRot);
}
