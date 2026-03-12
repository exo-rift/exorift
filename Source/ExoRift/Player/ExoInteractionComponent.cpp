#include "Player/ExoInteractionComponent.h"
#include "Player/ExoCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
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
	UpdateHighlight(DeltaTime);
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

// ---------------------------------------------------------------------------
// Interactable highlight — custom depth + pulsing point light
// ---------------------------------------------------------------------------

void UExoInteractionComponent::UpdateHighlight(float DeltaTime)
{
	AActor* Current = CurrentInteractable.Get();
	AActor* Previous = PreviousInteractable.Get();

	// Focus changed — swap highlight
	if (Current != Previous)
	{
		ClearHighlight(Previous);
		ApplyHighlight(Current);
		PreviousInteractable = Current;
		HighlightPulsePhase = 0.f;
	}

	// Pulse the highlight light while focused
	if (Current && HighlightLight)
	{
		HighlightPulsePhase += DeltaTime * 4.f;
		float Pulse = 0.6f + 0.4f * FMath::Sin(HighlightPulsePhase);
		HighlightLight->SetIntensity(3000.f * Pulse);
		HighlightLight->SetWorldLocation(Current->GetActorLocation());
	}
}

void UExoInteractionComponent::ApplyHighlight(AActor* Target)
{
	if (!Target) return;

	// Enable custom depth on all primitive components for outline support
	TArray<UPrimitiveComponent*> Primitives;
	Target->GetComponents<UPrimitiveComponent>(Primitives);
	for (UPrimitiveComponent* Prim : Primitives)
	{
		Prim->SetRenderCustomDepth(true);
		Prim->SetCustomDepthStencilValue(1);
	}

	// Spawn a small pulsing light attached to the owner (interaction component)
	// positioned at the interactable for an immediate glow effect
	HighlightLight = NewObject<UPointLightComponent>(GetOwner());
	if (HighlightLight)
	{
		HighlightLight->RegisterComponent();
		HighlightLight->SetWorldLocation(Target->GetActorLocation());
		HighlightLight->SetIntensity(3000.f);
		HighlightLight->SetAttenuationRadius(200.f);
		HighlightLight->SetLightColor(FLinearColor(0.3f, 0.7f, 1.f)); // Sci-fi cyan-blue
		HighlightLight->SetCastShadows(false);
	}
}

void UExoInteractionComponent::ClearHighlight(AActor* Target)
{
	if (Target)
	{
		TArray<UPrimitiveComponent*> Primitives;
		Target->GetComponents<UPrimitiveComponent>(Primitives);
		for (UPrimitiveComponent* Prim : Primitives)
		{
			Prim->SetRenderCustomDepth(false);
		}
	}

	if (HighlightLight)
	{
		HighlightLight->DestroyComponent();
		HighlightLight = nullptr;
	}
}
