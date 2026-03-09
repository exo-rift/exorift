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

	void SetAILODLevel(EAILODLevel NewLevel);
	EAILODLevel GetAILODLevel() const { return CurrentLODLevel; }

protected:
	UPROPERTY()
	EAILODLevel CurrentLODLevel = EAILODLevel::Full;

	float FullTickInterval = 0.f;       // Every frame
	float SimplifiedTickInterval = 0.1f; // 10Hz
	float BasicTickInterval = 0.5f;      // 2Hz

	float TickAccumulator = 0.f;
};
