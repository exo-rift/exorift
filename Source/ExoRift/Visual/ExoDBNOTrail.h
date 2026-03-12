#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDBNOTrail.generated.h"

class UStaticMeshComponent;

/** Small energy bleed mark left on the ground by DBNO players crawling. */
UCLASS()
class AExoDBNOTrail : public AActor
{
	GENERATED_BODY()

public:
	AExoDBNOTrail();
	virtual void Tick(float DeltaTime) override;

	static void SpawnMark(UWorld* World, const FVector& Location);

private:
	UPROPERTY()
	UStaticMeshComponent* Mark;

	float Age = 0.f;
	static constexpr float FadeStart = 3.f;
	static constexpr float Lifetime = 6.f;
};
