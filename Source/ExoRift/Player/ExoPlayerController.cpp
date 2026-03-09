#include "Player/ExoPlayerController.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Player/ExoAbilityComponent.h"
#include "Core/ExoGameSettings.h"
#include "UI/ExoPingSystem.h"
#include "UI/ExoCommsWheel.h"
#include "UI/ExoSettingsMenu.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "ExoRift.h"

// Helper macro to reduce constructor boilerplate
#define LOAD_IA(VarName, Path) \
	{ static ConstructorHelpers::FObjectFinder<UInputAction> F(TEXT(Path)); \
	  if (F.Succeeded()) VarName = F.Object; }

AExoPlayerController::AExoPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(
		TEXT("/Game/Input/IMC_Default"));
	if (IMCFinder.Succeeded()) DefaultMappingContext = IMCFinder.Object;

	LOAD_IA(MoveAction, "/Game/Input/Actions/IA_Move");
	LOAD_IA(LookAction, "/Game/Input/Actions/IA_Look");
	LOAD_IA(JumpAction, "/Game/Input/Actions/IA_Jump");
	LOAD_IA(FireAction, "/Game/Variant_Shooter/Input/Actions/IA_Shoot");
	LOAD_IA(SwapWeaponAction, "/Game/Variant_Shooter/Input/Actions/IA_SwapWeapon");
	LOAD_IA(InteractAction, "/Game/Input/Actions/IA_Interact");
	LOAD_IA(DropWeaponAction, "/Game/Input/Actions/IA_Drop");
	LOAD_IA(CrouchAction, "/Game/Input/Actions/IA_Crouch");
	LOAD_IA(PingAction, "/Game/Input/Actions/IA_Ping");
	LOAD_IA(Ability1Action, "/Game/Input/Actions/IA_Ability1");
	LOAD_IA(Ability2Action, "/Game/Input/Actions/IA_Ability2");
	LOAD_IA(Ability3Action, "/Game/Input/Actions/IA_Ability3");
	LOAD_IA(PauseAction, "/Game/Input/Actions/IA_Pause");
	LOAD_IA(MenuUpAction, "/Game/Input/Actions/IA_MenuUp");
	LOAD_IA(MenuDownAction, "/Game/Input/Actions/IA_MenuDown");
	LOAD_IA(MenuLeftAction, "/Game/Input/Actions/IA_MenuLeft");
	LOAD_IA(MenuRightAction, "/Game/Input/Actions/IA_MenuRight");
	LOAD_IA(CommsAction, "/Game/Input/Actions/IA_Comms");
}

#undef LOAD_IA

void AExoPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AExoPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetupEnhancedInput();

	// Apply saved settings (sensitivity + FOV) to the new pawn
	if (UExoGameSettings* Settings = UExoGameSettings::Get(GetWorld()))
	{
		Settings->ApplySettings(GetWorld());
	}
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
	if (DropWeaponAction)
		EIC->BindAction(DropWeaponAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleDropWeapon);
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

	if (CommsAction)
	{
		EIC->BindAction(CommsAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleCommsOpen);
		EIC->BindAction(CommsAction, ETriggerEvent::Completed, this, &AExoPlayerController::HandleCommsClose);
	}

	// Menu / settings bindings — always active
	if (PauseAction)
		EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &AExoPlayerController::HandlePause);
	if (MenuUpAction)
		EIC->BindAction(MenuUpAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuUp);
	if (MenuDownAction)
		EIC->BindAction(MenuDownAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuDown);
	if (MenuLeftAction)
		EIC->BindAction(MenuLeftAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuLeft);
	if (MenuRightAction)
		EIC->BindAction(MenuRightAction, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuRight);
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

// --- Death → Spectator transition ---

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

// --- Movement / Camera ---

void AExoPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (FExoSettingsMenu::bIsOpen) return;
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
	if (FExoSettingsMenu::bIsOpen) return;
	FVector2D Input = Value.Get<FVector2D>();

	if (FExoCommsWheel::bIsOpen)
	{
		FExoCommsWheel::UpdateMouse(Input);
		return;
	}

	AddYawInput(Input.X);
	AddPitchInput(Input.Y);
}

void AExoPlayerController::HandleJump()
{
	if (FExoSettingsMenu::bIsOpen) return;
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

// --- Fire (spectate-next in spectator mode) ---

void AExoPlayerController::HandleFire()
{
	if (FExoSettingsMenu::bIsOpen) return;
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

// --- Swap weapon (spectate-prev in spectator mode) ---

void AExoPlayerController::HandleSwapWeapon()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (AExoSpectatorPawn* Spec = Cast<AExoSpectatorPawn>(GetPawn()))
	{
		HandleSpectatePrev();
		return;
	}

	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->SwapWeapon();
}

// --- Spectator cycling ---

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

// --- Sprint ---

void AExoPlayerController::HandleSprint()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StartSprint();
}

void AExoPlayerController::HandleSprintReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StopSprint();
}

// --- Crouch / Slide ---

void AExoPlayerController::HandleCrouch()
{
	if (FExoSettingsMenu::bIsOpen) return;
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

// --- Interaction ---

void AExoPlayerController::HandleInteract()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* ExoChar = Cast<AExoCharacter>(GetPawn());
	if (!ExoChar) return;

	UExoInteractionComponent* InterComp = ExoChar->GetInteractionComponent();
	if (InterComp)
	{
		InterComp->TryInteract();
	}
}

// --- Drop weapon ---

void AExoPlayerController::HandleDropWeapon()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* ExoChar = Cast<AExoCharacter>(GetPawn());
	if (!ExoChar) return;

	UExoInventoryComponent* Inv = ExoChar->GetInventoryComponent();
	if (Inv)
	{
		Inv->DropWeapon(Inv->GetCurrentSlotIndex());
	}
}

// --- Ping ---

void AExoPlayerController::HandlePing()
{
	if (FExoSettingsMenu::bIsOpen) return;
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

// --- Abilities ---

void AExoPlayerController::HandleAbility1()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::Dash);
}

void AExoPlayerController::HandleAbility2()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::AreaScan);
}

void AExoPlayerController::HandleAbility3()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::ShieldBubble);
}

// --- Comms Wheel ---

void AExoPlayerController::HandleCommsOpen()
{
	if (FExoSettingsMenu::bIsOpen) return;
	FExoCommsWheel::Open();
}

void AExoPlayerController::HandleCommsClose()
{
	FExoCommsWheel::Close(GetWorld());
}

// --- Settings Menu ---

void AExoPlayerController::HandlePause()  { FExoSettingsMenu::ToggleMenu(); }
void AExoPlayerController::HandleMenuUp() { FExoSettingsMenu::NavigateUp(); }
void AExoPlayerController::HandleMenuDown() { FExoSettingsMenu::NavigateDown(); }
void AExoPlayerController::HandleMenuLeft() { FExoSettingsMenu::AdjustValue(-1.f); }
void AExoPlayerController::HandleMenuRight() { FExoSettingsMenu::AdjustValue(1.f); }
