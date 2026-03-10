// ExoPlayerControllerActions.cpp — Social, menu, spectator, and endgame handlers
#include "Player/ExoPlayerController.h"
#include "Player/ExoCharacter.h"
#include "Player/ExoSpectatorPawn.h"
#include "Player/ExoAbilityComponent.h"
#include "Player/ExoEmoteComponent.h"
#include "Player/ExoPlayerCustomization.h"
#include "Core/ExoGameState.h"
#include "Core/ExoGameMode.h"
#include "Core/ExoTypes.h"
#include "UI/ExoCommsWheel.h"
#include "UI/ExoSettingsMenu.h"
#include "UI/ExoTacticalMap.h"
#include "ExoRift.h"

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

void AExoPlayerController::HandleAbility4()
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (C && C->GetAbilityComponent()) C->GetAbilityComponent()->UseAbility(EExoAbilityType::GrappleHook);
}

// --- Social ---

void AExoPlayerController::HandleEmote1() { PlayEmoteSlot(0); }
void AExoPlayerController::HandleEmote2() { PlayEmoteSlot(1); }
void AExoPlayerController::HandleEmote3() { PlayEmoteSlot(2); }
void AExoPlayerController::HandleEmote4() { PlayEmoteSlot(3); }

void AExoPlayerController::PlayEmoteSlot(int32 SlotIndex)
{
	if (FExoSettingsMenu::bIsOpen) return;
	AExoCharacter* C = Cast<AExoCharacter>(GetPawn());
	if (!C || !C->CanPerformActions()) return;
	UExoEmoteComponent* EC = C->GetEmoteComponent();
	if (!EC) return;
	UExoPlayerCustomization* Cust = UExoPlayerCustomization::Get(GetWorld());
	int32 EmoteIndex = Cust ? Cust->EmoteLoadout[FMath::Clamp(SlotIndex, 0, 3)] : SlotIndex;
	EC->PlayEmote(EmoteIndex);
}

void AExoPlayerController::HandleCommsOpen()
{
	if (FExoSettingsMenu::bIsOpen) return;
	FExoCommsWheel::Open();
}

void AExoPlayerController::HandleCommsClose() { FExoCommsWheel::Close(GetWorld()); }

// --- Spectator ---

void AExoPlayerController::HandleSpectateNext()
{
	if (auto* S = Cast<AExoSpectatorPawn>(GetPawn())) S->CycleSpectateTarget(1);
}

void AExoPlayerController::HandleSpectatePrev()
{
	if (auto* S = Cast<AExoSpectatorPawn>(GetPawn())) S->CycleSpectateTarget(-1);
}

// --- Menu ---

void AExoPlayerController::HandleTacticalMap() { FExoTacticalMap::bIsOpen = !FExoTacticalMap::bIsOpen; }
void AExoPlayerController::HandlePause()     { FExoSettingsMenu::ToggleMenu(); }
void AExoPlayerController::HandleMenuUp()    { FExoSettingsMenu::NavigateUp(); }
void AExoPlayerController::HandleMenuDown()  { FExoSettingsMenu::NavigateDown(); }
void AExoPlayerController::HandleMenuLeft()  { FExoSettingsMenu::AdjustValue(-1.f); }
void AExoPlayerController::HandleMenuRight() { FExoSettingsMenu::AdjustValue(1.f); }

// --- Endgame ---

void AExoPlayerController::HandleRestartMatch()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS || GS->MatchPhase != EBRMatchPhase::EndGame) return;
	if (auto* GM = Cast<AExoGameMode>(GetWorld()->GetAuthGameMode())) GM->RestartMatch();
}

void AExoPlayerController::HandleReturnToMenu()
{
	AExoGameState* GS = GetWorld()->GetGameState<AExoGameState>();
	if (!GS || GS->MatchPhase != EBRMatchPhase::EndGame) return;
	if (auto* GM = Cast<AExoGameMode>(GetWorld()->GetAuthGameMode())) GM->ReturnToMainMenu();
}
