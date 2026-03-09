#include "Player/ExoPlayerCustomization.h"
#include "ExoRift.h"

const FString UExoPlayerCustomization::IniSection = TEXT("/Script/ExoRift.PlayerCustomization");

void UExoPlayerCustomization::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadCustomization();
	UE_LOG(LogExoRift, Log, TEXT("PlayerCustomization loaded (Name=%s Banner=%d)"), *PlayerName, BannerIndex);
}

void UExoPlayerCustomization::SaveCustomization()
{
	GConfig->SetString(*IniSection, TEXT("PlayerName"), *PlayerName, GGameUserSettingsIni);
	GConfig->SetInt(*IniSection, TEXT("BannerIndex"), BannerIndex, GGameUserSettingsIni);
	GConfig->SetString(*IniSection, TEXT("PrimaryColor"), *PrimaryColor.ToString(), GGameUserSettingsIni);

	for (int32 i = 0; i < 4; ++i)
	{
		FString Key = FString::Printf(TEXT("EmoteSlot%d"), i);
		GConfig->SetInt(*IniSection, *Key, EmoteLoadout[i], GGameUserSettingsIni);
	}

	GConfig->Flush(false, GGameUserSettingsIni);
}

void UExoPlayerCustomization::LoadCustomization()
{
	GConfig->GetString(*IniSection, TEXT("PlayerName"), PlayerName, GGameUserSettingsIni);
	GConfig->GetInt(*IniSection, TEXT("BannerIndex"), BannerIndex, GGameUserSettingsIni);

	FString ColorStr;
	if (GConfig->GetString(*IniSection, TEXT("PrimaryColor"), ColorStr, GGameUserSettingsIni))
	{
		PrimaryColor.InitFromString(ColorStr);
	}

	for (int32 i = 0; i < 4; ++i)
	{
		FString Key = FString::Printf(TEXT("EmoteSlot%d"), i);
		GConfig->GetInt(*IniSection, *Key, EmoteLoadout[i], GGameUserSettingsIni);
	}

	BannerIndex = FMath::Clamp(BannerIndex, 0, 9);
}

UExoPlayerCustomization* UExoPlayerCustomization::Get(UWorld* World)
{
	if (!World) return nullptr;
	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;
	return GI->GetSubsystem<UExoPlayerCustomization>();
}
