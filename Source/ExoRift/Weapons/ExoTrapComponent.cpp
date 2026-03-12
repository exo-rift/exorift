#include "Weapons/ExoTrapComponent.h"
#include "Weapons/ExoProximityMine.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoAudioManager.h"
#include "UI/ExoHUD.h"
#include "ExoRift.h"

UExoTrapComponent::UExoTrapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UExoTrapComponent::PlaceTrap(FVector Location, FRotator Rotation)
{
	if (TrapCount <= 0) return false;

	AExoCharacter* OwnerChar = Cast<AExoCharacter>(GetOwner());
	if (!OwnerChar) return false;

	FActorSpawnParameters Params;
	Params.Owner = OwnerChar;
	Params.Instigator = OwnerChar;

	AExoProximityMine* Mine = GetWorld()->SpawnActor<AExoProximityMine>(
		AExoProximityMine::StaticClass(), Location, Rotation, Params);

	if (Mine)
	{
		Mine->OwnerCharacter = OwnerChar;
		--TrapCount;

		// Audio feedback — placement confirmation chirp
		if (UExoAudioManager* Audio = UExoAudioManager::Get(GetWorld()))
			Audio->PlayTrapPlacedSound();

		// HUD notification — "Trap Placed"
		APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
		if (PC)
		{
			if (AExoHUD* HUD = Cast<AExoHUD>(PC->GetHUD()))
			{
				FString Msg = FString::Printf(TEXT("Trap Placed (%d remaining)"), TrapCount);
				HUD->GetNotifications().AddNotification(Msg,
					FLinearColor(1.f, 0.8f, 0.2f), 3.f);
			}
		}

		UE_LOG(LogExoRift, Log, TEXT("%s placed mine (%d remaining)"),
			*OwnerChar->GetName(), TrapCount);
		return true;
	}

	return false;
}

void UExoTrapComponent::AddTraps(int32 Count)
{
	TrapCount = FMath::Clamp(TrapCount + Count, 0, MaxTraps);
}
