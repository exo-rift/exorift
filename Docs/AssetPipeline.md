# ExoRift Asset Pipeline — Blender / Substance Painter / UE5.7

> Spot Cloud b.v. (2026) — ExoRift Battle Royale
> Engine: UE5.7 | Renderer: Lumen HW-RT, VSM, TSR, Nanite
> Style: Realistic sci-fi (see StyleBible.md)

---

## Table of Contents

1. [Naming Conventions](#1-naming-conventions)
2. [Folder Structure](#2-folder-structure)
3. [Performance Budgets](#3-performance-budgets)
4. [Blender Modeling Guidelines](#4-blender-modeling-guidelines)
5. [FBX Export from Blender](#5-fbx-export-from-blender)
6. [Substance Painter Texturing](#6-substance-painter-texturing)
7. [UE5 Import Settings](#7-ue5-import-settings)
8. [Material Setup in UE5](#8-material-setup-in-ue5)
9. [Nanite & LOD Notes](#9-nanite--lod-notes)
10. [Collision Guidelines](#10-collision-guidelines)
11. [Automation Scripts](#11-automation-scripts)

---

## 1. Naming Conventions

| Prefix | Type                    | Example                        |
|--------|-------------------------|--------------------------------|
| `SM_`  | Static Mesh             | `SM_ControlPanel_A`            |
| `SK_`  | Skeletal Mesh           | `SK_PlayerArms`                |
| `T_`   | Texture                 | `T_ControlPanel_A_BaseColor`   |
| `M_`   | Master Material         | `M_Structural_Metal`           |
| `MI_`  | Material Instance       | `MI_ControlPanel_A`            |
| `MAT_` | Material (imported)     | `MAT_ControlPanel_Imported`    |

### Texture suffix conventions

| Suffix         | Channel content                     |
|----------------|-------------------------------------|
| `_BaseColor`   | RGB albedo                          |
| `_Normal`      | DirectX tangent-space normal map    |
| `_ORM`         | Packed: R=AO, G=Roughness, B=Metal |
| `_Emissive`    | RGB emissive color                  |
| `_Mask`        | Grayscale utility mask              |

### Blender object naming

Name objects in Blender **without** the `SM_` prefix. The export script adds it automatically.

- Use PascalCase: `ControlPanel_A`, `DropPod_Door`, `CrateSmall`
- Suffix variants with `_A`, `_B`, `_C` (not numbers unless sequential)
- Separate collision hulls: `UCX_ControlPanel_A` (UE5 auto-detects this prefix)

---

## 2. Folder Structure

### Source (on disk)

```
Assets_Raw/
├── Custom/                   ← Your authored assets land here
│   ├── *.fbx                ← Exported meshes from Blender
│   └── Textures/
│       ├── T_*_BaseColor.png
│       ├── T_*_Normal.png
│       ├── T_*_ORM.png
│       └── T_*_Emissive.png
├── KayKit_FBX/              ← KayKit Space Base (MIT)
├── Kenney_SpaceKit/         ← Kenney Space Kit
├── Quaternius_SciFi/        ← Quaternius pack
└── SciFiDoor/               ← Animated door asset
```

### Content (in UE5 Content Browser)

```
Content/
├── Meshes/
│   ├── Custom/              ← import_custom.py target
│   ├── Kenney_SpaceKit/
│   └── Quaternius_SciFi/
├── Textures/
│   └── Custom/              ← Auto-imported textures
├── Materials/
│   ├── Master/              ← M_ master materials (if using .uasset)
│   └── Instances/           ← MI_ material instances
├── KayKit/
│   └── SpaceBase/           ← KayKit meshes + mats
└── Maps/
    └── *.umap
```

---

## 3. Performance Budgets

All counts assume Nanite-enabled meshes (no traditional LODs needed). Triangle counts below are the **source mesh** high-poly count before Nanite streaming.

| Asset Type        | Triangles   | Texture Res      | Draw Distance | Notes                          |
|-------------------|-------------|------------------|---------------|--------------------------------|
| **Hero prop**     | 50k-200k    | 2048x2048        | 0-50m         | Weapons, terminals, reactors   |
| **Medium prop**   | 10k-50k     | 1024x1024        | 0-200m        | Crates, barrels, consoles      |
| **Small prop**    | 1k-10k      | 512x512          | 0-100m        | Debris, bolts, small detail    |
| **Building**      | 100k-500k   | 2048x2048 (trim) | 0-1000m       | Compound structures            |
| **Vehicle**       | 50k-150k    | 2048x2048        | 0-500m        | Hover vehicle, trucks          |
| **Weapon (FP)**   | 20k-60k     | 2048x2048        | Always on     | First-person viewmodel         |
| **Weapon (WP)**   | 5k-15k      | 1024x1024        | 0-100m        | World/pickup model             |
| **Terrain rock**  | 5k-30k      | 1024x1024        | 0-500m        | Boulder, outcrop               |
| **Foliage/debris**| 500-5k      | 512x512          | 0-200m        | Scattered small objects        |

### Memory targets per texture set

- 2048x2048 RGBA (BC7): ~5.3 MB per map (with mips)
- 1024x1024 RGBA (BC7): ~1.3 MB per map
- 512x512 RGBA (BC7): ~0.3 MB per map

### Scene-wide budget

- Total unique meshes on screen: aim for <500 Nanite meshes
- Total unique textures in a zone: <200 MB GPU texture memory
- Total unique materials: <100 per visible zone

---

## 4. Blender Modeling Guidelines

### Version

Blender 4.2 LTS or newer.

### Topology for Nanite

Nanite handles high-poly natively. You do NOT need:
- Traditional LOD chains
- Carefully optimized quad topology for runtime
- Manually reduced meshes

You DO need:
- **Manifold, watertight meshes** — no holes, no overlapping faces, no interior faces
- **Consistent normals** — all faces pointing outward
- **Clean geometry** — no zero-area faces, no duplicate vertices
- **Quads or tris** — Nanite accepts both, but avoid n-gons (5+ verts). The export script triangulates automatically
- **No overlapping UVs within the same UV island** — packing overlaps are fine across UV sets, but individual islands must not overlap themselves

### UV Layout

- **One UV channel (UV0)** for texturing — this maps to Substance Painter and UE5 material UV
- **Second UV channel (UV1)** for lightmaps if not using Lumen (we use Lumen, so UV1 is optional but good practice)
- UV0 texel density: aim for consistent density across the mesh. In Substance Painter, use `Texture Set Settings > UV Tile Ratio` to verify
- Pack UVs with 2-4px padding at 2048 resolution (4-8px at 4096) to prevent bleeding
- No UV stretching above 15% variance

### Scale

- **Work in metric (meters)**. 1 Blender unit = 1m = 100 UE units
- A standard crate: ~0.6m x 0.6m x 0.6m in Blender
- A character: ~1.85m tall
- ALWAYS apply scale (Ctrl+A > Scale) before export

### Origin

- Place origin at the **bottom center** of the object (where it sits on a surface)
- Exception: doors/hatches — origin at the hinge point
- Exception: weapons — origin at the grip/hand attachment point

### Collision Hulls

For custom collision (instead of auto-generated), create simple convex meshes:
- Name them `UCX_<MeshName>_##` (e.g., `UCX_ControlPanel_A_00`, `UCX_ControlPanel_A_01`)
- They import alongside the visual mesh and UE5 auto-assigns them
- Keep collision hulls convex (UE5 requires convex decomposition)
- Max 32 convex hulls per mesh

### Checklist before export

- [ ] Scale applied (Ctrl+A > All Transforms)
- [ ] Normals consistent (Mesh > Normals > Recalculate Outside)
- [ ] No n-gons (Select > All by Trait > Faces by Sides > Greater Than 4)
- [ ] No non-manifold edges (Select > All by Trait > Non Manifold)
- [ ] No loose vertices/edges
- [ ] Origin placed correctly
- [ ] UV0 unwrapped with no overlaps
- [ ] Named correctly (PascalCase, no SM_ prefix)

---

## 5. FBX Export from Blender

### Manual export settings

If not using the `blender_export_ue5.py` script:

1. File > Export > FBX (.fbx)
2. Settings:

| Setting                  | Value              |
|--------------------------|--------------------|
| Path Mode               | Copy (embed textures OFF for our pipeline) |
| Forward                 | -Y Forward         |
| Up                      | Z Up               |
| Apply Unit              | ON                 |
| Apply Transform         | OFF (we apply manually before export) |
| Mesh Only               | ON (unless exporting skeleton) |
| Apply Modifiers         | ON                 |
| Triangulate Faces       | ON                 |
| Tangent Space           | ON                 |
| Smoothing               | Face               |
| Armature > Add Leaf Bones | OFF              |

### Automated export

Use `Scripts/blender_export_ue5.py` — it validates the mesh and exports with correct settings. See [Section 11](#11-automation-scripts).

Run in Blender:
```
Blender > Edit > Preferences > File Paths > Scripts (add exorift/Scripts)
# Then in scripting workspace:
exec(open("C:/Users/falk/exorift/Scripts/blender_export_ue5.py").read())
```

Or from Blender command line:
```bash
blender --background myfile.blend --python "C:/Users/falk/exorift/Scripts/blender_export_ue5.py"
```

---

## 6. Substance Painter Texturing

### Project Setup

1. **Template**: PBR - Metallic Roughness (Alpha Blending)
2. **Resolution**: 2048x2048 for hero props, 1024x1024 for medium, 512x512 for small
3. **Normal Map Format**: DirectX (Y-)
4. **Compute tangent space per fragment**: ON

### Channels to author

| Channel       | Content                                | sRGB? |
|---------------|----------------------------------------|-------|
| Base Color    | Albedo RGB                             | Yes   |
| Roughness     | Roughness (0=mirror, 1=matte)         | No    |
| Metallic      | Metallic (0=dielectric, 1=metal)       | No    |
| Normal        | DirectX tangent-space normal           | No    |
| Ambient Occ.  | Baked AO                               | No    |
| Emissive      | RGB emissive glow                      | Yes   |

### PBR Value Targets (from ExoRift Style Bible)

| Surface Type  | Metallic Range | Roughness Range | Notes                          |
|---------------|----------------|-----------------|--------------------------------|
| Terrain       | 0.00 - 0.02    | 0.82 - 0.94    | Dirt, sand, rock               |
| Structural    | 0.35 - 0.60    | 0.40 - 0.60    | Walls, floors, panels          |
| Accent/Metal  | 0.70 - 0.85    | 0.20 - 0.30    | Trim, pipes, vents, handles    |

### Emissive guidelines

- Persistent scene emissive: max 3x multiplier in UE5 material
- Keep emissive areas small (indicator lights, panel strips, display screens)
- Color: predominantly cyan/teal (#00D4FF) or amber (#FFB347) to match palette
- Never pure white emissive (#FFFFFF) — always tinted

### Export Preset

Use `Scripts/substance_export_preset.json` for the export config. Key outputs:

| Output Map      | Channels          | Format    | Bit Depth |
|-----------------|-------------------|-----------|-----------|
| BaseColor       | RGB               | PNG       | 8-bit     |
| Normal          | RGB (DX, Y- flip) | PNG       | 16-bit    |
| ORM             | R=AO, G=Rough, B=Metal | PNG  | 8-bit     |
| Emissive        | RGB               | PNG       | 8-bit     |

### Export steps

1. File > Export Textures
2. Config: select `ExoRift_UE5` preset (import from `substance_export_preset.json`)
3. Output directory: `C:/Users/falk/exorift/Assets_Raw/Custom/Textures/`
4. File type: PNG
5. Padding: Dilation (infinite)
6. Size: Match document resolution

### Naming

Substance Painter auto-names based on texture set name. Ensure your mesh name matches:
- Mesh: `ControlPanel_A` in Blender
- Substance exports: `T_ControlPanel_A_BaseColor.png`, `T_ControlPanel_A_Normal.png`, etc.

To get this naming, set the output template in the export dialog to:
```
T_$mesh_$textureSet_$channel
```

Or if single texture set per mesh:
```
T_$mesh_$channel
```

---

## 7. UE5 Import Settings

### FBX Import (Static Mesh)

When importing FBX meshes manually in UE5:

| Setting                         | Value           |
|---------------------------------|-----------------|
| Import Mesh                     | ON              |
| Import Textures                 | OFF (we import separately) |
| Import Materials                | OFF (we create instances from masters) |
| Import as Skeletal              | OFF             |
| Combine Meshes                  | ON              |
| Auto Generate Collision         | ON              |
| Generate Lightmap UVs           | OFF (Lumen, not baked) |
| Transform Vertex to Absolute    | ON              |
| Convert Scene                   | ON              |
| Normal Import Method            | Import Normals  |
| Normal Generation Method        | MikkTSpace      |

### Texture Import

| Setting                         | Value           |
|---------------------------------|-----------------|
| Compression                     | Default for BaseColor/Emissive (DXT5/BC7) |
| Compression (Normal)            | Normalmap (BC5) |
| Compression (ORM)               | Masks (no sRGB) |
| sRGB                            | ON for BaseColor, OFF for Normal/ORM |
| LOD Bias                        | 0               |
| Mip Gen Settings                | From Texture Group |

### Post-Import: Enable Nanite

For every imported static mesh:
1. Open the mesh asset
2. Nanite Settings > Enable Nanite Support: **ON**
3. Hit "Apply Changes"

Or via Python (see `import_custom.py`):
```python
mesh.set_editor_property('enable_nanite', True)
```

---

## 8. Material Setup in UE5

### Option A: C++ Runtime Materials (current ExoRift approach)

ExoRift creates materials at runtime via `FExoMaterialFactory`:
- `GetLitEmissive()` — PBR + emissive glow
- `GetLitTextured()` — PBR + procedural detail noise
- `GetEmissiveAdditive()` — Additive unlit for VFX
- `GetGlassTranslucent()` — Glass with specular

For custom-textured assets, create a new material instance in C++ using `UMaterialInstanceDynamic`:
```cpp
UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(
    FExoMaterialFactory::GetLitEmissive(), this);
MID->SetTextureParameterValue(TEXT("BaseColor"), LoadedBaseColorTex);
MID->SetTextureParameterValue(TEXT("Normal"), LoadedNormalTex);
MID->SetTextureParameterValue(TEXT("ORM"), LoadedORMTex);
MID->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0, 0.83f, 1.0f));
MID->SetScalarParameterValue(TEXT("EmissiveMultiplier"), 2.5f);
```

### Option B: Editor Material Instances

If creating materials in the editor:
1. Create a Master Material with BaseColor, Normal, ORM, Emissive texture inputs
2. Create Material Instance (`MI_AssetName`)
3. Assign textures in instance parameters
4. Assign to mesh

### Texture assignment quick reference

| Parameter         | Texture suffix   | sRGB | Sampler    |
|-------------------|------------------|------|------------|
| BaseColor         | `_BaseColor`     | Yes  | Color      |
| Normal            | `_Normal`        | No   | Normal     |
| ORM               | `_ORM`           | No   | Linear     |
| Emissive          | `_Emissive`      | Yes  | Color      |

---

## 9. Nanite & LOD Notes

### Nanite

- **Enable on all static meshes** except: foliage with alpha masking, translucent meshes, meshes that need to deform
- Nanite replaces traditional LODs entirely — no need for LOD0/LOD1/LOD2
- Nanite meshes can be arbitrarily high-poly (100k+ tris is fine)
- Nanite handles draw-distance streaming automatically
- **Collision is separate** — Nanite does not use visual mesh for collision

### When NOT to use Nanite

- Translucent/masked materials (glass, foliage, holograms)
- Skinned/skeletal meshes (characters, weapons in first-person)
- World Partition landscape (uses its own LOD system)
- Very small meshes under 128 triangles (Nanite overhead exceeds benefit)

### LODs for non-Nanite meshes

If a mesh cannot use Nanite, create manual LODs:
- LOD0: full detail (100%)
- LOD1: 50% at 20m
- LOD2: 25% at 50m
- LOD3: 12% at 100m

---

## 10. Collision Guidelines

### Auto-generated (default)

UE5 auto-generates collision from the mesh. Works well for simple convex shapes.

### Custom collision

For complex shapes (L-shaped walls, archways, furniture):
- Model simple convex hulls in Blender
- Name them `UCX_<MeshName>_00`, `UCX_<MeshName>_01`, etc.
- Export in same FBX — UE5 auto-detects and assigns
- Aim for 1-8 convex hulls per mesh

### Per-poly collision

Only for:
- Terrain/landscape collision proxy
- Complex walkable surfaces

Enable in UE5: Mesh > Collision Complexity > Use Complex Collision As Simple

**Warning:** Per-poly collision is expensive. Use sparingly.

---

## 11. Automation Scripts

### `Scripts/blender_export_ue5.py`

Blender Python script — validates mesh, applies transforms, exports FBX with UE5-correct settings. Run inside Blender.

### `Scripts/substance_export_preset.json`

Substance Painter export preset — BaseColor + Normal(DX) + ORM + Emissive. Import into Substance Painter via File > Export Textures > Config > Import preset.

### `Scripts/import_custom.py`

UE5 Python import script — scans `Assets_Raw/Custom/`, imports FBX to `/Game/Meshes/Custom/`, enables Nanite, imports textures to `/Game/Textures/Custom/`. Run in UE5 via:

```
UnrealEditor.exe exorift.uproject -ExecCmds="py Scripts/import_custom.py"
```

Or in-editor: Tools > Execute Python Script.

---

## Quick Reference: Full Pipeline Walkthrough

```
1. MODEL in Blender 4.2+
   - Follow topology/UV guidelines above
   - Name: ControlPanel_A (no SM_ prefix)
   - Run blender_export_ue5.py
   - Output: Assets_Raw/Custom/SM_ControlPanel_A.fbx

2. TEXTURE in Substance Painter
   - Import SM_ControlPanel_A.fbx
   - Paint using PBR Metallic Roughness
   - Export with ExoRift_UE5 preset
   - Output: Assets_Raw/Custom/Textures/T_ControlPanel_A_*.png

3. IMPORT into UE5.7
   - Run Scripts/import_custom.py in editor
   - Meshes → /Game/Meshes/Custom/
   - Textures → /Game/Textures/Custom/
   - Nanite auto-enabled on meshes
   - Material instances auto-created

4. PLACE in level
   - Reference from C++ via TSoftObjectPtr<UStaticMesh>
   - Or place in editor viewport
```
