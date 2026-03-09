#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDamageNumbers.generated.h"

struct FFloatingDamageNumber
{
	FVector WorldLocation;
	float Damage;
	float SpawnTime;
	float Lifetime;
	bool bIsCritical;
	FVector Velocity; // Float upward
};

UCLASS()
class EXORIFT_API AExoDamageNumbers : public AActor
{
	GENERATED_BODY()

public:
	AExoDamageNumbers();

	virtual void Tick(float DeltaTime) override;

	void SpawnDamageNumber(const FVector& Location, float Damage, bool bCritical = false);

	// Called from HUD to draw all active numbers
	void DrawNumbers(AHUD* HUD, UCanvas* Canvas, UFont* Font) const;

	static AExoDamageNumbers* Get(UWorld* World);

protected:
	TArray<FFloatingDamageNumber> ActiveNumbers;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers")
	float DefaultLifetime = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Numbers")
	float FloatSpeed = 100.f;
};
