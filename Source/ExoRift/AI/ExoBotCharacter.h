#pragma once

#include "CoreMinimal.h"
#include "Player/ExoCharacter.h"
#include "Core/ExoTypes.h"
#include "ExoBotCharacter.generated.h"

UCLASS()
class EXORIFT_API AExoBotCharacter : public AExoCharacter
{
	GENERATED_BODY()

public:
	AExoBotCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	void SetAILODLevel(EAILODLevel NewLevel);
	EAILODLevel GetAILODLevel() const { return CurrentLODLevel; }

	// Audio callouts — spatial sounds that alert nearby players
	void PlaySpottedCallout();
	void PlayFireCallout();

protected:
	UPROPERTY()
	EAILODLevel CurrentLODLevel = EAILODLevel::Full;

	float FullTickInterval = 0.f;       // Every frame
	float SimplifiedTickInterval = 0.1f; // 10Hz
	float BasicTickInterval = 0.5f;      // 2Hz

	float TickAccumulator = 0.f;

	// Audio callout throttle — prevents sound spam
	float PainCalloutCooldown = 0.f;
	float SpottedCalloutCooldown = 0.f;
	float FireCalloutCooldown = 0.f;
};
