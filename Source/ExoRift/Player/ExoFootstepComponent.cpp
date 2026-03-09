#include "Player/ExoFootstepComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ExoRift.h"

UExoFootstepComponent::UExoFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Driven externally
}

void UExoFootstepComponent::PlayFootstep(bool bIsSprinting, bool bIsSliding)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	EExoSurfaceType Surface = DetectSurface();
	USoundBase* Sound = GetSoundForSurface(Surface);
	if (!Sound) return;

	// Volume and pitch based on movement mode
	float Volume = WalkVolume;
	float Pitch = 1.f;

	if (bIsSliding)
	{
		Volume = SlideVolume;
		Pitch = 0.7f; // Scraping/slide sound pitched down
	}
	else if (bIsSprinting)
	{
		Volume = SprintVolume;
		Pitch = 0.9f;
	}

	// 3D positioned at the actor's feet
	FVector FootLocation = Owner->GetActorLocation();
	FootLocation.Z -= 90.f; // Approximate foot offset

	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(), Sound, FootLocation, Volume, Pitch,
		0.f, nullptr, nullptr);
}

EExoSurfaceType UExoFootstepComponent::DetectSurface() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return EExoSurfaceType::Default;

	FVector Start = Owner->GetActorLocation();
	FVector End = Start - FVector(0.f, 0.f, TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	Params.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);

	if (!bHit || !Hit.PhysMaterial.IsValid()) return EExoSurfaceType::Default;

	// Map physical material surface type to our enum.
	// EPhysicalSurface values are project-defined in DefaultEngine.ini.
	// We use a simple convention: SurfaceType1=Metal, 2=Dirt, 3=Water, 4=Concrete
	switch (Hit.PhysMaterial->SurfaceType)
	{
	case SurfaceType1:  return EExoSurfaceType::Metal;
	case SurfaceType2:  return EExoSurfaceType::Dirt;
	case SurfaceType3:  return EExoSurfaceType::Water;
	case SurfaceType4:  return EExoSurfaceType::Concrete;
	default:            return EExoSurfaceType::Default;
	}
}

USoundBase* UExoFootstepComponent::GetSoundForSurface(EExoSurfaceType Surface) const
{
	const TSoftObjectPtr<USoundBase>* Ptr = nullptr;

	switch (Surface)
	{
	case EExoSurfaceType::Metal:    Ptr = &MetalFootstep;    break;
	case EExoSurfaceType::Dirt:     Ptr = &DirtFootstep;     break;
	case EExoSurfaceType::Water:    Ptr = &WaterFootstep;    break;
	case EExoSurfaceType::Concrete: Ptr = &ConcreteFootstep; break;
	case EExoSurfaceType::Default:
	default:                        Ptr = &DefaultFootstep;  break;
	}

	return (Ptr && Ptr->IsValid()) ? Ptr->Get() : nullptr;
}
