// ExoProceduralTextures.cpp — Procedural noise texture generation
#include "Visual/ExoProceduralTextures.h"
#include "Engine/Texture2D.h"

// --- Value noise with smoothstep interpolation ---

static float Hash2D(int32 IX, int32 IY)
{
	float N = FMath::Sin((float)IX * 12.9898f + (float)IY * 78.233f) * 43758.5453f;
	return N - FMath::FloorToFloat(N);
}

static float SmoothNoise2D(float X, float Y)
{
	int32 IX = FMath::FloorToInt(X);
	int32 IY = FMath::FloorToInt(Y);
	float FX = X - (float)IX;
	float FY = Y - (float)IY;

	// Smoothstep interpolation (C1 continuous)
	float SX = FX * FX * (3.f - 2.f * FX);
	float SY = FY * FY * (3.f - 2.f * FY);

	float V00 = Hash2D(IX, IY);
	float V10 = Hash2D(IX + 1, IY);
	float V01 = Hash2D(IX, IY + 1);
	float V11 = Hash2D(IX + 1, IY + 1);

	return FMath::Lerp(
		FMath::Lerp(V00, V10, SX),
		FMath::Lerp(V01, V11, SX), SY);
}

static float FBM(float X, float Y, int32 Octaves)
{
	float Value = 0.f;
	float Amp = 0.5f;
	float Freq = 1.f;
	for (int32 i = 0; i < Octaves; i++)
	{
		Value += Amp * SmoothNoise2D(X * Freq, Y * Freq);
		Amp *= 0.5f;
		Freq *= 2.f;
	}
	return Value;
}

/**
 * Creates a noise texture with values in [MinVal, MaxVal] range.
 * The narrow range (e.g. 0.7-1.0) ensures subtle surface variation
 * when multiplied with base color — no harsh boundaries.
 */
static UTexture2D* CreateNoiseTexture(int32 Size, float NoiseScale,
	int32 Octaves, float MinVal, float MaxVal)
{
	UTexture2D* Tex = UTexture2D::CreateTransient(Size, Size, PF_B8G8R8A8);
	if (!Tex) return nullptr;

	Tex->AddToRoot();
	Tex->SRGB = false;
	Tex->Filter = TF_Bilinear;
	Tex->AddressX = TA_Wrap;
	Tex->AddressY = TA_Wrap;

	FTexture2DMipMap& Mip = Tex->GetPlatformData()->Mips[0];
	uint8* Data = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));

	float Range = MaxVal - MinVal;
	for (int32 Y = 0; Y < Size; Y++)
	{
		for (int32 X = 0; X < Size; X++)
		{
			float NX = (float)X / (float)Size * NoiseScale;
			float NY = (float)Y / (float)Size * NoiseScale;
			float N = FBM(NX, NY, Octaves);

			float Val = FMath::Clamp(MinVal + N * Range, 0.f, 1.f);
			uint8 Byte = (uint8)(Val * 255.f);

			int32 Idx = (Y * Size + X) * 4;
			Data[Idx + 0] = Byte; // B
			Data[Idx + 1] = Byte; // G
			Data[Idx + 2] = Byte; // R
			Data[Idx + 3] = 255;  // A
		}
	}

	Mip.BulkData.Unlock();
	Tex->UpdateResource();
	return Tex;
}

/**
 * Creates a tangent-space normal map from FBM height data.
 * Height differences → surface normals → packed to (0.5 + N*0.5) * 255.
 * Strength controls the bump intensity (higher = more pronounced relief).
 */
static UTexture2D* CreateNormalTexture(int32 Size, float NoiseScale,
	int32 Octaves, float Strength)
{
	UTexture2D* Tex = UTexture2D::CreateTransient(Size, Size, PF_B8G8R8A8);
	if (!Tex) return nullptr;

	Tex->AddToRoot();
	Tex->SRGB = false;
	Tex->Filter = TF_Bilinear;
	Tex->AddressX = TA_Wrap;
	Tex->AddressY = TA_Wrap;
	Tex->CompressionSettings = TC_Normalmap;

	FTexture2DMipMap& Mip = Tex->GetPlatformData()->Mips[0];
	uint8* Data = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));

	// Pre-compute height field for central-difference normals
	float InvSize = NoiseScale / (float)Size;
	for (int32 Y = 0; Y < Size; Y++)
	{
		for (int32 X = 0; X < Size; X++)
		{
			float NX = (float)X * InvSize;
			float NY = (float)Y * InvSize;
			float Step = InvSize;

			// Central differences for height gradient
			float HL = FBM(NX - Step, NY, Octaves);
			float HR = FBM(NX + Step, NY, Octaves);
			float HD = FBM(NX, NY - Step, Octaves);
			float HU = FBM(NX, NY + Step, Octaves);

			FVector Normal(
				(HL - HR) * Strength,
				(HD - HU) * Strength,
				1.f);
			Normal.Normalize();

			// Pack from [-1,1] → [0,255] (UE5 BGRA format)
			int32 Idx = (Y * Size + X) * 4;
			Data[Idx + 0] = (uint8)((Normal.Z * 0.5f + 0.5f) * 255.f); // B = Z
			Data[Idx + 1] = (uint8)((Normal.Y * 0.5f + 0.5f) * 255.f); // G = Y
			Data[Idx + 2] = (uint8)((Normal.X * 0.5f + 0.5f) * 255.f); // R = X
			Data[Idx + 3] = 255;
		}
	}

	Mip.BulkData.Unlock();
	Tex->UpdateResource();
	return Tex;
}

static UTexture2D* GroundNoiseCache = nullptr;
static UTexture2D* MetalNoiseCache = nullptr;
static UTexture2D* GroundNormalCache = nullptr;
static UTexture2D* MetalNormalCache = nullptr;

UTexture2D* FExoProceduralTextures::GetGroundNoise()
{
	if (GroundNoiseCache) return GroundNoiseCache;
	GroundNoiseCache = CreateNoiseTexture(256, 6.f, 4, 0.70f, 1.0f);
	return GroundNoiseCache;
}

UTexture2D* FExoProceduralTextures::GetMetalNoise()
{
	if (MetalNoiseCache) return MetalNoiseCache;
	MetalNoiseCache = CreateNoiseTexture(256, 12.f, 5, 0.78f, 1.0f);
	return MetalNoiseCache;
}

UTexture2D* FExoProceduralTextures::GetGroundNormal()
{
	if (GroundNormalCache) return GroundNormalCache;
	// Broad bumps: 4 noise periods, 4 octaves, strong relief
	GroundNormalCache = CreateNormalTexture(256, 4.f, 4, 3.f);
	return GroundNormalCache;
}

UTexture2D* FExoProceduralTextures::GetMetalNormal()
{
	if (MetalNormalCache) return MetalNormalCache;
	// Fine detail: 10 noise periods, 5 octaves, subtle relief
	MetalNormalCache = CreateNormalTexture(256, 10.f, 5, 1.5f);
	return MetalNormalCache;
}
