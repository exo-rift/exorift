#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExoGameSettings.generated.h"

/**
 * Persistent game settings — saved to GameUserSettings.ini.
 * Lives on the GameInstance so settings survive level transitions.
 */
UCLASS()
class EXORIFT_API UExoGameSettings : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Settings ---

	UPROPERTY()
	float MouseSensitivity = 1.0f;

	UPROPERTY()
	float FOV = 90.f;

	UPROPERTY()
	float MasterVolume = 1.0f;

	UPROPERTY()
	float SFXVolume = 1.0f;

	UPROPERTY()
	float MusicVolume = 0.5f;

	UPROPERTY()
	bool bShowFPS = false;

	UPROPERTY()
	bool bShowPingLatency = false;

	UPROPERTY()
	int32 CrosshairStyle = 0; // 0=default, 1=dot only, 2=circle

	UPROPERTY()
	float AimAssistStrength = 0.5f; // 0-1

	UPROPERTY()
	bool bAutoSprint = false;

	UPROPERTY()
	int32 ColorBlindMode = 0; // 0=Off, 1=Protanopia, 2=Deuteranopia, 3=Tritanopia

	UPROPERTY()
	bool bShowDamageNumbers = true;

	UPROPERTY()
	bool bShowMinimap = true;

	// --- Operations ---

	void SaveSettings();
	void LoadSettings();
	void ApplySettings(UWorld* World);

	/** Convenience accessor from any world context. */
	static UExoGameSettings* Get(const UWorld* World);

	// Setting names for the menu display
	static constexpr int32 SettingsCount = 13;
	FString GetSettingName(int32 Index) const;
	FString GetSettingValue(int32 Index) const;
	void AdjustSetting(int32 Index, float Direction);

private:
	static const FString IniSection;
};
