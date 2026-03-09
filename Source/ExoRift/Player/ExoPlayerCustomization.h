#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExoPlayerCustomization.generated.h"

/**
 * Persistent player customization data — saved to GameUserSettings.ini.
 * Lives on the GameInstance so it survives level transitions.
 */
UCLASS(Config = Game)
class EXORIFT_API UExoPlayerCustomization : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Customization Data ---

	UPROPERTY()
	FString PlayerName = TEXT("Pilot");

	UPROPERTY()
	int32 BannerIndex = 0;

	UPROPERTY()
	FLinearColor PrimaryColor = FLinearColor(0.f, 1.f, 1.f, 1.f); // Cyan

	/** 4 equipped emote slots — indices into global emote table. */
	UPROPERTY()
	int32 EmoteLoadout[4] = { 0, 1, 2, 3 };

	// --- Operations ---

	void SaveCustomization();
	void LoadCustomization();

	/** Convenience accessor from any world context. */
	static UExoPlayerCustomization* Get(UWorld* World);

private:
	static const FString IniSection;
};
