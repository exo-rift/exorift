// ExoLevelBuilderStorytelling.cpp — Environmental storytelling props
// Small details implying events happened before the match: abandoned camps,
// battle damage, wrecked vehicles, science equipment, warning barriers.
#include "Map/ExoLevelBuilder.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Visual/ExoMaterialFactory.h"
#include "ExoRift.h"

void AExoLevelBuilder::BuildStorytelling()
{
	// ================================================================
	// 1. ABANDONED CAMPSITES — overturned containers, supply crates,
	//    dead campfires with ember glow
	// ================================================================
	auto SpawnCampsite = [&](const FVector& Center, float Yaw)
	{
		FRotator Facing(0.f, Yaw, 0.f);
		FLinearColor CrateCol(0.07f, 0.06f, 0.05f);
		FLinearColor MetalCol(0.05f, 0.05f, 0.06f);

		// Overturned container — tilted on its side
		SpawnStaticMesh(Center + Facing.RotateVector(FVector(200.f, -150.f, 60.f)),
			FVector(3.f, 1.5f, 1.2f),
			Facing + FRotator(0.f, 0.f, 75.f), CubeMesh, CrateCol);

		// Scattered supply crates (3 small boxes)
		SpawnStaticMesh(Center + Facing.RotateVector(FVector(-180.f, 100.f, 30.f)),
			FVector(0.8f, 0.6f, 0.6f),
			Facing + FRotator(0.f, 15.f, 0.f), CubeMesh, MetalCol);
		SpawnStaticMesh(Center + Facing.RotateVector(FVector(-80.f, 200.f, 25.f)),
			FVector(0.7f, 0.5f, 0.5f),
			Facing + FRotator(5.f, -20.f, 8.f), CubeMesh, CrateCol);
		SpawnStaticMesh(Center + Facing.RotateVector(FVector(50.f, 250.f, 20.f)),
			FVector(0.6f, 0.6f, 0.4f),
			Facing + FRotator(-3.f, 40.f, 0.f), CubeMesh, MetalCol);

		// Dead campfire — dark charred cylinder on ground
		SpawnStaticMesh(Center + FVector(0.f, 0.f, 10.f),
			FVector(1.2f, 1.2f, 0.15f), FRotator::ZeroRotator,
			CylinderMesh, FLinearColor(0.02f, 0.018f, 0.015f));

		// Ember glow sphere — faint orange emissive
		UStaticMeshComponent* Ember = SpawnStaticMesh(
			Center + FVector(0.f, 0.f, 18.f),
			FVector(0.5f, 0.5f, 0.25f), FRotator::ZeroRotator,
			SphereMesh, FLinearColor(0.8f, 0.3f, 0.05f));
		if (Ember)
		{
			UMaterialInterface* EmMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* EM = UMaterialInstanceDynamic::Create(EmMat, this);
			if (!EM) { return; }
			EM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(4.f, 1.2f, 0.2f));
			Ember->SetMaterial(0, EM);
		}

		// Upturned stool — small cylinder on its side
		SpawnStaticMesh(Center + Facing.RotateVector(FVector(150.f, 180.f, 20.f)),
			FVector(0.4f, 0.4f, 0.6f),
			Facing + FRotator(85.f, 0.f, 0.f), CylinderMesh, MetalCol);
	};

	// Place campsites near compounds
	SpawnCampsite(FVector(800.f, 15000.f, GroundZ), 30.f);     // North compound outskirts
	SpawnCampsite(FVector(-14800.f, 1000.f, GroundZ), -45.f);   // West compound
	SpawnCampsite(FVector(14400.f, -1200.f, GroundZ), 110.f);   // East compound
	SpawnCampsite(FVector(-7000.f, -14400.f, GroundZ), 200.f); // South approach
	SpawnCampsite(FVector(2400.f, 2400.f, GroundZ), 0.f);     // Near hub

	// ================================================================
	// 2. BATTLE DAMAGE MARKINGS — bullet holes, cracked floor panels
	// ================================================================
	auto SpawnBulletHoleCluster = [&](const FVector& WallPos, float WallYaw, int32 Count)
	{
		// Small dark cylinders embedded in wall surface
		FRotator WallRot(0.f, WallYaw, 0.f);
		FLinearColor HoleCol(0.015f, 0.012f, 0.01f);
		for (int32 i = 0; i < Count; i++)
		{
			float OffX = FMath::RandRange(-80.f, 80.f);
			float OffZ = FMath::RandRange(-60.f, 60.f);
			FVector Offset = WallRot.RotateVector(FVector(0.f, OffX, 0.f))
				+ FVector(0.f, 0.f, OffZ);
			SpawnStaticMesh(WallPos + Offset,
				FVector(0.06f, 0.06f, 0.04f),
				FRotator(90.f, WallYaw, 0.f), CylinderMesh, HoleCol);
		}
	};

	auto SpawnCrackedFloor = [&](const FVector& Pos, float Yaw)
	{
		FRotator R(0.f, Yaw, 0.f);
		FLinearColor FloorCol(0.035f, 0.035f, 0.04f);
		// Cracked panel — main slab slightly raised
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 3.f),
			FVector(3.f, 3.f, 0.06f), R, CubeMesh, FloorCol);
		// Crack lines — thin dark strips across the panel
		SpawnStaticMesh(Pos + R.RotateVector(FVector(-40.f, 0.f, 5.f)),
			FVector(3.2f, 0.04f, 0.02f),
			R + FRotator(0.f, 12.f, 0.f), CubeMesh,
			FLinearColor(0.01f, 0.01f, 0.012f));
		SpawnStaticMesh(Pos + R.RotateVector(FVector(30.f, 20.f, 5.f)),
			FVector(2.f, 0.03f, 0.02f),
			R + FRotator(0.f, -25.f, 0.f), CubeMesh,
			FLinearColor(0.01f, 0.01f, 0.012f));
	};

	// Bullet holes on building walls near compounds
	SpawnBulletHoleCluster(FVector(400.f, 15600.f, 250.f), 0.f, 6);
	SpawnBulletHoleCluster(FVector(15600.f, 400.f, 300.f), 90.f, 5);
	SpawnBulletHoleCluster(FVector(-15600.f, -600.f, 200.f), 180.f, 7);
	SpawnBulletHoleCluster(FVector(-400.f, -15600.f, 280.f), 270.f, 4);

	// Cracked floor panels
	SpawnCrackedFloor(FVector(1000.f, 16400.f, GroundZ), 10.f);
	SpawnCrackedFloor(FVector(16600.f, -600.f, GroundZ), 45.f);
	SpawnCrackedFloor(FVector(-16400.f, 800.f, GroundZ), -15.f);
	SpawnCrackedFloor(FVector(-200.f, 400.f, GroundZ), 0.f);

	// ================================================================
	// 3. ABANDONED VEHICLES — wrecked hover vehicles
	// ================================================================
	auto SpawnWreckedHover = [&](const FVector& Pos, float Yaw, float Tilt)
	{
		FRotator R(Tilt, Yaw, FMath::RandRange(-3.f, 3.f));
		FLinearColor HullCol(0.045f, 0.05f, 0.055f);
		FLinearColor DarkCol(0.03f, 0.03f, 0.035f);

		// Main hull — rectangular body tilted
		SpawnStaticMesh(Pos + FVector(0.f, 0.f, 100.f),
			FVector(6.f, 3.f, 1.5f), R, CubeMesh, HullCol);

		// Broken left engine pod — detached, on ground
		SpawnStaticMesh(Pos + R.RotateVector(FVector(-200.f, 250.f, 30.f)),
			FVector(1.f, 1.f, 2.f),
			FRotator(FMath::RandRange(30.f, 60.f), Yaw + 20.f, 10.f),
			CylinderMesh, DarkCol);

		// Right engine pod — still attached but dangling
		SpawnStaticMesh(Pos + R.RotateVector(FVector(-200.f, -180.f, 60.f)),
			FVector(1.f, 1.f, 2.5f),
			R + FRotator(15.f, 0.f, -25.f), CylinderMesh, DarkCol);

		// Canopy — cracked windshield (dark sphere cap)
		SpawnStaticMesh(Pos + R.RotateVector(FVector(250.f, 0.f, 160.f)),
			FVector(2.f, 2.f, 0.8f), R, SphereMesh,
			FLinearColor(0.02f, 0.025f, 0.035f));
	};

	SpawnWreckedHover(FVector(-1200.f, 14000.f, GroundZ), 55.f, 8.f);
	SpawnWreckedHover(FVector(15000.f, 1600.f, GroundZ), -30.f, -6.f);
	SpawnWreckedHover(FVector(-8400.f, -15000.f, GroundZ), 160.f, 10.f);
	SpawnWreckedHover(FVector(1600.f, -1000.f, GroundZ), 90.f, -4.f);

	// ================================================================
	// 4. SCIENCE EQUIPMENT — overturned lab tables, broken monitors,
	//    scattered data pads
	// ================================================================
	auto SpawnLabSite = [&](const FVector& Center, float Yaw)
	{
		FRotator Facing(0.f, Yaw, 0.f);
		FLinearColor TableCol(0.08f, 0.08f, 0.09f);

		// Overturned lab table — flat cube on its side
		SpawnStaticMesh(Center + Facing.RotateVector(FVector(0.f, 0.f, 40.f)),
			FVector(4.f, 2.f, 0.15f),
			Facing + FRotator(0.f, 0.f, 85.f), CubeMesh, TableCol);

		// Broken monitor — cube with emissive flickering screen
		FVector MonPos = Center + Facing.RotateVector(FVector(180.f, 100.f, 35.f));
		SpawnStaticMesh(MonPos,
			FVector(0.6f, 0.8f, 0.5f),
			Facing + FRotator(12.f, -10.f, 0.f), CubeMesh,
			FLinearColor(0.04f, 0.04f, 0.045f));
		// Screen face — thin emissive panel
		UStaticMeshComponent* Screen = SpawnStaticMesh(
			MonPos + Facing.RotateVector(FVector(32.f, 0.f, 5.f)),
			FVector(0.02f, 0.7f, 0.4f),
			Facing + FRotator(12.f, -10.f, 0.f), CubeMesh,
			FLinearColor(0.1f, 0.3f, 0.5f));
		if (Screen)
		{
			UMaterialInterface* ScMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* SM = UMaterialInstanceDynamic::Create(ScMat, this);
			if (!SM) { return; }
			SM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(0.5f, 1.5f, 2.5f));
			Screen->SetMaterial(0, SM);
		}

		// Scattered data pads — small thin cubes
		for (int32 i = 0; i < 3; i++)
		{
			float Ox = FMath::RandRange(-150.f, 150.f);
			float Oy = FMath::RandRange(-100.f, 100.f);
			SpawnStaticMesh(
				Center + Facing.RotateVector(FVector(Ox, Oy, 5.f)),
				FVector(0.25f, 0.15f, 0.02f),
				FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f),
				CubeMesh, FLinearColor(0.06f, 0.065f, 0.07f));
		}

		// Toppled equipment rack — tall thin cube fallen
		SpawnStaticMesh(
			Center + Facing.RotateVector(FVector(-200.f, -80.f, 50.f)),
			FVector(0.4f, 1.5f, 3.f),
			Facing + FRotator(0.f, 0.f, 70.f), CubeMesh,
			FLinearColor(0.06f, 0.06f, 0.07f));
	};

	SpawnLabSite(FVector(600.f, 16600.f, GroundZ), 0.f);     // North compound interior
	SpawnLabSite(FVector(16800.f, -400.f, GroundZ), 90.f);   // East compound
	SpawnLabSite(FVector(-16600.f, 600.f, GroundZ), -90.f);  // West compound
	SpawnLabSite(FVector(400.f, -600.f, GroundZ), 45.f);    // Central hub

	// ================================================================
	// 5. WARNING BARRIERS — yellow/black striped barriers near hazards,
	//    knocked-over posts
	// ================================================================
	auto SpawnWarningBarrier = [&](const FVector& Pos, float Yaw, bool bKnockedOver)
	{
		FRotator R(0.f, Yaw, 0.f);
		FLinearColor BarCol(0.08f, 0.08f, 0.02f);       // Dark yellow
		FLinearColor StripeCol(0.02f, 0.02f, 0.02f);    // Black stripe

		if (bKnockedOver)
		{
			// Barrier rail fallen on ground
			SpawnStaticMesh(Pos + FVector(0.f, 0.f, 15.f),
				FVector(5.f, 0.3f, 0.15f),
				R + FRotator(0.f, 0.f, 5.f), CubeMesh, BarCol);
			// Stripe on barrier
			SpawnStaticMesh(Pos + R.RotateVector(FVector(0.f, 16.f, 17.f)),
				FVector(4.5f, 0.04f, 0.1f),
				R + FRotator(0.f, 0.f, 5.f), CubeMesh, StripeCol);
			// Fallen post
			SpawnStaticMesh(Pos + R.RotateVector(FVector(-250.f, 40.f, 10.f)),
				FVector(0.2f, 0.2f, 1.5f),
				R + FRotator(80.f, 0.f, 0.f), CylinderMesh,
				FLinearColor(0.06f, 0.06f, 0.065f));
		}
		else
		{
			// Standing barrier posts
			for (float Offset : {-200.f, 200.f})
			{
				SpawnStaticMesh(
					Pos + R.RotateVector(FVector(Offset, 0.f, 50.f)),
					FVector(0.2f, 0.2f, 1.f), R,
					CylinderMesh, FLinearColor(0.06f, 0.06f, 0.065f));
			}
			// Horizontal barrier rail
			SpawnStaticMesh(Pos + FVector(0.f, 0.f, 90.f),
				FVector(5.f, 0.3f, 0.15f), R, CubeMesh, BarCol);
			// Black warning stripe
			SpawnStaticMesh(Pos + R.RotateVector(FVector(0.f, 16.f, 92.f)),
				FVector(4.5f, 0.04f, 0.1f), R, CubeMesh, StripeCol);
		}

		// Emissive warning light on top of one post
		UStaticMeshComponent* WarnLight = SpawnStaticMesh(
			Pos + R.RotateVector(FVector(bKnockedOver ? -250.f : 200.f, 0.f,
				bKnockedOver ? 20.f : 110.f)),
			FVector(0.15f, 0.15f, 0.1f), FRotator::ZeroRotator,
			SphereMesh, FLinearColor(0.8f, 0.5f, 0.02f));
		if (WarnLight)
		{
			UMaterialInterface* WMat = FExoMaterialFactory::GetEmissiveOpaque();
			UMaterialInstanceDynamic* WM = UMaterialInstanceDynamic::Create(WMat, this);
			if (!WM) { return; }
			WM->SetVectorParameterValue(TEXT("EmissiveColor"),
				FLinearColor(6.f, 3.f, 0.3f));
			WarnLight->SetMaterial(0, WM);
		}
	};

	// Standing barriers near hazard zones
	SpawnWarningBarrier(FVector(-1200.f, 16400.f, GroundZ), 0.f, false);
	SpawnWarningBarrier(FVector(16400.f, 1200.f, GroundZ), 90.f, false);
	SpawnWarningBarrier(FVector(-16400.f, -1200.f, GroundZ), 180.f, false);

	// Knocked-over barriers — signs of recent conflict
	SpawnWarningBarrier(FVector(1200.f, 15600.f, GroundZ), 20.f, true);
	SpawnWarningBarrier(FVector(15600.f, -1200.f, GroundZ), 75.f, true);
	SpawnWarningBarrier(FVector(-7600.f, -15600.f, GroundZ), 150.f, true);
	SpawnWarningBarrier(FVector(600.f, 1000.f, GroundZ), -10.f, true);

	UE_LOG(LogExoRift, Log,
		TEXT("LevelBuilder: Environmental storytelling props placed"));
}
