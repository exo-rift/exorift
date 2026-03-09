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

static void NotifDrawRect(UCanvas* C, float X, float Y, float W, float H, const FLinearColor& Col)
{
	C->SetDrawColor(
		FMath::Clamp((int32)(Col.R * 255), 0, 255),
		FMath::Clamp((int32)(Col.G * 255), 0, 255),
		FMath::Clamp((int32)(Col.B * 255), 0, 255),
		FMath::Clamp((int32)(Col.A * 255), 0, 255));
	C->DrawTile(C->DefaultTexture, X, Y, W, H, 0, 0, 1, 1);
}

void FExoNotificationSystem::Draw(UCanvas* Canvas, UFont* Font)
{
	if (!Canvas || !Font) return;

	const float ScreenW = Canvas->SizeX;
	float Y = TopMargin;

	for (int32 i = FMath::Max(0, Notifications.Num() - MaxVisible); i < Notifications.Num(); i++)
	{
		const FExoNotification& N = Notifications[i];
		if (N.Alpha <= 0.01f) continue;

		float TextW, TextH;
		Canvas->TextSize(Font, N.Message, TextW, TextH);

		// Slide in from right
		float SlideOffset = (1.f - FMath::Clamp(N.Alpha * 2.f, 0.f, 1.f)) * 60.f;
		float PanelW = TextW + 24.f;
		float X = ScreenW - RightMargin - PanelW - SlideOffset;

		// Background panel
		NotifDrawRect(Canvas, X, Y - 2.f, PanelW, NotificationHeight,
			FLinearColor(0.02f, 0.02f, 0.05f, 0.75f * N.Alpha));

		// Left color accent stripe
		NotifDrawRect(Canvas, X, Y - 2.f, 3.f, NotificationHeight,
			FLinearColor(N.Color.R, N.Color.G, N.Color.B, 0.8f * N.Alpha));

		// Top border line (subtle)
		NotifDrawRect(Canvas, X, Y - 2.f, PanelW, 1.f,
			FLinearColor(N.Color.R * 0.5f, N.Color.G * 0.5f, N.Color.B * 0.5f, 0.3f * N.Alpha));

		// Text
		FLinearColor DrawColor = N.Color;
		DrawColor.A = N.Alpha;
		Canvas->SetDrawColor(
			FMath::Clamp((int32)(DrawColor.R * 255), 0, 255),
			FMath::Clamp((int32)(DrawColor.G * 255), 0, 255),
			FMath::Clamp((int32)(DrawColor.B * 255), 0, 255),
			FMath::Clamp((int32)(DrawColor.A * 255), 0, 255));
		Canvas->DrawText(Font, N.Message, X + 12.f, Y);

		Y += NotificationHeight + 5.f;
	}
}

void FExoNotificationSystem::Clear()
{
	Notifications.Empty();
}
