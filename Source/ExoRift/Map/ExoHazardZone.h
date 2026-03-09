#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoHazardZone.generated.h"

class AExoCharacter;

UENUM(BlueprintType)
enum class EHazardType : uint8
{
	Radiation,
	Fire,
	Electric,
	Toxic
};

UCLASS()
class EXORIFT_API AExoHazardZone : public AActor
{
	GENERATED_BODY()

public:
	AExoHazardZone();

	virtual void Tick(float DeltaTime) override;

	/** Enable or disable the hazard at runtime (for timed hazards). */
	void SetHazardEnabled(bool bEnable);
	bool IsHazardEnabled() const { return bEnabled; }

	UPROPERTY(EditAnywhere, Category = "Hazard")
	float HazardRadius = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Hazard")
	float DamagePerSecond = 10.f;

	UPROPERTY(EditAnywhere, Category = "Hazard")
	FString HazardName = TEXT("Radiation Zone");

	UPROPERTY(EditAnywhere, Category = "Hazard")
	EHazardType HazardType = EHazardType::Radiation;

protected:
	UPROPERTY(EditAnywhere, Category = "Hazard")
	bool bEnabled = true;

private:
	void ApplyHazardDamage(float DeltaTime);
};
