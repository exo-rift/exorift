// ExoWeaponViewModelBuilders.cpp — Procedural weapon geometry for each type
#include "Visual/ExoWeaponViewModel.h"
#include "Components/StaticMeshComponent.h"

void UExoWeaponViewModel::BuildRifleModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.08f, 0.08f, 0.1f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Trim(0.1f, 0.1f, 0.13f);
	FLinearColor Dim(0.06f, 0.06f, 0.07f);

	// Main receiver
	AddPart(FVector(15.f, 0.f, 0.f), FVector(0.35f, 0.06f, 0.05f), Body);
	// Receiver upper plate
	AddPart(FVector(15.f, 0.f, 2.8f), FVector(0.32f, 0.058f, 0.005f), Dim);
	// Barrel shroud
	AddBarrelPart(FVector(32.f, 0.f, 0.f), FVector(0.12f, 0.04f, 0.04f), Dark);
	// Barrel
	MuzzleTip = AddBarrelPart(FVector(42.f, 0.f, 0.5f), FVector(0.16f, 0.02f, 0.02f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Muzzle brake
	AddBarrelPart(FVector(48.f, 0.f, 0.5f), FVector(0.025f, 0.025f, 0.015f), Trim, CylinderMesh);
	// Muzzle brake slots
	AddBarrelPart(FVector(48.f, 1.5f, 0.5f), FVector(0.02f, 0.002f, 0.01f), Trim);
	AddBarrelPart(FVector(48.f, -1.5f, 0.5f), FVector(0.02f, 0.002f, 0.01f), Trim);
	// Magazine
	AddPart(FVector(10.f, 0.f, -4.f), FVector(0.06f, 0.03f, 0.08f), Dark);
	// Magazine energy window
	AddPart(FVector(10.f, 1.6f, -4.f), FVector(0.04f, 0.003f, 0.04f), Accent);
	// Grip
	AddPart(FVector(0.f, 0.f, -3.f), FVector(0.04f, 0.04f, 0.06f), Body);
	// Trigger guard
	AddPart(FVector(5.f, 0.f, -1.8f), FVector(0.04f, 0.015f, 0.018f), Dark);
	// Scope rail
	AddPart(FVector(18.f, 0.f, 3.5f), FVector(0.14f, 0.015f, 0.008f), Trim);
	// Heat vents (side slits)
	AddPart(FVector(25.f, 3.2f, 0.f), FVector(0.08f, 0.003f, 0.02f), Trim);
	AddPart(FVector(25.f, -3.2f, 0.f), FVector(0.08f, 0.003f, 0.02f), Trim);
	// Energy conduit along barrel (glowing tube)
	AddPart(FVector(38.f, 2.f, 1.f), FVector(0.1f, 0.004f, 0.004f), Accent, CylinderMesh);
	// Accent stripe — glowing rarity color
	AddPart(FVector(20.f, 0.f, 2.5f), FVector(0.22f, 0.065f, 0.005f), Accent);
	// Charging handle
	AddPart(FVector(5.f, 0.f, 2.8f), FVector(0.03f, 0.02f, 0.01f), Trim);
	// Ejection port cover
	AddPart(FVector(14.f, 3.f, 1.f), FVector(0.04f, 0.003f, 0.015f), Trim);
	// Forward grip nub
	AddPart(FVector(25.f, 0.f, -2.5f), FVector(0.025f, 0.025f, 0.04f), Dark);
	// Buttstock
	AddPart(FVector(-12.f, 0.f, 0.f), FVector(0.08f, 0.04f, 0.04f), Dark);
	AddPart(FVector(-16.f, 0.f, 0.f), FVector(0.02f, 0.05f, 0.04f), Trim);
	// Stock accent light
	AddPart(FVector(-16.f, 0.f, 2.f), FVector(0.003f, 0.03f, 0.003f), Accent);
	// Status LED (rear sight area)
	AddPart(FVector(2.f, 0.f, 3.2f), FVector(0.006f, 0.006f, 0.006f), Accent);
	// Side panel lines
	AddPart(FVector(12.f, 3.1f, 0.f), FVector(0.15f, 0.002f, 0.03f), Dim);
	AddPart(FVector(12.f, -3.1f, 0.f), FVector(0.15f, 0.002f, 0.03f), Dim);
}

void UExoWeaponViewModel::BuildSMGModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.07f, 0.07f, 0.09f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Trim(0.09f, 0.09f, 0.12f);
	FLinearColor Dim(0.055f, 0.055f, 0.065f);

	// Compact body
	AddPart(FVector(10.f, 0.f, 0.f), FVector(0.22f, 0.055f, 0.045f), Body);
	// Body top plate
	AddPart(FVector(10.f, 0.f, 2.4f), FVector(0.20f, 0.053f, 0.004f), Dim);
	// Upper rail
	AddPart(FVector(14.f, 0.f, 2.8f), FVector(0.1f, 0.012f, 0.006f), Trim);
	// Short barrel
	MuzzleTip = AddBarrelPart(FVector(28.f, 0.f, 0.5f), FVector(0.08f, 0.018f, 0.018f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Barrel shroud
	AddBarrelPart(FVector(24.f, 0.f, 0.5f), FVector(0.05f, 0.022f, 0.022f), Body, CylinderMesh);
	// Muzzle compensator
	AddBarrelPart(FVector(32.f, 0.f, 0.5f), FVector(0.01f, 0.02f, 0.02f), Trim, CylinderMesh);
	// Magazine (straight)
	AddPart(FVector(8.f, 0.f, -4.f), FVector(0.04f, 0.025f, 0.07f), Dark);
	// Magazine energy indicator
	AddPart(FVector(8.f, 1.3f, -3.f), FVector(0.025f, 0.003f, 0.03f), Accent);
	// Grip
	AddPart(FVector(-2.f, 0.f, -3.f), FVector(0.035f, 0.035f, 0.05f), Body);
	// Trigger guard
	AddPart(FVector(3.f, 0.f, -1.5f), FVector(0.035f, 0.012f, 0.015f), Dark);
	// Folding stock stub
	AddPart(FVector(-8.f, 0.f, 0.5f), FVector(0.06f, 0.02f, 0.02f), Dark);
	// Stock hinge accent
	AddPart(FVector(-5.f, 0.f, 0.5f), FVector(0.004f, 0.015f, 0.015f), Accent, CylinderMesh);
	// Side heat vents
	AddPart(FVector(18.f, 2.8f, 0.f), FVector(0.06f, 0.003f, 0.015f), Trim);
	AddPart(FVector(18.f, -2.8f, 0.f), FVector(0.06f, 0.003f, 0.015f), Trim);
	// Accent band — glowing
	AddPart(FVector(15.f, 0.f, 2.f), FVector(0.15f, 0.06f, 0.004f), Accent);
	// Side status dots
	AddPart(FVector(6.f, 2.9f, 1.f), FVector(0.005f, 0.003f, 0.005f), Accent);
	AddPart(FVector(9.f, 2.9f, 1.f), FVector(0.005f, 0.003f, 0.005f), Accent);
}

void UExoWeaponViewModel::BuildShotgunModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.1f, 0.08f, 0.07f);
	FLinearColor Dark(0.05f, 0.04f, 0.04f);
	FLinearColor Trim(0.12f, 0.1f, 0.09f);
	FLinearColor Dim(0.07f, 0.06f, 0.055f);

	// Thick body
	AddPart(FVector(12.f, 0.f, 0.f), FVector(0.3f, 0.07f, 0.06f), Body);
	// Receiver ridge
	AddPart(FVector(12.f, 0.f, 3.2f), FVector(0.28f, 0.068f, 0.005f), Dim);
	// Wide barrel
	MuzzleTip = AddBarrelPart(FVector(38.f, 0.f, 0.f), FVector(0.12f, 0.03f, 0.03f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Muzzle flare (wider at end)
	AddBarrelPart(FVector(44.f, 0.f, 0.f), FVector(0.015f, 0.035f, 0.035f), Trim, CylinderMesh);
	// Barrel heat shield
	AddBarrelPart(FVector(35.f, 0.f, 2.f), FVector(0.08f, 0.003f, 0.012f), Trim);
	// Pump slide
	AddPart(FVector(22.f, 0.f, -1.5f), FVector(0.08f, 0.04f, 0.035f), Trim);
	// Pump grip ridges
	AddPart(FVector(20.f, 0.f, -1.5f), FVector(0.003f, 0.042f, 0.037f), Dim);
	AddPart(FVector(24.f, 0.f, -1.5f), FVector(0.003f, 0.042f, 0.037f), Dim);
	// Pump rail
	AddPart(FVector(28.f, 0.f, -2.5f), FVector(0.12f, 0.006f, 0.006f), Dark);
	// Grip
	AddPart(FVector(-2.f, 0.f, -4.f), FVector(0.04f, 0.04f, 0.07f), Body);
	// Stock
	AddPart(FVector(-12.f, 0.f, 0.f), FVector(0.1f, 0.04f, 0.04f), Dark);
	// Stock pad
	AddPart(FVector(-17.f, 0.f, 0.f), FVector(0.02f, 0.05f, 0.05f), Trim);
	// Shell indicator lights (3 dots on side)
	AddPart(FVector(8.f, 3.8f, 1.f), FVector(0.01f, 0.005f, 0.005f), Accent);
	AddPart(FVector(11.f, 3.8f, 1.f), FVector(0.01f, 0.005f, 0.005f), Accent);
	AddPart(FVector(14.f, 3.8f, 1.f), FVector(0.01f, 0.005f, 0.005f), Accent);
	// Top accent stripe
	AddPart(FVector(8.f, 0.f, 3.f), FVector(0.1f, 0.075f, 0.004f), Accent);
	// Under-barrel energy conduit
	AddPart(FVector(30.f, 0.f, -3.f), FVector(0.08f, 0.004f, 0.004f), Accent, CylinderMesh);
	// Side panel
	AddPart(FVector(6.f, 3.6f, 0.f), FVector(0.12f, 0.002f, 0.035f), Dim);
}

void UExoWeaponViewModel::BuildSniperModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.06f, 0.06f, 0.08f);
	FLinearColor Dark(0.03f, 0.03f, 0.04f);
	FLinearColor Trim(0.08f, 0.08f, 0.1f);
	FLinearColor Dim(0.045f, 0.045f, 0.06f);

	// Long body
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.45f, 0.05f, 0.04f), Body);
	// Body top plate
	AddPart(FVector(20.f, 0.f, 2.2f), FVector(0.43f, 0.048f, 0.004f), Dim);
	// Long barrel
	MuzzleTip = AddBarrelPart(FVector(55.f, 0.f, 0.5f), FVector(0.22f, 0.015f, 0.015f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Suppressor/muzzle device
	AddBarrelPart(FVector(64.f, 0.f, 0.5f), FVector(0.04f, 0.02f, 0.02f), Trim, CylinderMesh);
	// Barrel stabilizer fins
	AddBarrelPart(FVector(50.f, 0.f, 2.5f), FVector(0.04f, 0.002f, 0.015f), Trim);
	AddBarrelPart(FVector(50.f, 0.f, -1.5f), FVector(0.04f, 0.002f, 0.015f), Trim);
	// Barrel energy rail
	AddPart(FVector(45.f, 1.2f, 0.5f), FVector(0.12f, 0.003f, 0.003f), Accent, CylinderMesh);
	// Scope body (cylinder)
	AddPart(FVector(22.f, 0.f, 4.5f), FVector(0.1f, 0.025f, 0.025f), Dark, CylinderMesh);
	// Scope lens (front) — glowing accent
	AddPart(FVector(27.f, 0.f, 4.5f), FVector(0.003f, 0.02f, 0.02f), Accent, CylinderMesh);
	// Scope lens (rear)
	AddPart(FVector(17.f, 0.f, 4.5f), FVector(0.003f, 0.015f, 0.015f), Accent, CylinderMesh);
	// Scope mount rings
	AddPart(FVector(19.f, 0.f, 3.2f), FVector(0.008f, 0.015f, 0.015f), Trim);
	AddPart(FVector(25.f, 0.f, 3.2f), FVector(0.008f, 0.015f, 0.015f), Trim);
	// Magazine
	AddPart(FVector(15.f, 0.f, -4.f), FVector(0.04f, 0.025f, 0.06f), Dark);
	// Magazine glow window
	AddPart(FVector(15.f, 1.3f, -3.5f), FVector(0.02f, 0.003f, 0.025f), Accent);
	// Grip
	AddPart(FVector(0.f, 0.f, -3.5f), FVector(0.035f, 0.035f, 0.06f), Body);
	// Cheek rest
	AddPart(FVector(-10.f, 0.f, 2.f), FVector(0.06f, 0.03f, 0.015f), Body);
	// Stock — adjustable
	AddPart(FVector(-15.f, 0.f, 0.f), FVector(0.12f, 0.04f, 0.05f), Dark);
	AddPart(FVector(-20.f, 0.f, 0.5f), FVector(0.04f, 0.05f, 0.04f), Trim);
	// Stock accent
	AddPart(FVector(-20.f, 0.f, 2.5f), FVector(0.003f, 0.035f, 0.003f), Accent);
	// Bipod stubs (folded)
	AddPart(FVector(30.f, 2.f, -2.f), FVector(0.005f, 0.005f, 0.04f), Dark);
	AddPart(FVector(30.f, -2.f, -2.f), FVector(0.005f, 0.005f, 0.04f), Dark);
	// Accent energy line — long glowing strip
	AddPart(FVector(25.f, 0.f, 2.f), FVector(0.32f, 0.055f, 0.003f), Accent);
	// Side panel lines
	AddPart(FVector(15.f, 2.6f, 0.f), FVector(0.2f, 0.002f, 0.025f), Dim);
}

void UExoWeaponViewModel::BuildPistolModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.07f, 0.07f, 0.09f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);
	FLinearColor Trim(0.09f, 0.09f, 0.12f);
	FLinearColor Dim(0.055f, 0.055f, 0.065f);

	// Slide
	AddPart(FVector(8.f, 0.f, 1.f), FVector(0.18f, 0.04f, 0.035f), Body);
	// Slide top flat
	AddPart(FVector(8.f, 0.f, 2.8f), FVector(0.16f, 0.038f, 0.004f), Dim);
	// Slide serrations (back)
	AddPart(FVector(0.f, 0.f, 1.f), FVector(0.03f, 0.042f, 0.037f), Trim);
	// Slide serration lines
	AddPart(FVector(-0.5f, 2.15f, 1.f), FVector(0.004f, 0.002f, 0.03f), Dim);
	AddPart(FVector(0.5f, 2.15f, 1.f), FVector(0.004f, 0.002f, 0.03f), Dim);
	// Short barrel
	MuzzleTip = AddBarrelPart(FVector(22.f, 0.f, 1.f), FVector(0.06f, 0.015f, 0.015f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Frame / lower
	AddPart(FVector(6.f, 0.f, -1.f), FVector(0.14f, 0.038f, 0.02f), Dark);
	// Grip (angled)
	AddPart(FVector(2.f, 0.f, -4.f), FVector(0.035f, 0.035f, 0.07f), Dark);
	// Grip texture ridges
	AddPart(FVector(2.f, 1.8f, -4.f), FVector(0.025f, 0.003f, 0.05f), Dim);
	// Trigger guard
	AddPart(FVector(5.f, 0.f, -1.5f), FVector(0.04f, 0.015f, 0.02f), Dark);
	// Accessory rail
	AddPart(FVector(12.f, 0.f, -1.5f), FVector(0.06f, 0.01f, 0.006f), Trim);
	// Accent stripe — glowing
	AddPart(FVector(10.f, 0.f, 2.5f), FVector(0.12f, 0.045f, 0.003f), Accent);
	// Energy window on slide
	AddPart(FVector(12.f, 2.1f, 1.f), FVector(0.04f, 0.003f, 0.015f), Accent);
	// Rear sight
	AddPart(FVector(1.f, 0.f, 2.2f), FVector(0.008f, 0.02f, 0.008f), Trim);
	// Front sight dot
	AddPart(FVector(17.f, 0.f, 2.5f), FVector(0.004f, 0.004f, 0.004f), Accent);
}

void UExoWeaponViewModel::BuildLauncherModel(const FLinearColor& Accent)
{
	FLinearColor Body(0.1f, 0.09f, 0.08f);
	FLinearColor Dark(0.05f, 0.05f, 0.06f);
	FLinearColor Trim(0.12f, 0.11f, 0.1f);
	FLinearColor Dim(0.07f, 0.065f, 0.06f);

	// Thick cylindrical barrel
	MuzzleTip = AddBarrelPart(FVector(30.f, 0.f, 0.f), FVector(0.22f, 0.04f, 0.04f), Dark, CylinderMesh);
	if (MuzzleTip) MuzzleTip->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	// Barrel ring (muzzle end)
	AddBarrelPart(FVector(40.f, 0.f, 0.f), FVector(0.015f, 0.05f, 0.05f), Trim, CylinderMesh);
	// Inner barrel glow ring
	AddPart(FVector(41.f, 0.f, 0.f), FVector(0.003f, 0.03f, 0.03f), Accent, CylinderMesh);
	// Body housing
	AddPart(FVector(10.f, 0.f, 0.f), FVector(0.2f, 0.07f, 0.06f), Body);
	// Body panel lines
	AddPart(FVector(5.f, 3.6f, 0.f), FVector(0.003f, 0.002f, 0.04f), Dim);
	AddPart(FVector(15.f, 3.6f, 0.f), FVector(0.003f, 0.002f, 0.04f), Dim);
	// Drum magazine (cylinder below body)
	AddPart(FVector(12.f, 0.f, -4.f), FVector(0.06f, 0.05f, 0.05f), Dark, CylinderMesh);
	// Drum cap
	AddPart(FVector(12.f, 0.f, -7.f), FVector(0.003f, 0.04f, 0.04f), Trim, CylinderMesh);
	// Drum energy indicator
	AddPart(FVector(12.f, 2.6f, -4.f), FVector(0.003f, 0.003f, 0.03f), Accent, CylinderMesh);
	// Grip
	AddPart(FVector(-2.f, 0.f, -4.f), FVector(0.04f, 0.04f, 0.07f), Body);
	// Front grip
	AddPart(FVector(20.f, 0.f, -3.f), FVector(0.03f, 0.03f, 0.05f), Dark);
	// Warning stripe — hazard orange accent
	AddPart(FVector(18.f, 0.f, 3.5f), FVector(0.15f, 0.075f, 0.004f), Accent);
	// Side panel
	AddPart(FVector(8.f, 3.5f, 0.f), FVector(0.1f, 0.003f, 0.04f), Trim);
	// Targeting laser housing
	AddPart(FVector(22.f, 0.f, 3.f), FVector(0.02f, 0.015f, 0.015f), Dark);
	// Laser lens
	AddPart(FVector(23.5f, 0.f, 3.f), FVector(0.003f, 0.008f, 0.008f), Accent, CylinderMesh);
}

void UExoWeaponViewModel::BuildMeleeModel(const FLinearColor& Accent)
{
	FLinearColor Blade(0.15f, 0.15f, 0.18f);
	FLinearColor Grip(0.06f, 0.06f, 0.07f);
	FLinearColor Dark(0.04f, 0.04f, 0.05f);

	// Main blade
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.25f, 0.005f, 0.04f), Blade);
	// Blade edge — glowing accent
	AddPart(FVector(20.f, 0.f, 2.5f), FVector(0.24f, 0.003f, 0.002f), Accent);
	// Blade energy channel (center groove)
	AddPart(FVector(20.f, 0.f, 0.f), FVector(0.20f, 0.003f, 0.008f), Accent);
	// Blade spine (thicker back edge)
	AddPart(FVector(20.f, 0.f, -2.f), FVector(0.24f, 0.008f, 0.003f), Grip);
	// Blade tip reinforcement
	AddPart(FVector(33.f, 0.f, 1.f), FVector(0.02f, 0.006f, 0.02f), Blade);
	// Cross guard
	AddPart(FVector(5.f, 0.f, 0.f), FVector(0.015f, 0.06f, 0.015f), Grip);
	// Guard accent dots
	AddPart(FVector(5.f, 3.f, 0.f), FVector(0.005f, 0.005f, 0.005f), Accent);
	AddPart(FVector(5.f, -3.f, 0.f), FVector(0.005f, 0.005f, 0.005f), Accent);
	// Guard energy strip
	AddPart(FVector(5.f, 0.f, 0.f), FVector(0.003f, 0.055f, 0.003f), Accent);
	// Grip handle (cylinder)
	AddPart(FVector(-5.f, 0.f, 0.f), FVector(0.08f, 0.025f, 0.025f), Grip, CylinderMesh);
	// Grip wrap ridges
	AddPart(FVector(-3.f, 0.f, 0.f), FVector(0.003f, 0.027f, 0.027f), Dark, CylinderMesh);
	AddPart(FVector(-7.f, 0.f, 0.f), FVector(0.003f, 0.027f, 0.027f), Dark, CylinderMesh);
	// Pommel
	AddPart(FVector(-10.f, 0.f, 0.f), FVector(0.015f, 0.03f, 0.03f), Blade);
	// Pommel accent
	AddPart(FVector(-11.f, 0.f, 0.f), FVector(0.003f, 0.02f, 0.02f), Accent, CylinderMesh);
}
