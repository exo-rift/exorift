#include "Player/ExoInteractionComponent.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "ExoRift.h"

UExoInteractionComponent::UExoInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Tick every frame to keep the prompt responsive
}

void UExoInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateTrace();
}

void UExoInteractionComponent::TryInteract()
{
	AActor* Target = CurrentInteractable.Get();
	if (!Target) return;

	IExoInteractable* Interactable = Cast<IExoInteractable>(Target);
	if (!Interactable) return;

	AExoCharacter* OwnerChar = Cast<AExoCharacter>(GetOwner());
	if (OwnerChar && OwnerChar->IsAlive())
	{
		Interactable->Interact(OwnerChar);
	}
}

FString UExoInteractionComponent::GetCurrentPrompt() const
{
	AActor* Target = CurrentInteractable.Get();
	if (!Target) return FString();

	IExoInteractable* Interactable = Cast<IExoInteractable>(Target);
	if (!Interactable) return FString();

	return Interactable->GetInteractionPrompt();
}

void UExoInteractionComponent::UpdateTrace()
{
	CurrentInteractable = nullptr;

	AExoCharacter* OwnerChar = Cast<AExoCharacter>(GetOwner());
	if (!OwnerChar || !OwnerChar->IsAlive()) return;

	UCameraComponent* Camera = OwnerChar->GetFirstPersonCamera();
	if (!Camera) return;

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * TraceRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

	FHitResult Hit;
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit, Start, End, FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	if (bHit && Hit.GetActor())
	{
		IExoInteractable* Interactable = Cast<IExoInteractable>(Hit.GetActor());
		if (Interactable)
		{
			CurrentInteractable = Hit.GetActor();
		}
	}
}
