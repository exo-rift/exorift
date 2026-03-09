#include "Player/ExoPlayerController.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoAbilityComponent.h"
#include "UI/ExoPingSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "ExoRift.h"

AExoPlayerController::AExoPlayerController()
{
	// Load existing input assets from Content
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(
		TEXT("/Game/Input/IMC_Default"));
	if (IMCFinder.Succeeded()) DefaultMappingContext = IMCFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveFinder(
		TEXT("/Game/Input/Actions/IA_Move"));
	if (MoveFinder.Succeeded()) MoveAction = MoveFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> LookFinder(
		TEXT("/Game/Input/Actions/IA_Look"));
	if (LookFinder.Succeeded()) LookAction = LookFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpFinder(
		TEXT("/Game/Input/Actions/IA_Jump"));
	if (JumpFinder.Succeeded()) JumpAction = JumpFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> FireFinder(
		TEXT("/Game/Variant_Shooter/Input/Actions/IA_Shoot"));
	if (FireFinder.Succeeded()) FireAction = FireFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> SwapFinder(
		TEXT("/Game/Variant_Shooter/Input/Actions/IA_SwapWeapon"));
	if (SwapFinder.Succeeded()) SwapWeaponAction = SwapFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractFinder(
		TEXT("/Game/Input/Actions/IA_Interact"));
	if (InteractFinder.Succeeded()) InteractAction = InteractFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> CrouchFinder(
		TEXT("/Game/Input/Actions/IA_Crouch"));
	if (CrouchFinder.Succeeded()) CrouchAction = CrouchFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> PingFinder(
		TEXT("/Game/Input/Actions/IA_Ping"));
	if (PingFinder.Succeeded()) PingAction = PingFinder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> Ability1Finder(
		TEXT("/Game/Input/Actions/IA_Ability1"));
	if (Ability1Finder.Succeeded()) Ability1Action = Ability1Finder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> Ability2Finder(
		TEXT("/Game/Input/Actions/IA_Ability2"));
	if (Ability2Finder.Succeeded()) Ability2Action = Ability2Finder.Object;

	static ConstructorHelpers::FObjectFinder<UInputAction> Ability3Finder(
		TEXT("/Game/Input/Actions/IA_Ability3"));
	if (Ability3Finder.Succeeded()) Ability3Action = Ability3Finder.Object;
}

void AExoPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AExoPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// Add mapping context after possession per UE5 requirement
	SetupEnhancedInput();
}

void AExoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC) return;

	if (MoveAction)
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AExoPlayerController::HandleMove);
	if (LookAction)
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AExoPlayerController::HandleLook);
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleJump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AExoPlayerController::HandleJumpReleased);
	}
	if (FireAction)
	{
		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleFire);
		EIC->BindAction(FireAction, ETriggerEvent::Completed, this, &AExoPlayerController::HandleFireReleased);
	}
	if (SwapWeaponAction)
		EIC->BindAction(SwapWeaponAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleSwapWeapon);
	if (SprintAction)
	{
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleSprint);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &AExoPlayerController::HandleSprintReleased);
	}
	if (InteractAction)
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleInteract);
	if (CrouchAction)
	{
		EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleCrouch);
		EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AExoPlayerController::HandleCrouchReleased);
	}
	if (PingAction)
		EIC->BindAction(PingAction, ETriggerEvent::Started, this, &AExoPlayerController::HandlePing);
	if (Ability1Action)
		EIC->BindAction(Ability1Action, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility1);
	if (Ability2Action)
		EIC->BindAction(Ability2Action, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility2);
	if (Ability3Action)
		EIC->BindAction(Ability3Action, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility3);
}

void AExoPlayerController::SetupEnhancedInput()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

// ---------------------------------------------------------------------------
// Death → Spectator transition
// ---------------------------------------------------------------------------

void AExoPlayerController::OnCharacterDied(AController* Killer)
{
	if (bIsSpectating) return;
	bIsSpectating = true;

	APawn* DeadPawn = GetPawn();
	if (!DeadPawn) return;

	FVector DeathLoc = DeadPawn->GetActorLocation();
	FRotator DeathRot = DeadPawn->GetActorRotation();

	// Spawn spectator pawn at death location
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	AExoSpectatorPawn* Spectator = GetWorld()->SpawnActor<AExoSpectatorPawn>(
		AExoSpectatorPawn::StaticClass(), DeathLoc, DeathRot, SpawnParams);

	if (!Spectator) return;

	// Release the dead character and possess the spectator
	UnPossess();
	Possess(Spectator);

	// Start the death camera sequence
	AActor* KillerPawn = Killer ? Killer->GetPawn() : nullptr;
	Spectator->StartDeathCam(KillerPawn, DeathLoc);

	UE_LOG(LogExoRift, Log, TEXT("PlayerController transitioned to spectator"));
}

// ---------------------------------------------------------------------------
// Movement / Camera
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleMove(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (APawn* P = GetPawn())
	{
		FRotator Rot = GetControlRotation();
		FRotator YawRot(0.f, Rot.Yaw, 0.f);
		FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		P->AddMovementInput(Forward, Input.Y);
		P->AddMovementInput(Right, Input.X);
	}
}

void AExoPlayerController::HandleLook(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	AddYawInput(Input.X);
	AddPitchInput(Input.Y);
}

void AExoPlayerController::HandleJump()
{
	AExoCharacter* ExoC = Cast<AExoCharacter>(GetPawn());
	if (!ExoC) return;

	if (ExoC->GetCharacterMovement()->IsFalling())
	{
		// In the air — attempt mantle
		ExoC->TryMantle();
	}
	else
	{
		ExoC->Jump();
	}
}

void AExoPlayerController::HandleJumpReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StopJumping();
}

// ---------------------------------------------------------------------------
// Fire — doubles as spectate-next when in spectator mode
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleFire()
{
	if (AExoSpectatorPawn* Spec = Cast<AExoSpectatorPawn>(GetPawn()))
	{
		HandleSpectateNext();
		return;
	}

	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StartFire();
}

void AExoPlayerController::HandleFireReleased()
{
	// No action needed for spectator; only relevant for live character
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StopFire();
}

// ---------------------------------------------------------------------------
// Swap weapon — doubles as spectate-prev when in spectator mode
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleSwapWeapon()
{
	if (AExoSpectatorPawn* Spec = Cast<AExoSpectatorPawn>(GetPawn()))
	{
		HandleSpectatePrev();
		return;
	}

	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->SwapWeapon();
}

// ---------------------------------------------------------------------------
// Spectator cycling helpers
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleSpectateNext()
{
	if (AExoSpectatorPawn* Spec = Cast<AExoSpectatorPawn>(GetPawn()))
	{
		Spec->CycleSpectateTarget(1);
	}
}

void AExoPlayerController::HandleSpectatePrev()
{
	if (AExoSpectatorPawn* Spec = Cast<AExoSpectatorPawn>(GetPawn()))
	{
		Spec->CycleSpectateTarget(-1);
	}
}

// ---------------------------------------------------------------------------
// Sprint
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleSprint()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StartSprint();
}

void AExoPlayerController::HandleSprintReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StopSprint();
}

// ---------------------------------------------------------------------------
// Crouch / Slide
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleCrouch()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;

	if (C->IsSprinting())
	{
		C->StartSlide();
	}
	else
	{
		C->Crouch();
	}
}

void AExoPlayerController::HandleCrouchReleased()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;

	if (C->IsSliding())
	{
		C->StopSlide();
	}
	else
	{
		C->UnCrouch();
	}
}

// ---------------------------------------------------------------------------
// Interaction
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleInteract()
{
	AExoCharacter* ExoChar = Cast<AExoCharacter>(GetPawn());
	if (!ExoChar) return;

	UExoInteractionComponent* InterComp = ExoChar->GetInteractionComponent();
	if (InterComp)
	{
		InterComp->TryInteract();
	}
}

// ---------------------------------------------------------------------------
// Ping
// ---------------------------------------------------------------------------

void AExoPlayerController::HandlePing()
{
	AExoCharacter* ExoChar = Cast<AExoCharacter>(GetPawn());
	if (!ExoChar) return;

	UCameraComponent* Cam = ExoChar->GetFirstPersonCamera();
	if (!Cam) return;

	FVector Start = Cam->GetComponentLocation();
	FVector End = Start + Cam->GetForwardVector() * 50000.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(ExoChar);

	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	FVector PingLocation = bHit ? Hit.ImpactPoint : End;
	FExoPingSystem::AddPing(PingLocation, TEXT("Enemy Here"), FLinearColor(1.f, 0.4f, 0.1f, 1.f));
}

// ---------------------------------------------------------------------------
// Abilities
// ---------------------------------------------------------------------------

void AExoPlayerController::HandleAbility1()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::Dash);
}

void AExoPlayerController::HandleAbility2()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::AreaScan);
}

void AExoPlayerController::HandleAbility3()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::ShieldBubble);
}
