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

	// Update and remove expired numbers
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
	Num.Velocity = FVector(FMath::RandRange(-20.f, 20.f), FMath::RandRange(-20.f, 20.f), FloatSpeed);

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
		float Alpha = FMath::Clamp(1.f - (Age / Num.Lifetime), 0.f, 1.f);

		// Project to screen
		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(Num.WorldLocation, ScreenPos)) continue;

		FString DmgText = FString::Printf(TEXT("%.0f"), Num.Damage);
		float Scale = Num.bIsCritical ? 1.5f : 1.0f;

		FLinearColor Color = Num.bIsCritical ?
			FLinearColor(1.f, 0.8f, 0.1f, Alpha) :  // Gold for crits
			FLinearColor(1.f, 1.f, 1.f, Alpha);       // White normal

		float TextW, TextH;
		HUD->GetTextSize(DmgText, TextW, TextH, Font, Scale);
		HUD->DrawText(DmgText, Color,
			ScreenPos.X - TextW * 0.5f,
			ScreenPos.Y - TextH * 0.5f,
			Font, Scale);
	}
}

AExoDamageNumbers* AExoDamageNumbers::Get(UWorld* World)
{
	if (!World) return nullptr;

	for (TActorIterator<AExoDamageNumbers> It(World); It; ++It)
	{
		return *It;
	}

	// Auto-spawn if not found
	return World->SpawnActor<AExoDamageNumbers>();
}
