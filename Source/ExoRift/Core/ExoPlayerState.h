#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ExoPlayerState.generated.h"

UCLASS()
class EXORIFT_API AExoPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AExoPlayerState();

	UPROPERTY(Replicated)
	int32 Kills = 0;

	UPROPERTY(Replicated)
	int32 Placement = 0;

	UPROPERTY(Replicated)
	bool bIsAlive = true;

	UPROPERTY(Replicated)
	FString DisplayName;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
