// ExoKillAnnouncer.cpp — Multi-kill and first blood announcements
#include "UI/ExoKillAnnouncer.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

int32 FExoKillAnnouncer::TotalMatchKills = 0;
int32 FExoKillAnnouncer::RapidKillCount = 0;
float FExoKillAnnouncer::RapidKillTimer = 0.f;
bool FExoKillAnnouncer::bFirstBloodClaimed = false;

void FExoKillAnnouncer::Reset()
{
	TotalMatchKills = 0;
	RapidKillCount = 0;
	RapidKillTimer = 0.f;
	bFirstBloodClaimed = false;
}

void FExoKillAnnouncer::RegisterKill()
{
	TotalMatchKills++;

	// First blood check (first kill of the entire match)
	if (!bFirstBloodClaimed)
	{
		bFirstBloodClaimed = true;
		// Note: shown via instance since static can't call ShowAnnouncement
		// The HUD will pick this up through a flag
	}

	// Rapid kill tracking
	if (RapidKillTimer > 0.f)
	{
		RapidKillCount++;
	}
	else
	{
		RapidKillCount = 1;
	}
	RapidKillTimer = RapidKillWindow;
}

void FExoKillAnnouncer::Tick(float DeltaTime)
{
	// Update rapid kill timer
	if (RapidKillTimer > 0.f)
	{
		RapidKillTimer -= DeltaTime;
		if (RapidKillTimer <= 0.f)
		{
			// Window closed — evaluate and announce
			if (RapidKillCount >= 2)
			{
				FString Text;
				FLinearColor Color;
				float Dur = 2.5f;

				if (RapidKillCount >= 5)
				{
					Text = TEXT("PENTAKILL");
					Color = FLinearColor(1.f, 0.1f, 0.1f);
					Dur = 3.5f;
				}
				else if (RapidKillCount >= 4)
				{
					Text = TEXT("QUAD KILL");
					Color = FLinearColor(1.f, 0.2f, 0.5f);
					Dur = 3.f;
				}
				else if (RapidKillCount >= 3)
				{
					Text = TEXT("TRIPLE KILL");
					Color = FLinearColor(1.f, 0.6f, 0.1f);
				}
				else
				{
					Text = TEXT("DOUBLE KILL");
					Color = FLinearColor(1.f, 0.85f, 0.2f);
				}
				ShowAnnouncement(Text, Color, Dur);
			}
			RapidKillCount = 0;
		}
	}

	// Check first blood (show once on first RegisterKill)
	static int32 LastAnnouncedKills = 0;
	if (TotalMatchKills == 1 && LastAnnouncedKills == 0)
	{
		ShowAnnouncement(TEXT("FIRST BLOOD"), FLinearColor(0.9f, 0.15f, 0.1f), 2.5f);
		LastAnnouncedKills = 1;
	}
	if (TotalMatchKills == 0) LastAnnouncedKills = 0;

	// Tick active announcements
	for (int32 i = ActiveAnnouncements.Num() - 1; i >= 0; --i)
	{
		ActiveAnnouncements[i].ElapsedTime += DeltaTime;
		if (ActiveAnnouncements[i].ElapsedTime >= ActiveAnnouncements[i].Duration)
		{
			ActiveAnnouncements.RemoveAt(i);
		}
	}
}

void FExoKillAnnouncer::Draw(UCanvas* Canvas, UFont* Font)
{
	if (!Canvas || !Font || ActiveAnnouncements.Num() == 0) return;

	float CX = Canvas->SizeX * 0.5f;
	float BaseY = Canvas->SizeY * 0.25f;

	for (int32 i = 0; i < ActiveAnnouncements.Num(); i++)
	{
		const FAnnouncement& Ann = ActiveAnnouncements[i];
		float T = Ann.ElapsedTime / Ann.Duration;

		// Scale animation: punch in, hold, shrink out
		float Scale = 1.f;
		if (T < 0.15f)
		{
			// Punch in: overshoot
			float In = T / 0.15f;
			Scale = FMath::Lerp(2.5f, 1.f, In * In);
		}
		else if (T > 0.8f)
		{
			float Out = (T - 0.8f) / 0.2f;
			Scale = FMath::Lerp(1.f, 0.5f, Out);
		}

		// Alpha
		float Alpha = 1.f;
		if (T < 0.1f) Alpha = T / 0.1f;
		else if (T > 0.75f) Alpha = 1.f - (T - 0.75f) / 0.25f;

		FLinearColor DrawColor = Ann.Color;
		DrawColor.A = FMath::Clamp(Alpha, 0.f, 1.f);

		// Measure text
		float TextW = 0.f, TextH = 0.f;
		Canvas->TextSize(Font, Ann.Text, TextW, TextH, Scale, Scale);

		float DrawX = CX - TextW * 0.5f;
		float DrawY = BaseY + i * 50.f - TextH * 0.5f;

		// Shadow
		Canvas->SetDrawColor(0, 0, 0, FMath::RoundToInt32(DrawColor.A * 180.f));
		Canvas->DrawText(Font, Ann.Text, DrawX + 2.f, DrawY + 2.f, Scale, Scale);

		// Main text
		Canvas->SetDrawColor(
			FMath::RoundToInt32(DrawColor.R * 255.f),
			FMath::RoundToInt32(DrawColor.G * 255.f),
			FMath::RoundToInt32(DrawColor.B * 255.f),
			FMath::RoundToInt32(DrawColor.A * 255.f));
		Canvas->DrawText(Font, Ann.Text, DrawX, DrawY, Scale, Scale);
	}
}

void FExoKillAnnouncer::ShowAnnouncement(const FString& Text, const FLinearColor& Color, float Duration)
{
	FAnnouncement Ann;
	Ann.Text = Text;
	Ann.Color = Color;
	Ann.Duration = Duration;
	ActiveAnnouncements.Add(Ann);

	// Cap max concurrent
	if (ActiveAnnouncements.Num() > 3)
	{
		ActiveAnnouncements.RemoveAt(0);
	}
}
