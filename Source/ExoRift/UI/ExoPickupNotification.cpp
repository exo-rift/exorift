#include "UI/ExoPickupNotification.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

TArray<FExoPickupNotification::FNotifEntry> FExoPickupNotification::Entries;

void FExoPickupNotification::ShowWeaponPickup(const FString& WeaponName,
	const FLinearColor& RarityColor)
{
	FNotifEntry Entry;
	Entry.Text = WeaponName;
	Entry.SubText = TEXT("PICKED UP");
	Entry.Color = RarityColor;
	Entry.Lifetime = 2.f;

	if (Entries.Num() >= MaxEntries)
		Entries.RemoveAt(0);
	Entries.Add(Entry);
}

void FExoPickupNotification::ShowElimination(const FString& VictimName, bool bHeadshot)
{
	FNotifEntry Entry;
	Entry.Text = FString::Printf(TEXT("ELIMINATED %s"), *VictimName);
	Entry.SubText = bHeadshot ? TEXT("HEADSHOT") : TEXT("");
	Entry.Color = bHeadshot
		? FLinearColor(1.f, 0.85f, 0.2f, 1.f)
		: FLinearColor(1.f, 0.3f, 0.15f, 1.f);
	Entry.Lifetime = 3.f;
	Entry.bIsElimination = true;

	if (Entries.Num() >= MaxEntries)
		Entries.RemoveAt(0);
	Entries.Add(Entry);
}

void FExoPickupNotification::Tick(float DeltaTime)
{
	for (int32 i = Entries.Num() - 1; i >= 0; i--)
	{
		Entries[i].Age += DeltaTime;
		if (Entries[i].Age >= Entries[i].Lifetime)
			Entries.RemoveAt(i);
	}
}

void FExoPickupNotification::Draw(AHUD* HUD, UCanvas* Canvas, UFont* Font)
{
	if (!HUD || !Canvas || !Font || Entries.Num() == 0) return;

	float CX = Canvas->SizeX * 0.5f;
	float BaseY = Canvas->SizeY * 0.25f;

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const FNotifEntry& E = Entries[i];

		// Fade in first 0.2s, fade out last 0.5s
		float Alpha;
		if (E.Age < 0.2f)
			Alpha = E.Age / 0.2f;
		else if (E.Age > E.Lifetime - 0.5f)
			Alpha = (E.Lifetime - E.Age) / 0.5f;
		else
			Alpha = 1.f;
		Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

		// Slide up as they age
		float SlideY = BaseY + i * 45.f - E.Age * 8.f;

		float Scale = E.bIsElimination ? 1.2f : 1.f;
		FLinearColor TextColor = E.Color;
		TextColor.A = Alpha;

		// Main text
		float TW, TH;
		HUD->GetTextSize(E.Text, TW, TH, Font, Scale);
		float TX = CX - TW * 0.5f;

		// Background bar
		HUD->DrawRect(FLinearColor(0.02f, 0.02f, 0.04f, Alpha * 0.6f),
			TX - 15.f, SlideY - 5.f, TW + 30.f, TH + 25.f);

		// Color accent line on left
		HUD->DrawRect(FLinearColor(E.Color.R, E.Color.G, E.Color.B, Alpha * 0.9f),
			TX - 15.f, SlideY - 5.f, 3.f, TH + 25.f);

		HUD->DrawText(E.Text, TextColor, TX, SlideY, Font, Scale);

		// Sub text
		if (!E.SubText.IsEmpty())
		{
			FLinearColor SubColor(0.5f, 0.55f, 0.6f, Alpha * 0.7f);
			float SW, SH;
			HUD->GetTextSize(E.SubText, SW, SH, Font, 0.7f);
			HUD->DrawText(E.SubText, SubColor, CX - SW * 0.5f, SlideY + TH + 2.f, Font, 0.7f);
		}
	}
}
