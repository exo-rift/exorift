#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoHazardZone.generated.h"

class AExoCharacter;
class USoundBase;
class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInstanceDynamic;

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

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void SetHazardEnabled(bool bEnable);
	bool IsHazardEnabled() const { return bEnabled; }

	EHazardType GetHazardType() const { return HazardType; }
	float GetHazardRadius() const { return HazardRadius; }

protected:
	UPROPERTY(EditAnywhere, Category = "Hazard")
	float HazardRadius = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Hazard")
	float DamagePerSecond = 10.f;

	UPROPERTY(EditAnywhere, Category = "Hazard")
	FString HazardName = TEXT("Radiation Zone");

	UPROPERTY(EditAnywhere, Category = "Hazard")
	EHazardType HazardType = EHazardType::Radiation;

	UPROPERTY(EditAnywhere, Category = "Hazard")
	bool bEnabled = true;

private:
	void ApplyHazardDamage(float DeltaTime);
	void UpdateVFX(float DeltaTime);
	void UpdateHazardAudio(float DeltaTime);
	FLinearColor GetHazardColor() const;

	// Visual components
	UPROPERTY()
	UStaticMeshComponent* GroundDisk = nullptr;

	UPROPERTY()
	UStaticMeshComponent* BoundaryRing = nullptr;

	UPROPERTY()
	UPointLightComponent* AmbientGlow = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* DiskMat = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* RingMat = nullptr;

	// Ambient hum audio
	UPROPERTY()
	USoundBase* HumSound = nullptr;

	float Age = 0.f;
	float HumTimer = 0.f;
	float BaseLightIntensity = 2500.f;
	bool bPlayerInsideZone = false;
};
