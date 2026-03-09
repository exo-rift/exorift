#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExoFootstepComponent.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class EExoSurfaceType : uint8
{
	Default,
	Metal,
	Dirt,
	Water,
	Concrete
};

/**
 * Footstep audio component. Traces downward to detect surface type
 * via Physical Material, then plays a 3D-positioned footstep sound.
 * Attach to AExoCharacter and call PlayFootstep() at movement intervals.
 */
UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent))
class EXORIFT_API UExoFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExoFootstepComponent();

	/** Call this from the owning character at the right cadence. */
	void PlayFootstep(bool bIsSprinting, bool bIsSliding);

	// --- Sound assets per surface type (set in editor) ---

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TSoftObjectPtr<USoundBase> DefaultFootstep;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TSoftObjectPtr<USoundBase> MetalFootstep;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TSoftObjectPtr<USoundBase> DirtFootstep;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TSoftObjectPtr<USoundBase> WaterFootstep;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	TSoftObjectPtr<USoundBase> ConcreteFootstep;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	float TraceDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	float WalkVolume = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	float SprintVolume = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Footsteps")
	float SlideVolume = 0.5f;

protected:
	/** Traces down from the owner's location to determine surface type. */
	EExoSurfaceType DetectSurface() const;

	/** Resolves the sound asset for a given surface. */
	USoundBase* GetSoundForSurface(EExoSurfaceType Surface) const;
};
