// ExoPlayerController.cpp — Input setup, movement, combat, interaction handlers
#include "Player/ExoPlayerController.h"
#include "Player/ExoCharacter.h"
#include "Weapons/ExoWeaponBase.h"
#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoInteractionComponent.h"
#include "Player/ExoInventoryComponent.h"
#include "Core/ExoInputSetup.h"
#include "Core/ExoGameSettings.h"
#include "Core/ExoPlayerState.h"
#include "UI/ExoPingSystem.h"
#include "UI/ExoSettingsMenu.h"
#include "UI/ExoCommsWheel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "ExoRift.h"

AExoPlayerController::AExoPlayerController() {}

void AExoPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AExoPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetupEnhancedInput();

	if (UExoGameSettings* Settings = UExoGameSettings::Get(GetWorld()))
		Settings->ApplySettings(GetWorld());
	if (AExoPlayerState* PS = GetPlayerState<AExoPlayerState>())
		PS->InitDisplayNameFromCustomization(GetWorld());
}

void AExoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC) return;

	FExoInputSetup& In = FExoInputSetup::Get();

	// Axis actions (Triggered fires every frame while active)
	EIC->BindAction(In.Move, ETriggerEvent::Triggered, this, &AExoPlayerController::HandleMove);
	EIC->BindAction(In.Look, ETriggerEvent::Triggered, this, &AExoPlayerController::HandleLook);
	EIC->BindAction(In.ScrollWeapon, ETriggerEvent::Triggered, this, &AExoPlayerController::HandleScrollWeapon);

	// Press/release pairs
	EIC->BindAction(In.Jump, ETriggerEvent::Started, this, &AExoPlayerController::HandleJump);
	EIC->BindAction(In.Jump, ETriggerEvent::Completed, this, &AExoPlayerController::HandleJumpReleased);
	EIC->BindAction(In.Fire, ETriggerEvent::Started, this, &AExoPlayerController::HandleFire);
	EIC->BindAction(In.Fire, ETriggerEvent::Completed, this, &AExoPlayerController::HandleFireReleased);
	EIC->BindAction(In.AimDownSight, ETriggerEvent::Started, this, &AExoPlayerController::HandleADS);
	EIC->BindAction(In.AimDownSight, ETriggerEvent::Completed, this, &AExoPlayerController::HandleADSReleased);
	EIC->BindAction(In.FireMode, ETriggerEvent::Started, this, &AExoPlayerController::HandleFireMode);
	EIC->BindAction(In.Sprint, ETriggerEvent::Started, this, &AExoPlayerController::HandleSprint);
	EIC->BindAction(In.Sprint, ETriggerEvent::Completed, this, &AExoPlayerController::HandleSprintReleased);
	EIC->BindAction(In.Crouch, ETriggerEvent::Started, this, &AExoPlayerController::HandleCrouch);
	EIC->BindAction(In.Crouch, ETriggerEvent::Completed, this, &AExoPlayerController::HandleCrouchReleased);
	EIC->BindAction(In.Comms, ETriggerEvent::Started, this, &AExoPlayerController::HandleCommsOpen);
	EIC->BindAction(In.Comms, ETriggerEvent::Completed, this, &AExoPlayerController::HandleCommsClose);

	// Single-press actions
	EIC->BindAction(In.SwapWeapon, ETriggerEvent::Started, this, &AExoPlayerController::HandleSwapWeapon);
	EIC->BindAction(In.Interact, ETriggerEvent::Started, this, &AExoPlayerController::HandleInteract);
	EIC->BindAction(In.DropWeapon, ETriggerEvent::Started, this, &AExoPlayerController::HandleDropWeapon);
	EIC->BindAction(In.Ping, ETriggerEvent::Started, this, &AExoPlayerController::HandlePing);
	EIC->BindAction(In.Grenade, ETriggerEvent::Started, this, &AExoPlayerController::HandleGrenade);
	EIC->BindAction(In.Melee, ETriggerEvent::Started, this, &AExoPlayerController::HandleMelee);
	EIC->BindAction(In.Inspect, ETriggerEvent::Started, this, &AExoPlayerController::HandleInspect);
	EIC->BindAction(In.Inspect, ETriggerEvent::Completed, this, &AExoPlayerController::HandleInspectReleased);
	EIC->BindAction(In.Ability1, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility1);
	EIC->BindAction(In.Ability2, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility2);
	EIC->BindAction(In.Ability3, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility3);
	EIC->BindAction(In.Ability4, ETriggerEvent::Started, this, &AExoPlayerController::HandleAbility4);

	// Emotes
	EIC->BindAction(In.Emote1, ETriggerEvent::Started, this, &AExoPlayerController::HandleEmote1);
	EIC->BindAction(In.Emote2, ETriggerEvent::Started, this, &AExoPlayerController::HandleEmote2);
	EIC->BindAction(In.Emote3, ETriggerEvent::Started, this, &AExoPlayerController::HandleEmote3);
	EIC->BindAction(In.Emote4, ETriggerEvent::Started, this, &AExoPlayerController::HandleEmote4);

	// Menu navigation
	EIC->BindAction(In.TacticalMap, ETriggerEvent::Started, this, &AExoPlayerController::HandleTacticalMap);
	EIC->BindAction(In.Pause, ETriggerEvent::Started, this, &AExoPlayerController::HandlePause);
	EIC->BindAction(In.MenuUp, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuUp);
	EIC->BindAction(In.MenuDown, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuDown);
	EIC->BindAction(In.MenuLeft, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuLeft);
	EIC->BindAction(In.MenuRight, ETriggerEvent::Started, this, &AExoPlayerController::HandleMenuRight);

	// Endgame
	EIC->BindAction(In.Restart, ETriggerEvent::Started, this, &AExoPlayerController::HandleRestartMatch);
	EIC->BindAction(In.ReturnToMenu, ETriggerEvent::Started, this, &AExoPlayerController::HandleReturnToMenu);
}

void AExoPlayerController::SetupEnhancedInput()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(FExoInputSetup::Get().MappingContext, 0);
	}
}

// --- Death -> Spectator ---

void AExoPlayerController::OnCharacterDied(AController* Killer)
{
	if (bIsSpectating) return;
	bIsSpectating = true;

	APawn* DeadPawn = GetPawn();
	if (!DeadPawn) return;

	FVector DeathLoc = DeadPawn->GetActorLocation();
	FRotator DeathRot = DeadPawn->GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	AExoSpectatorPawn* Spectator = GetWorld()->SpawnActor<AExoSpectatorPawn>(
		AExoSpectatorPawn::StaticClass(), DeathLoc, DeathRot, SpawnParams);
	if (!Spectator) return;

	UnPossess();
	Possess(Spectator);

	AActor* KillerPawn = Killer ? Killer->GetPawn() : nullptr;
	Spectator->StartDeathCam(KillerPawn, DeathLoc);
	UE_LOG(LogExoRift, Log, TEXT("Transitioned to spectator"));
}

// --- Movement ---

void AExoPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (FExoSettingsMenu::bIsOpen) return;
	FVector2D Input = Value.Get<FVector2D>();
	if (APawn* P = GetPawn())
	{
		FRotator YawRot(0.f, GetControlRotation().Yaw, 0.f);
		P->AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::X), Input.Y);
		P->AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y), Input.X);
	}
}

void AExoPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (FExoSettingsMenu::bIsOpen) return;
	FVector2D Input = Value.Get<FVector2D>();
	if (FExoCommsWheel::bIsOpen) { FExoCommsWheel::UpdateMouse(Input); return; }
	AddYawInput(Input.X);
	AddPitchInput(Input.Y);
}

void AExoPlayerController::HandleJump()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;
	C->GetCharacterMovement()->IsFalling() ? C->TryMantle() : C->Jump();
}

void AExoPlayerController::HandleJumpReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StopJumping();
}

void AExoPlayerController::HandleSprint()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StartSprint();
}

void AExoPlayerController::HandleSprintReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StopSprint();
}

void AExoPlayerController::HandleCrouch()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;
	C->IsSprinting() ? C->StartSlide() : C->Crouch();
}

void AExoPlayerController::HandleCrouchReleased()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;
	C->IsSliding() ? C->StopSlide() : C->UnCrouch();
}

// --- Combat ---

void AExoPlayerController::HandleFire()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (Cast<AExoSpectatorPawn>(GetPawn())) { HandleSpectateNext(); return; }
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StartFire();
}

void AExoPlayerController::HandleFireReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StopFire();
}

void AExoPlayerController::HandleADS()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StartADS();
}

void AExoPlayerController::HandleADSReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->StopADS();
}

void AExoPlayerController::HandleFireMode()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->ToggleFireMode();
}

void AExoPlayerController::HandleSwapWeapon()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (Cast<AExoSpectatorPawn>(GetPawn())) { HandleSpectatePrev(); return; }
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->SwapWeapon();
}

void AExoPlayerController::HandleDropWeapon()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;
	if (auto* Inv = C->GetInventoryComponent())
		Inv->DropWeapon(Inv->GetCurrentSlotIndex());
}

void AExoPlayerController::HandleGrenade()
{
	if (FExoSettingsMenu::bIsOpen) return;
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn())) C->ThrowGrenade();
}

void AExoPlayerController::HandleMelee()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetInventoryComponent()) C->GetInventoryComponent()->SwitchToMelee();
}

void AExoPlayerController::HandleInspect()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetCurrentWeapon()) C->GetCurrentWeapon()->StartInspect();
}

void AExoPlayerController::HandleInspectReleased()
{
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetCurrentWeapon()) C->GetCurrentWeapon()->StopInspect();
}

void AExoPlayerController::HandleScrollWeapon(const FInputActionValue& Value)
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C || !C->GetInventoryComponent()) return;
	float Dir = Value.Get<float>();
	C->GetInventoryComponent()->CycleWeapon(Dir > 0.f ? 1 : -1);
}

// --- Interaction ---

void AExoPlayerController::HandleInteract()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C) if (auto* IC = C->GetInteractionComponent()) IC->TryInteract();
}

void AExoPlayerController::HandlePing()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C) return;
	UCameraComponent* Cam = C->GetFirstPersonCamera();
	if (!Cam) return;
	FVector Start = Cam->GetComponentLocation();
	FVector End = Start + Cam->GetForwardVector() * 50000.f;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(C);
	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	FExoPingSystem::AddPing(bHit ? Hit.ImpactPoint : End, TEXT("Enemy Here"),
		FLinearColor(1.f, 0.4f, 0.1f, 1.f));
}

