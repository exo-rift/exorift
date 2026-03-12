# ExoRift Visual Style Bible

> Spot Cloud b.v. (2026) — ExoRift Battle Royale
> Engine: UE5.7 | Renderer: Lumen HW-RT, VSM 4096 pages, TSR, Nanite
> Theme: Realistic sci-fi frontier — worn, functional, lived-in

---

## Table of Contents

1. [Mood & References](#1-mood--references)
2. [Color Palette](#2-color-palette)
3. [PBR Material Tiers](#3-pbr-material-tiers)
4. [Emissive Budget](#4-emissive-budget)
5. [Lighting Specification](#5-lighting-specification)
6. [Post-Processing](#6-post-processing)
7. [Atmosphere & Fog](#7-atmosphere--fog)
8. [Surface Detail & Weathering](#8-surface-detail--weathering)
9. [Forbidden List](#9-forbidden-list)
10. [Asset Type Style Notes](#10-asset-type-style-notes)

---

## 1. Mood & References

### Core visual identity

ExoRift takes place on a harsh, sun-baked alien frontier. The environment is a desert-industrial hybrid: dusty open terrain punctuated by worn military-industrial compounds, crumbling ruins, and functional sci-fi infrastructure. Everything looks like it was built to last, then left to weather.

### Reference touchstones

| Reference                         | What to take from it                               |
|-----------------------------------|-----------------------------------------------------|
| **Destiny 2** — Cosmodrome, EDZ   | Scale of open terrain, weathered concrete, warm light |
| **Titanfall 2** — industrial maps  | Functional sci-fi architecture, panel detail, grime  |
| **Apex Legends** — Kings Canyon    | Desert palette, warm tones, readable silhouettes     |
| **Halo Reach** — Forge World       | Military outpost feel, muted naturalism              |
| **Star Citizen** — ground outposts | Believable sci-fi infrastructure, PBR quality        |

### Mood keywords

Dusty. Functional. Worn. Sun-bleached. Teal accents on warm metal. Readable silhouettes. Nothing pristine, nothing ruined — everything in between.

---

## 2. Color Palette

### Primary palette (environment)

| Name               | Hex       | sRGB                | Use                                |
|--------------------|-----------|---------------------|------------------------------------|
| Warm Sand          | `#8C7A5E` | (140, 122, 94)      | Terrain base, ground fill          |
| Desert Dust        | `#A69072` | (166, 144, 114)     | Lighter ground, path surfaces      |
| Scorched Earth     | `#5A4A38` | (90, 74, 56)        | Dark terrain, shadows in sand      |
| Concrete Gray      | `#6E6960` | (110, 105, 96)      | Building walls, floor slabs        |
| Structural Steel   | `#505860` | (80, 88, 96)        | Metal panels, structural beams     |
| Dark Panel         | `#2A2E33` | (42, 46, 51)        | Dark metal, interior panels        |
| Oxidized Metal     | `#7A6850` | (122, 104, 80)      | Weathered metal, rust tones        |

### Accent palette (functional color)

| Name               | Hex       | sRGB                | Use                                |
|--------------------|-----------|---------------------|------------------------------------|
| Cyan/Teal          | `#00D4FF` | (0, 212, 255)       | Primary accent, UI, friendly indicators |
| Amber Warning      | `#FFB347` | (255, 179, 71)      | Warm accents, caution indicators   |
| Hot Orange         | `#FF6B35` | (255, 107, 53)      | Danger, explosions, heat glow      |
| Alert Red          | `#FF3344` | (255, 51, 68)       | Enemy, damage, critical warnings   |
| Energy Blue        | `#3399FF` | (51, 153, 255)      | Shield, energy, friendly tech      |
| Toxic Green        | `#44FF66` | (68, 255, 102)      | Healing, revive, safe zones        |
| Deep Space Blue    | `#0A1628` | (10, 22, 40)        | Sky zenith, deep shadows           |
| Void Purple        | `#6633CC` | (102, 51, 204)      | Legendary rarity, special FX       |

### Rarity colors (weapons & loot)

| Rarity     | Hex       | sRGB                |
|------------|-----------|---------------------|
| Common     | `#B0B0B0` | (176, 176, 176)     |
| Rare       | `#3399FF` | (51, 153, 255)      |
| Epic       | `#9933FF` | (153, 51, 255)      |
| Legendary  | `#FFB347` | (255, 179, 71)      |

### Temperature rule

- **Warm** tones dominate the environment (sand, rust, amber sun)
- **Cool** tones are reserved for technology (cyan emissives, blue shields, teal UI)
- This warm/cool contrast is the core of ExoRift's visual identity — never violate it

---

## 3. PBR Material Tiers

Three strict tiers. Every surface in the game falls into one of these.

### Tier 1: Terrain (natural surfaces)

| Property  | Range          | Notes                              |
|-----------|----------------|------------------------------------|
| Metallic  | 0.00 - 0.02    | Non-metallic (dielectric)          |
| Roughness | 0.82 - 0.94    | Very rough, matte                  |
| Examples  | Dirt, sand, rock, concrete ground, dried mud |

Base color range: linear 0.08-0.16 (warm sandy tones, NOT cold gray).
Ground albedo for Lumen GI: `FColor(55, 50, 42)` — warm bounce light.

### Tier 2: Structural (built surfaces)

| Property  | Range          | Notes                              |
|-----------|----------------|------------------------------------|
| Metallic  | 0.35 - 0.60    | Partial metal (painted/coated)     |
| Roughness | 0.40 - 0.60    | Medium roughness, satin-like       |
| Examples  | Walls, floors, panels, doors, ceilings, platforms |

Base color range:
- Walls: linear 0.14-0.20
- Trim/frames: linear 0.22-0.30
- Road surface: linear 0.09-0.10

### Tier 3: Accent (pure metal / functional)

| Property  | Range          | Notes                              |
|-----------|----------------|------------------------------------|
| Metallic  | 0.70 - 0.85    | High metallic                      |
| Roughness | 0.20 - 0.30    | Smoother, more reflective          |
| Examples  | Pipes, vents, handles, weapons, trim strips, bolts |

Accent metals should show color variation — not uniform silver. Use warm tones (oxidized brass, gunmetal, bronzed steel).

### Quick validation chart

```
Surface feels like dirt/rock?     → Tier 1 (M:0, R:0.9)
Surface feels like a wall/floor?  → Tier 2 (M:0.5, R:0.5)
Surface feels like bare metal?    → Tier 3 (M:0.8, R:0.25)
```

---

## 4. Emissive Budget

### Rules

| Context               | Max Emissive Multiplier | Notes                          |
|-----------------------|-------------------------|--------------------------------|
| Persistent scene      | 3.0x                    | Panel lights, indicators, strips |
| VFX momentary (<1s)   | 50-250x                 | Muzzle flash, impacts, sparks  |
| VFX sustained (<5s)   | 5-50x                   | Tracers, projectiles, ribbons  |
| UI/HUD glow           | No limit                | Screen-space, no scene impact  |

### Emissive color rules

- Primary emissive: Cyan/Teal `#00D4FF` — most common
- Secondary emissive: Amber `#FFB347` — warning indicators, some accent lights
- Danger emissive: Hot Orange `#FF6B35` or Red `#FF3344` — reactor cores, hazards
- **Never use pure white (`#FFFFFF`)** as emissive — always tint toward cyan, amber, or a warm off-white

### Emissive area budget

On any single mesh, emissive surfaces should cover **less than 10%** of the total surface area. Emissive is for small accent lights, indicator strips, and display panels — not large glowing surfaces.

Exception: holographic displays and force field effects can cover larger areas but must use translucent additive blending, not opaque emissive.

---

## 5. Lighting Specification

### Sun (Directional Light)

| Property          | Value                    |
|-------------------|--------------------------|
| Intensity         | 12 lux                   |
| Color             | (1.0, 0.9, 0.75) — warm |
| Cast Shadows      | ON (VSM)                 |
| Source Angle       | 0.5 (soft shadow edge)  |

### Sky Light

| Property          | Value                    |
|-------------------|--------------------------|
| Intensity         | 4.5                      |
| Real Time Capture | ON                       |
| Source Type        | SLS Captured Scene       |

### Sky Atmosphere

| Property          | Value                    |
|-------------------|--------------------------|
| Rayleigh Scattering | (0.058, 0.1, 0.22) — reduced blue, warmer scatter |
| Mie Scattering    | Default                  |
| Absorption         | Default                 |

### Auto Exposure

| Property          | Value                    |
|-------------------|--------------------------|
| Exposure Bias     | 1.2                      |
| Min Brightness    | 0.1                      |
| Max Brightness    | 6.0                      |
| Method            | Auto Exposure Histogram  |

### Key lighting principles

1. **Single dominant light source** — the sun. No fill lights except for very specific dark interiors
2. **Lumen GI handles bounce** — do not add manual bounce lights
3. **6 compound approach lights total** — subtle, functional (not decorative)
4. **Contact shadows ON** for grounding objects to surfaces
5. **AO method 1** (SSAO) for additional depth in crevices

---

## 6. Post-Processing

### Bloom

| Property          | Value                    |
|-------------------|--------------------------|
| Method            | BM_SOG (Sum of Gaussians)|
| Intensity         | 0.8                      |
| Threshold         | 1.0                      |

Bloom makes emissives read as "glowing" without washing out the scene. Threshold at 1.0 means only HDR-bright surfaces bloom.

### Vignette

| Property          | Value                    |
|-------------------|--------------------------|
| Intensity         | 0.25                     |

Subtle darkening at screen edges. Adds focus and cinematic feel.

### Film Grain

| Property          | Value                    |
|-------------------|--------------------------|
| Intensity         | 0.015                    |

Very subtle grain to break up banding in gradients and add grit.

### Motion Blur

| Property          | Value                    |
|-------------------|--------------------------|
| Sprint max        | 0.3                      |
| Amount            | Dynamic (speed-scaled)   |

Only active during sprint. Not used for camera rotation.

### Chromatic Aberration

Applied dynamically during sprint and damage — not persistent.

### Color Grading

No aggressive LUT. Minor adjustments:
- Slight warm shift in shadows
- Slight cool shift in highlights
- Contrast: default (1.0)
- Saturation: default (1.0)

---

## 7. Atmosphere & Fog

### Exponential Height Fog (dual-layer)

| Layer              | Density    | Height Falloff | Color              |
|--------------------|------------|----------------|--------------------|
| Atmospheric        | 0.0004     | Default        | Reduced blue tint  |
| Ground haze        | 0.001      | Low falloff    | Warm dust color    |

### Volumetric Fog

| Property          | Value                    |
|-------------------|--------------------------|
| Enabled           | ON                       |
| Extinction Scale  | 0.5                      |
| Scattering        | Default                  |

Volumetric fog adds god rays through structural gaps and depth in interiors.

### Atmospheric rules

1. Fog is warm-tinted, never cold blue
2. Ground haze is densest in low-lying areas (valleys, trenches)
3. Distant mountains/terrain should fade to a warm haze, not a sharp silhouette
4. Fog never completely obscures gameplay-relevant objects at combat distances (<300m)

---

## 8. Surface Detail & Weathering

### Procedural noise (runtime, from ExoProceduralTextures)

| Texture           | Use                       | Tiling      | Strength |
|-------------------|---------------------------|-------------|----------|
| Ground Noise      | Terrain color variation    | World UV    | 3.0      |
| Metal Noise       | Structural weathering      | World UV    | 1.5      |
| Ground Normal     | Terrain bump              | World UV    | 3.0      |
| Metal Normal      | Panel line detail         | World UV    | 1.5      |

### Weathering guidelines for Substance Painter

Every structural surface should show:
1. **Edge wear** — Metallic edges showing through paint (use curvature-based generators)
2. **Dirt accumulation** — Darker values in crevices and low areas (use AO-based generators)
3. **Dust deposits** — Lighter values on top-facing surfaces (use position-based with world Y)
4. **Scratch marks** — Subtle directional scratches on high-traffic surfaces
5. **Panel line darkening** — Slightly darker values along seams and recesses

### Weathering intensity by location

| Location          | Weathering Level | Notes                         |
|-------------------|------------------|-------------------------------|
| Exterior exposed  | Heavy (80-100%)  | Full sun, sand, wind           |
| Exterior sheltered| Medium (50-80%)  | Under overhangs, between walls |
| Interior          | Light (20-50%)   | Protected, cleaner             |
| New/active tech   | Minimal (0-20%)  | Recently deployed, maintained  |

### Normal map guidelines

- Authored normals (from Substance Painter) for unique detail: panel lines, rivets, vents
- Runtime procedural normals (from ExoProceduralTextures) for generic variation
- Both stack in UE5 via detail normal blending
- **Normal strength for terrain:** broad, 3.0
- **Normal strength for metal:** fine, 1.5

---

## 9. Forbidden List

These are explicit art direction rules. Violating these breaks the visual identity.

### Materials

- **No fully reflective chrome** (Metallic=1.0, Roughness=0.0). Nothing in the scene should be a perfect mirror. Minimum roughness for the shiniest metal is 0.20.
- **No pure white emissives** (#FFFFFF). All emissive must be tinted — cyan, amber, warm white, never pure neutral.
- **No flat-lit surfaces**. Every surface must have roughness variation, normal detail, or color variation. Uniform flat values read as placeholder.
- **No pure black** (BaseColor = 0,0,0). Darkest allowed base color: linear 0.02. True black creates unrealistic energy-absorbing surfaces.
- **No "plastic" look** (Metallic=0, Roughness=0.3-0.5 with saturated base color). If it is not metal, roughness should be higher (>0.6). If it is metal, metallic should be higher (>0.3).

### Lighting

- **No colored point lights as fill**. Scene lighting comes from the sun + Lumen GI. The only colored lights are emissive surfaces and the 6 compound approach lights.
- **No light flickering faster than 2Hz**. Rapid strobing is not cinematic, it is annoying.
- **No spotlights as fake GI**. Lumen handles GI. Do not approximate bounce with placed lights.

### Environment

- **No pristine surfaces**. Everything has age, wear, dust. Even "new" technology has transit damage and installation scuffs.
- **No uniform tiling**. If a texture tiles visibly, break it with vertex color blending, decals, or macro variation.
- **No floating objects**. Everything must be grounded — contact shadows and proper placement. No objects hovering 1cm above the floor.

### VFX

- **No particle systems larger than 50m radius** unless they are atmosphere (fog, dust). Combat VFX should be tight and readable.
- **No fully opaque particle sprites**. Use additive or alpha blending for all VFX particles.
- **No emissive multipliers above 3x for persistent scene objects**. Only momentary VFX can exceed this budget.

---

## 10. Asset Type Style Notes

### Weapons

- Functional, angular, utilitarian design
- Dark gunmetal body (Tier 3 accent metal: M:0.8, R:0.25)
- Single cyan or amber emissive accent strip per weapon
- Rarity indicated by emissive color/intensity of the accent, not by mesh changes
- Heat glow: 4-tier color ramp from cool to hot (dark → amber → orange → bright white-amber)

### Buildings/Structures

- Modular panels, visible seams, recessed wall detail
- Parapet roofs, window frames with sills and mullions
- Walls: Tier 2 structural (M:0.5, R:0.5) with Tier 3 trim
- Floors: slightly darker than walls, more roughness
- Interior lighting from small emissive ceiling strips (cyan tint)

### Vehicles

- Hover vehicle: sleek but battered, panel lines, thruster glow (amber)
- Metallic body (Tier 3) with painted sections (Tier 2)
- Underside thruster emissive: amber/orange, 2-3x multiplier max
- Dust/dirt on lower surfaces, cleaner on top

### Terrain/Rocks

- Warm sandy base (Tier 1: M:0, R:0.9)
- Color range: `#8C7A5E` to `#5A4A38`
- Rocks should have subtle color variation — not uniform
- Use procedural noise for breakup

### Props (crates, barrels, consoles)

- Functional, labeled (stenciled text, warning stripes)
- Crates: Tier 2 structural body, Tier 3 metal edges/latches
- Consoles: dark panel body with small emissive displays
- Barrels: cylindrical, with wear marks and label remnants

---

## Quick Reference Card

```
PALETTE:    Warm sand + concrete + gunmetal + cyan accents
PBR:        Terrain M:0 R:0.9 | Structural M:0.5 R:0.5 | Accent M:0.8 R:0.25
EMISSIVE:   Scene ≤3x, VFX momentary up to 250x, never pure white
SUN:        12 lux, warm (1.0, 0.9, 0.75)
SKY:        4.5 intensity, reduced Rayleigh blue
BLOOM:      0.8 intensity, 1.0 threshold
FOG:        Dual-layer warm, volumetric extinction 0.5
EXPOSURE:   Bias 1.2, range 0.1-6.0
FORBIDDEN:  No chrome mirrors, no pure white emissive, no flat surfaces
REFERENCE:  Destiny 2 frontier + Titanfall 2 industrial + Apex desert
```
