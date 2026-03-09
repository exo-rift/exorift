#pragma once

#include "CoreMinimal.h"

class UCanvas;
class UFont;

/**
 * Queued notification system for gameplay events.
 * Shows toast-style messages that slide in from the right and fade out.
 */
struct FExoNotification
{
	FString Message;
	FLinearColor Color;
	float Duration;
	float ElapsedTime = 0.f;
	float Alpha = 0.f;   // Animated opacity

	bool IsExpired() const { return ElapsedTime >= Duration; }
};

class FExoNotificationSystem
{
public:
	void AddNotification(const FString& Message, FLinearColor Color = FLinearColor::White, float Duration = 4.f);
	void Tick(float DeltaTime);
	void Draw(UCanvas* Canvas, UFont* Font);
	void Clear();

private:
	TArray<FExoNotification> Notifications;
	static constexpr int32 MaxVisible = 5;
	static constexpr float FadeInTime = 0.3f;
	static constexpr float FadeOutTime = 0.8f;
	static constexpr float NotificationHeight = 30.f;
	static constexpr float RightMargin = 20.f;
	static constexpr float TopMargin = 120.f;
};
