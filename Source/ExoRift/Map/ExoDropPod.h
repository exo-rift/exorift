#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoDropPod.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class AExoDropPodManager;

UCLASS()
class EXORIFT_API AExoDropPod : public AActor
{
	GENERATED_BODY()

public:
	AExoDropPod();

	virtual void Tick(float DeltaTime) override;

	void InitPod(AController* InPassenger, AExoDropPodManager* InManager);

protected:
	void OnLanded();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PodMesh;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PodCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DescentSpeed = 8000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DecelerationAltitude = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float LandingSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float GroundTraceLength = 100000.f;

	UPROPERTY()
	AController* Passenger = nullptr;

	UPROPERTY()
	AExoDropPodManager* Manager = nullptr;

	bool bHasLanded = false;
	float GroundZ = 0.f;
	bool bGroundDetected = false;
};
