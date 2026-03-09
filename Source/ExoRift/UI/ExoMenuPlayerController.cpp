#include "UI/ExoMenuPlayerController.h"
#include "UI/ExoMenuHUD.h"

AExoMenuPlayerController::AExoMenuPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AExoMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void AExoMenuPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent) return;

	// Navigation
	InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuUp);
	InputComponent->BindKey(EKeys::W, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuUp);
	InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuDown);
	InputComponent->BindKey(EKeys::S, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuDown);

	// Select
	InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuSelect);
	InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuSelect);
	InputComponent->BindKey(EKeys::Right, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuSelect);

	// Back
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuBack);
	InputComponent->BindKey(EKeys::Left, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuBack);
	InputComponent->BindKey(EKeys::BackSpace, IE_Pressed, this, &AExoMenuPlayerController::HandleMenuBack);
}

AExoMenuHUD* AExoMenuPlayerController::GetMenuHUD() const
{
	return Cast<AExoMenuHUD>(GetHUD());
}

void AExoMenuPlayerController::HandleMenuUp()
{
	if (AExoMenuHUD* HUD = GetMenuHUD()) HUD->NavigateUp();
}

void AExoMenuPlayerController::HandleMenuDown()
{
	if (AExoMenuHUD* HUD = GetMenuHUD()) HUD->NavigateDown();
}

void AExoMenuPlayerController::HandleMenuSelect()
{
	if (AExoMenuHUD* HUD = GetMenuHUD()) HUD->SelectCurrent();
}

void AExoMenuPlayerController::HandleMenuBack()
{
	if (AExoMenuHUD* HUD = GetMenuHUD()) HUD->GoBack();
}
