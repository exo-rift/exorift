#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoLaunchColumn.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

/**
 * Vertical energy column VFX spawned when a jump pad launches a player.
 * Rapidly expands upward then fades over ~0.6s.
 */
UCLASS()
class AExoLaunchColumn : public AActor
{
	GENERATED_BODY()

public:
	AExoLaunchColumn();
	virtual void Tick(float DeltaTime) override;

	void InitColumn(const FLinearColor& Color, float Height);

	static void SpawnColumn(UWorld* World, const FVector& Location,
		const FLinearColor& Color, float Height = 800.f);

private:
	UPROPERTY()
	UStaticMeshComponent* InnerBeam;

	UPROPERTY()
	UStaticMeshComponent* OuterGlow;

	UPROPERTY()
	UStaticMeshComponent* BaseRing;

	UPROPERTY()
	UPointLightComponent* ColumnLight;

	float Age = 0.f;
	float Lifetime = 0.6f;
	float ColumnHeight = 800.f;
};
