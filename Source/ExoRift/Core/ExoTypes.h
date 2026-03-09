#pragma once

#include "CoreMinimal.h"
#include "ExoTypes.generated.h"

UENUM(BlueprintType)
enum class EBRMatchPhase : uint8
{
	WaitingForPlayers,
	DropPhase,
	Playing,
	ZoneShrinking,
	EndGame
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rifle,
	Pistol,
	GrenadeLauncher
};

UENUM(BlueprintType)
enum class EAILODLevel : uint8
{
	Full,       // <100m  — full BT, aiming, cover
	Simplified, // 100-500m — reduced tick, simplified pathing
	Basic       // >500m — minimal tick, wander only
};

USTRUCT()
struct FKillFeedEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString KillerName;

	UPROPERTY()
	FString VictimName;

	UPROPERTY()
	FString WeaponName;

	UPROPERTY()
	float Timestamp = 0.f;
};

USTRUCT()
struct FZoneStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector2D Center = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere)
	float Radius = 20000.f;

	UPROPERTY(EditAnywhere)
	float ShrinkDuration = 60.f;

	UPROPERTY(EditAnywhere)
	float HoldDuration = 90.f;

	UPROPERTY(EditAnywhere)
	float DamagePerSecond = 5.f;
};
