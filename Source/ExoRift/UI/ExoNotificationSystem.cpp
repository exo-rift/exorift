#include "UI/ExoNotificationSystem.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

void FExoNotificationSystem::AddNotification(const FString& Message, FLinearColor Color, float Duration)
{
	FExoNotification Notif;
	Notif.Message = Message;
	Notif.Color = Color;
	Notif.Duration = Duration;
	Notifications.Add(Notif);

	// Trim old ones
	while (Notifications.Num() > MaxVisible * 2)
	{
		Notifications.RemoveAt(0);
	}
}

void FExoNotificationSystem::Tick(float DeltaTime)
{
	for (int32 i = Notifications.Num() - 1; i >= 0; i--)
	{
		FExoNotification& N = Notifications[i];
		N.ElapsedTime += DeltaTime;

		// Fade in
		if (N.ElapsedTime < FadeInTime)
			N.Alpha = N.ElapsedTime / FadeInTime;
		// Fade out
		else if (N.ElapsedTime > N.Duration - FadeOutTime)
			N.Alpha = FMath::Max((N.Duration - N.ElapsedTime) / FadeOutTime, 0.f);
		else
			N.Alpha = 1.f;

		if (N.IsExpired())
			Notifications.RemoveAt(i);
	}
}

void FExoNotificationSystem::Draw(UCanvas* Canvas, UFont* Font)
{
	if (!Canvas || !Font) return;

	const float ScreenW = Canvas->SizeX;
	float Y = TopMargin;
	int32 Drawn = 0;

	for (int32 i = FMath::Max(0, Notifications.Num() - MaxVisible); i < Notifications.Num(); i++)
	{
		const FExoNotification& N = Notifications[i];
		if (N.Alpha <= 0.01f) continue;

		float TextW, TextH;
		Canvas->TextSize(Font, N.Message, TextW, TextH);

		// Slide in from right
		float SlideOffset = (1.f - N.Alpha) * 80.f;
		float X = ScreenW - RightMargin - TextW - SlideOffset;

		// Background pill
		FLinearColor BgColor(0.02f, 0.02f, 0.05f, 0.7f * N.Alpha);
		Canvas->SetDrawColor(
			FMath::Clamp((int32)(BgColor.R * 255), 0, 255),
			FMath::Clamp((int32)(BgColor.G * 255), 0, 255),
			FMath::Clamp((int32)(BgColor.B * 255), 0, 255),
			FMath::Clamp((int32)(BgColor.A * 255), 0, 255));
		Canvas->DrawTile(Canvas->DefaultTexture, X - 8.f, Y - 2.f, TextW + 16.f,
			NotificationHeight, 0, 0, 1, 1);

		// Text
		FLinearColor DrawColor = N.Color;
		DrawColor.A = N.Alpha;
		Canvas->SetDrawColor(
			FMath::Clamp((int32)(DrawColor.R * 255), 0, 255),
			FMath::Clamp((int32)(DrawColor.G * 255), 0, 255),
			FMath::Clamp((int32)(DrawColor.B * 255), 0, 255),
			FMath::Clamp((int32)(DrawColor.A * 255), 0, 255));
		Canvas->DrawText(Font, N.Message, X, Y);

		Y += NotificationHeight + 4.f;
		Drawn++;
	}
}

void FExoNotificationSystem::Clear()
{
	Notifications.Empty();
}
