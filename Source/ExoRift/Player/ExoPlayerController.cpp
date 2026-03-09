#include "Player/ExoPlayerController.h"
#include "Player/ExoCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
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
	if (ACharacter* C = Cast<ACharacter>(GetPawn()))
		C->Jump();
}

void AExoPlayerController::HandleJumpReleased()
{
	if (ACharacter* C = Cast<ACharacter>(GetPawn()))
		C->StopJumping();
}

void AExoPlayerController::HandleFire()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StartFire();
}

void AExoPlayerController::HandleFireReleased()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->StopFire();
}

void AExoPlayerController::HandleSwapWeapon()
{
	if (AExoCharacter* C = Cast<AExoCharacter>(GetPawn()))
		C->SwapWeapon();
}

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
