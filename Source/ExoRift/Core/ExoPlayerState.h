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
	int32 Deaths = 0;

	UPROPERTY(Replicated)
	int32 Assists = 0;

	UPROPERTY(Replicated)
	int32 DamageDealt = 0;

	UPROPERTY(Replicated)
	int32 Placement = 0;

	UPROPERTY(Replicated)
	bool bIsAlive = true;

	UPROPERTY(Replicated)
	FString DisplayName;

	UPROPERTY(Replicated)
	int32 Revives = 0;

	UPROPERTY(Replicated)
	int32 HeadshotKills = 0;

	// Not replicated — local tracking only
	float DamageTaken = 0.f;
	float LongestKillDistance = 0.f;
	int32 ShotsFired = 0;
	int32 ShotsHit = 0;
	float GetAccuracy() const { return ShotsFired > 0 ? (float)ShotsHit / ShotsFired * 100.f : 0.f; }

	void ResetStats();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
