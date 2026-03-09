#include "Core/ExoGameSettings.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "ExoRift.h"

const FString UExoGameSettings::IniSection = TEXT("/Script/ExoRift.GameSettings");

void UExoGameSettings::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettings();
	UE_LOG(LogExoRift, Log, TEXT("GameSettings initialized (Sens=%.2f FOV=%.0f)"), MouseSensitivity, FOV);
}

void UExoGameSettings::SaveSettings()
{
	GConfig->SetFloat(*IniSection, TEXT("MouseSensitivity"), MouseSensitivity, GGameUserSettingsIni);
	GConfig->SetFloat(*IniSection, TEXT("FOV"), FOV, GGameUserSettingsIni);
	GConfig->SetFloat(*IniSection, TEXT("MasterVolume"), MasterVolume, GGameUserSettingsIni);
	GConfig->SetFloat(*IniSection, TEXT("SFXVolume"), SFXVolume, GGameUserSettingsIni);
	GConfig->SetFloat(*IniSection, TEXT("MusicVolume"), MusicVolume, GGameUserSettingsIni);
	GConfig->SetBool(*IniSection, TEXT("bShowFPS"), bShowFPS, GGameUserSettingsIni);
	GConfig->SetBool(*IniSection, TEXT("bShowPingLatency"), bShowPingLatency, GGameUserSettingsIni);
	GConfig->SetInt(*IniSection, TEXT("CrosshairStyle"), CrosshairStyle, GGameUserSettingsIni);
	GConfig->SetFloat(*IniSection, TEXT("AimAssistStrength"), AimAssistStrength, GGameUserSettingsIni);
	GConfig->SetBool(*IniSection, TEXT("bAutoSprint"), bAutoSprint, GGameUserSettingsIni);
	GConfig->SetInt(*IniSection, TEXT("ColorBlindMode"), ColorBlindMode, GGameUserSettingsIni);
	GConfig->SetBool(*IniSection, TEXT("bShowDamageNumbers"), bShowDamageNumbers, GGameUserSettingsIni);
	GConfig->SetBool(*IniSection, TEXT("bShowMinimap"), bShowMinimap, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void UExoGameSettings::LoadSettings()
{
	GConfig->GetFloat(*IniSection, TEXT("MouseSensitivity"), MouseSensitivity, GGameUserSettingsIni);
	GConfig->GetFloat(*IniSection, TEXT("FOV"), FOV, GGameUserSettingsIni);
	GConfig->GetFloat(*IniSection, TEXT("MasterVolume"), MasterVolume, GGameUserSettingsIni);
	GConfig->GetFloat(*IniSection, TEXT("SFXVolume"), SFXVolume, GGameUserSettingsIni);
	GConfig->GetFloat(*IniSection, TEXT("MusicVolume"), MusicVolume, GGameUserSettingsIni);
	GConfig->GetBool(*IniSection, TEXT("bShowFPS"), bShowFPS, GGameUserSettingsIni);
	GConfig->GetBool(*IniSection, TEXT("bShowPingLatency"), bShowPingLatency, GGameUserSettingsIni);
	GConfig->GetInt(*IniSection, TEXT("CrosshairStyle"), CrosshairStyle, GGameUserSettingsIni);
	GConfig->GetFloat(*IniSection, TEXT("AimAssistStrength"), AimAssistStrength, GGameUserSettingsIni);
	GConfig->GetBool(*IniSection, TEXT("bAutoSprint"), bAutoSprint, GGameUserSettingsIni);
	GConfig->GetInt(*IniSection, TEXT("ColorBlindMode"), ColorBlindMode, GGameUserSettingsIni);
	GConfig->GetBool(*IniSection, TEXT("bShowDamageNumbers"), bShowDamageNumbers, GGameUserSettingsIni);
	GConfig->GetBool(*IniSection, TEXT("bShowMinimap"), bShowMinimap, GGameUserSettingsIni);

	// Clamp
	MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 5.0f);
	FOV = FMath::Clamp(FOV, 60.f, 120.f);
	MasterVolume = FMath::Clamp(MasterVolume, 0.f, 1.f);
	SFXVolume = FMath::Clamp(SFXVolume, 0.f, 1.f);
	MusicVolume = FMath::Clamp(MusicVolume, 0.f, 1.f);
	CrosshairStyle = FMath::Clamp(CrosshairStyle, 0, 2);
	AimAssistStrength = FMath::Clamp(AimAssistStrength, 0.f, 1.f);
	ColorBlindMode = FMath::Clamp(ColorBlindMode, 0, 3);
}

void UExoGameSettings::ApplySettings(UWorld* World)
{
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	PC->InputYawScale_DEPRECATED = MouseSensitivity;
	PC->InputPitchScale_DEPRECATED = -MouseSensitivity;

	if (APawn* Pawn = PC->GetPawn())
	{
		if (UCameraComponent* Cam = Pawn->FindComponentByClass<UCameraComponent>())
			Cam->SetFieldOfView(FOV);
	}
}

UExoGameSettings* UExoGameSettings::Get(const UWorld* World)
{
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UExoGameSettings>() : nullptr;
}

FString UExoGameSettings::GetSettingName(int32 Index) const
{
	switch (Index)
	{
	case 0:  return TEXT("Mouse Sensitivity");
	case 1:  return TEXT("Field of View");
	case 2:  return TEXT("Master Volume");
	case 3:  return TEXT("SFX Volume");
	case 4:  return TEXT("Music Volume");
	case 5:  return TEXT("Show FPS");
	case 6:  return TEXT("Show Ping");
	case 7:  return TEXT("Crosshair Style");
	case 8:  return TEXT("Aim Assist");
	case 9:  return TEXT("Auto Sprint");
	case 10: return TEXT("Color Blind Mode");
	case 11: return TEXT("Damage Numbers");
	case 12: return TEXT("Show Minimap");
	default: return TEXT("");
	}
}

FString UExoGameSettings::GetSettingValue(int32 Index) const
{
	switch (Index)
	{
	case 0:  return FString::Printf(TEXT("%.1f"), MouseSensitivity);
	case 1:  return FString::Printf(TEXT("%.0f"), FOV);
	case 2:  return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MasterVolume * 100.f));
	case 3:  return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(SFXVolume * 100.f));
	case 4:  return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MusicVolume * 100.f));
	case 5:  return bShowFPS ? TEXT("ON") : TEXT("OFF");
	case 6:  return bShowPingLatency ? TEXT("ON") : TEXT("OFF");
	case 7:  { const TCHAR* N[] = { TEXT("Default"), TEXT("Dot"), TEXT("Circle") }; return N[FMath::Clamp(CrosshairStyle, 0, 2)]; }
	case 8:  return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(AimAssistStrength * 100.f));
	case 9:  return bAutoSprint ? TEXT("ON") : TEXT("OFF");
	case 10: { const TCHAR* N[] = { TEXT("Off"), TEXT("Protanopia"), TEXT("Deuteranopia"), TEXT("Tritanopia") }; return N[FMath::Clamp(ColorBlindMode, 0, 3)]; }
	case 11: return bShowDamageNumbers ? TEXT("ON") : TEXT("OFF");
	case 12: return bShowMinimap ? TEXT("ON") : TEXT("OFF");
	default: return TEXT("");
	}
}

void UExoGameSettings::AdjustSetting(int32 Index, float Direction)
{
	switch (Index)
	{
	case 0:  MouseSensitivity = FMath::Clamp(MouseSensitivity + Direction * 0.1f, 0.1f, 5.0f); break;
	case 1:  FOV = FMath::Clamp(FOV + Direction * 5.f, 60.f, 120.f); break;
	case 2:  MasterVolume = FMath::Clamp(MasterVolume + Direction * 0.05f, 0.f, 1.f); break;
	case 3:  SFXVolume = FMath::Clamp(SFXVolume + Direction * 0.05f, 0.f, 1.f); break;
	case 4:  MusicVolume = FMath::Clamp(MusicVolume + Direction * 0.05f, 0.f, 1.f); break;
	case 5:  bShowFPS = !bShowFPS; break;
	case 6:  bShowPingLatency = !bShowPingLatency; break;
	case 7:  CrosshairStyle = FMath::Clamp(CrosshairStyle + FMath::RoundToInt(Direction), 0, 2); break;
	case 8:  AimAssistStrength = FMath::Clamp(AimAssistStrength + Direction * 0.1f, 0.f, 1.f); break;
	case 9:  bAutoSprint = !bAutoSprint; break;
	case 10: ColorBlindMode = FMath::Clamp(ColorBlindMode + FMath::RoundToInt(Direction), 0, 3); break;
	case 11: bShowDamageNumbers = !bShowDamageNumbers; break;
	case 12: bShowMinimap = !bShowMinimap; break;
	}
}
