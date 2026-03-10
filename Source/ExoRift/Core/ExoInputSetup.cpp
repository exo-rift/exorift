#include "Core/ExoInputSetup.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "ExoRift.h"

FExoInputSetup& FExoInputSetup::Get()
{
	static FExoInputSetup Instance;
	if (!Instance.bInitialized)
	{
		Instance.Initialize();
		Instance.bInitialized = true;
	}
	return Instance;
}

static UInputAction* MakeAction(const TCHAR* Name, EInputActionValueType Type)
{
	UInputAction* A = NewObject<UInputAction>(GetTransientPackage(), Name);
	A->ValueType = Type;
	A->AddToRoot();
	return A;
}

static void MapWASD(UInputMappingContext* IMC, UInputAction* Action)
{
	// W = Forward (+Y)
	{
		FEnhancedActionKeyMapping& M = IMC->MapKey(Action, EKeys::W);
		auto* S = NewObject<UInputModifierSwizzleAxis>();
		S->Order = EInputAxisSwizzle::YXZ;
		M.Modifiers.Add(S);
	}
	// S = Backward (-Y)
	{
		FEnhancedActionKeyMapping& M = IMC->MapKey(Action, EKeys::S);
		auto* S = NewObject<UInputModifierSwizzleAxis>();
		S->Order = EInputAxisSwizzle::YXZ;
		M.Modifiers.Add(S);
		M.Modifiers.Add(NewObject<UInputModifierNegate>());
	}
	// D = Right (+X)
	IMC->MapKey(Action, EKeys::D);
	// A = Left (-X)
	{
		FEnhancedActionKeyMapping& M = IMC->MapKey(Action, EKeys::A);
		M.Modifiers.Add(NewObject<UInputModifierNegate>());
	}
}

void FExoInputSetup::Initialize()
{
	auto B = EInputActionValueType::Boolean;
	auto A1 = EInputActionValueType::Axis1D;
	auto A2 = EInputActionValueType::Axis2D;

	// Create all input actions
	Move         = MakeAction(TEXT("IA_Move"),         A2);
	Look         = MakeAction(TEXT("IA_Look"),         A2);
	Jump         = MakeAction(TEXT("IA_Jump"),         B);
	Sprint       = MakeAction(TEXT("IA_Sprint"),       B);
	Crouch       = MakeAction(TEXT("IA_Crouch"),       B);
	Fire         = MakeAction(TEXT("IA_Fire"),         B);
	AimDownSight = MakeAction(TEXT("IA_ADS"),          B);
	FireMode     = MakeAction(TEXT("IA_FireMode"),     B);
	SwapWeapon   = MakeAction(TEXT("IA_SwapWeapon"),   B);
	DropWeapon   = MakeAction(TEXT("IA_Drop"),         B);
	Grenade      = MakeAction(TEXT("IA_Grenade"),      B);
	Melee        = MakeAction(TEXT("IA_Melee"),        B);
	Inspect      = MakeAction(TEXT("IA_Inspect"),      B);
	ScrollWeapon = MakeAction(TEXT("IA_ScrollWeapon"), A1);
	Interact     = MakeAction(TEXT("IA_Interact"),     B);
	Ping         = MakeAction(TEXT("IA_Ping"),         B);
	Ability1     = MakeAction(TEXT("IA_Ability1"),     B);
	Ability2     = MakeAction(TEXT("IA_Ability2"),     B);
	Ability3     = MakeAction(TEXT("IA_Ability3"),     B);
	Ability4     = MakeAction(TEXT("IA_Ability4"),     B);
	TacticalMap  = MakeAction(TEXT("IA_TacticalMap"),  B);
	Pause        = MakeAction(TEXT("IA_Pause"),        B);
	MenuUp       = MakeAction(TEXT("IA_MenuUp"),       B);
	MenuDown     = MakeAction(TEXT("IA_MenuDown"),     B);
	MenuLeft     = MakeAction(TEXT("IA_MenuLeft"),     B);
	MenuRight    = MakeAction(TEXT("IA_MenuRight"),    B);
	Comms        = MakeAction(TEXT("IA_Comms"),        B);
	Emote1       = MakeAction(TEXT("IA_Emote1"),       B);
	Emote2       = MakeAction(TEXT("IA_Emote2"),       B);
	Emote3       = MakeAction(TEXT("IA_Emote3"),       B);
	Emote4       = MakeAction(TEXT("IA_Emote4"),       B);
	Restart      = MakeAction(TEXT("IA_Restart"),      B);
	ReturnToMenu = MakeAction(TEXT("IA_ReturnMenu"),   B);

	// Build the mapping context
	MappingContext = NewObject<UInputMappingContext>(GetTransientPackage(), TEXT("IMC_ExoRift"));
	MappingContext->AddToRoot();

	// WASD movement (Axis2D with swizzle/negate modifiers)
	MapWASD(MappingContext, Move);

	// Mouse look
	MappingContext->MapKey(Look, EKeys::Mouse2D);

	// Weapon scroll (Axis1D)
	MappingContext->MapKey(ScrollWeapon, EKeys::MouseWheelAxis);

	// All boolean key bindings
	MappingContext->MapKey(Jump,         EKeys::SpaceBar);
	MappingContext->MapKey(Fire,         EKeys::LeftMouseButton);
	MappingContext->MapKey(AimDownSight, EKeys::RightMouseButton);
	MappingContext->MapKey(FireMode,     EKeys::B);
	MappingContext->MapKey(SwapWeapon,   EKeys::Q);
	MappingContext->MapKey(Sprint,       EKeys::LeftShift);
	MappingContext->MapKey(Interact,     EKeys::E);
	MappingContext->MapKey(DropWeapon,   EKeys::G);
	MappingContext->MapKey(Crouch,       EKeys::C);
	MappingContext->MapKey(Melee,        EKeys::X);
	MappingContext->MapKey(Inspect,      EKeys::T);
	MappingContext->MapKey(Ping,         EKeys::MiddleMouseButton);
	MappingContext->MapKey(Ability1,     EKeys::One);
	MappingContext->MapKey(Ability2,     EKeys::Two);
	MappingContext->MapKey(Ability3,     EKeys::Three);
	MappingContext->MapKey(Ability4,     EKeys::Four);
	MappingContext->MapKey(TacticalMap,  EKeys::M);
	MappingContext->MapKey(Pause,        EKeys::Escape);
	MappingContext->MapKey(MenuUp,       EKeys::Up);
	MappingContext->MapKey(MenuDown,     EKeys::Down);
	MappingContext->MapKey(MenuLeft,     EKeys::Left);
	MappingContext->MapKey(MenuRight,    EKeys::Right);
	MappingContext->MapKey(Grenade,      EKeys::F);
	MappingContext->MapKey(Comms,        EKeys::V);
	MappingContext->MapKey(Emote1,       EKeys::F1);
	MappingContext->MapKey(Emote2,       EKeys::F2);
	MappingContext->MapKey(Emote3,       EKeys::F3);
	MappingContext->MapKey(Emote4,       EKeys::F4);
	MappingContext->MapKey(Restart,      EKeys::Enter);
	MappingContext->MapKey(ReturnToMenu, EKeys::BackSpace);

	UE_LOG(LogExoRift, Log, TEXT("Input setup: 33 actions, 38 key bindings"));
}
