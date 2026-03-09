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

// ---------------------------------------------------------------------------
// Save / Load via GConfig
// ---------------------------------------------------------------------------

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

	// Clamp values to valid ranges
	MouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 5.0f);
	FOV = FMath::Clamp(FOV, 60.f, 120.f);
	MasterVolume = FMath::Clamp(MasterVolume, 0.f, 1.f);
	SFXVolume = FMath::Clamp(SFXVolume, 0.f, 1.f);
	MusicVolume = FMath::Clamp(MusicVolume, 0.f, 1.f);
	CrosshairStyle = FMath::Clamp(CrosshairStyle, 0, 2);
}

// ---------------------------------------------------------------------------
// Apply
// ---------------------------------------------------------------------------

void UExoGameSettings::ApplySettings(UWorld* World)
{
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	// Sensitivity — InputYawScale / InputPitchScale
	PC->InputYawScale_DEPRECATED = MouseSensitivity;
	PC->InputPitchScale_DEPRECATED = -MouseSensitivity; // inverted pitch

	// FOV — apply to possessed pawn's camera
	if (APawn* Pawn = PC->GetPawn())
	{
		if (UCameraComponent* Cam = Pawn->FindComponentByClass<UCameraComponent>())
		{
			Cam->SetFieldOfView(FOV);
		}
	}
}

// ---------------------------------------------------------------------------
// Static accessor
// ---------------------------------------------------------------------------

UExoGameSettings* UExoGameSettings::Get(const UWorld* World)
{
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	return GI->GetSubsystem<UExoGameSettings>();
}

// ---------------------------------------------------------------------------
// Menu helpers
// ---------------------------------------------------------------------------

FString UExoGameSettings::GetSettingName(int32 Index) const
{
	switch (Index)
	{
	case 0: return TEXT("Mouse Sensitivity");
	case 1: return TEXT("Field of View");
	case 2: return TEXT("Master Volume");
	case 3: return TEXT("SFX Volume");
	case 4: return TEXT("Music Volume");
	case 5: return TEXT("Show FPS");
	case 6: return TEXT("Show Ping");
	case 7: return TEXT("Crosshair Style");
	default: return TEXT("");
	}
}

FString UExoGameSettings::GetSettingValue(int32 Index) const
{
	switch (Index)
	{
	case 0: return FString::Printf(TEXT("%.1f"), MouseSensitivity);
	case 1: return FString::Printf(TEXT("%.0f"), FOV);
	case 2: return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MasterVolume * 100.f));
	case 3: return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(SFXVolume * 100.f));
	case 4: return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(MusicVolume * 100.f));
	case 5: return bShowFPS ? TEXT("ON") : TEXT("OFF");
	case 6: return bShowPingLatency ? TEXT("ON") : TEXT("OFF");
	case 7:
	{
		const TCHAR* Names[] = { TEXT("Default"), TEXT("Dot"), TEXT("Circle") };
		return Names[FMath::Clamp(CrosshairStyle, 0, 2)];
	}
	default: return TEXT("");
	}
}

void UExoGameSettings::AdjustSetting(int32 Index, float Direction)
{
	switch (Index)
	{
	case 0: MouseSensitivity = FMath::Clamp(MouseSensitivity + Direction * 0.1f, 0.1f, 5.0f); break;
	case 1: FOV = FMath::Clamp(FOV + Direction * 5.f, 60.f, 120.f); break;
	case 2: MasterVolume = FMath::Clamp(MasterVolume + Direction * 0.05f, 0.f, 1.f); break;
	case 3: SFXVolume = FMath::Clamp(SFXVolume + Direction * 0.05f, 0.f, 1.f); break;
	case 4: MusicVolume = FMath::Clamp(MusicVolume + Direction * 0.05f, 0.f, 1.f); break;
	case 5: bShowFPS = (Direction > 0.f) ? true : false; break;
	case 6: bShowPingLatency = (Direction > 0.f) ? true : false; break;
	case 7: CrosshairStyle = FMath::Clamp(CrosshairStyle + FMath::RoundToInt(Direction), 0, 2); break;
	}
}
