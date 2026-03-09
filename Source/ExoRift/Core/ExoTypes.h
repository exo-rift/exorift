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
	GrenadeLauncher,
	Sniper,
	Shotgun,
	SMG,
	Melee
};

UENUM(BlueprintType)
enum class EGrenadeType : uint8
{
	Frag,
	EMP,
	Smoke
};

UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
	Common,
	Rare,
	Epic,
	Legendary
};

UENUM(BlueprintType)
enum class EAILODLevel : uint8
{
	Full,       // <100m  — full BT, aiming, cover
	Simplified, // 100-500m — reduced tick, simplified pathing
	Basic       // >500m — minimal tick, wander only
};

UENUM(BlueprintType)
enum class EBotDifficulty : uint8
{
	Easy,       // Low accuracy, slow reaction, rarely strafes
	Medium,     // Moderate accuracy, strafes sometimes
	Hard,       // Good accuracy, fast reaction, strafes + seeks cover
	Elite       // Near-perfect aim, instant reaction, aggressive plays
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
