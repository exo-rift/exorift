#include "UI/ExoDamageNumbers.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AExoDamageNumbers::AExoDamageNumbers()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AExoDamageNumbers::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = ActiveNumbers.Num() - 1; i >= 0; i--)
	{
		FFloatingDamageNumber& Num = ActiveNumbers[i];
		Num.WorldLocation += Num.Velocity * DeltaTime;

		if (CurrentTime - Num.SpawnTime >= Num.Lifetime)
		{
			ActiveNumbers.RemoveAt(i);
		}
	}
}

void AExoDamageNumbers::SpawnDamageNumber(const FVector& Location, float Damage, bool bCritical)
{
	FFloatingDamageNumber Num;
	Num.WorldLocation = Location + FVector(
		FMath::RandRange(-30.f, 30.f),
		FMath::RandRange(-30.f, 30.f),
		FMath::RandRange(50.f, 100.f));
	Num.Damage = Damage;
	Num.SpawnTime = GetWorld()->GetTimeSeconds();
	Num.Lifetime = DefaultLifetime;
	Num.bIsCritical = bCritical;
	Num.Velocity = FVector(FMath::RandRange(-20.f, 20.f),
		FMath::RandRange(-20.f, 20.f), FloatSpeed);

	ActiveNumbers.Add(Num);
}

void AExoDamageNumbers::DrawNumbers(AHUD* HUD, UCanvas* Canvas, UFont* Font) const
{
	if (!HUD || !Canvas || !Font) return;

	APlayerController* PC = HUD->GetOwningPlayerController();
	if (!PC) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (const FFloatingDamageNumber& Num : ActiveNumbers)
	{
		float Age = CurrentTime - Num.SpawnTime;
		float LifeFrac = FMath::Clamp(Age / Num.Lifetime, 0.f, 1.f);
		float Alpha = 1.f - LifeFrac;

		// Scale-in animation (pop at spawn, settle to normal)
		float ScaleAnim = 1.f;
		if (Age < 0.12f)
		{
			ScaleAnim = 1.f + (1.f - Age / 0.12f) * 0.4f;
		}

		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(Num.WorldLocation, ScreenPos)) continue;

		FString DmgText = FString::Printf(TEXT("%.0f"), Num.Damage);

		// Scale by damage amount and crit
		float BaseScale = 1.0f;
		if (Num.Damage >= 50.f) BaseScale = 1.2f;
		if (Num.Damage >= 80.f) BaseScale = 1.4f;
		if (Num.bIsCritical) BaseScale = 1.6f;
		float Scale = BaseScale * ScaleAnim;

		// Color by damage type and amount
		FLinearColor Color;
		if (Num.bIsCritical)
		{
			// Gold crit with pulsing brightness
			float CritPulse = 0.85f + 0.15f * FMath::Sin(Age * 12.f);
			Color = FLinearColor(1.f * CritPulse, 0.8f * CritPulse, 0.1f, Alpha);
		}
		else if (Num.Damage >= 50.f)
		{
			// Orange for high damage
			Color = FLinearColor(1.f, 0.55f, 0.15f, Alpha);
		}
		else
		{
			// White for normal
			Color = FLinearColor(0.95f, 0.95f, 1.f, Alpha);
		}

		float TextW, TextH;
		HUD->GetTextSize(DmgText, TextW, TextH, Font, Scale);
		float DrawX = ScreenPos.X - TextW * 0.5f;
		float DrawY = ScreenPos.Y - TextH * 0.5f;

		// Shadow
		HUD->DrawText(DmgText, FLinearColor(0.f, 0.f, 0.f, Alpha * 0.6f),
			DrawX + 1.5f, DrawY + 1.5f, Font, Scale);
		HUD->DrawText(DmgText, Color, DrawX, DrawY, Font, Scale);

		// Crit exclamation
		if (Num.bIsCritical && Age < 0.5f)
		{
			float ExclAlpha = Alpha * FMath::Clamp(1.f - Age / 0.5f, 0.f, 1.f);
			HUD->DrawText(TEXT("!"), FLinearColor(1.f, 0.8f, 0.1f, ExclAlpha),
				DrawX + TextW + 2.f, DrawY, Font, Scale * 0.8f);
		}
	}
}

AExoDamageNumbers* AExoDamageNumbers::Get(UWorld* World)
{
	if (!World) return nullptr;

	for (TActorIterator<AExoDamageNumbers> It(World); It; ++It)
	{
		return *It;
	}

	return World->SpawnActor<AExoDamageNumbers>();
}
