"""
ExoRift — Import Custom Assets into UE5.7
Scans Assets_Raw/Custom/ for FBX meshes and textures, imports them into
/Game/Meshes/Custom/ and /Game/Textures/Custom/, enables Nanite on meshes,
and auto-creates basic material instances.

Run in UE5 via:
  - In-editor: Tools > Execute Python Script > browse to this file
  - Command line:
    "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" ^
      "C:/Users/falk/exorift/exorift.uproject" ^
      -ExecCmds="py C:/Users/falk/exorift/Scripts/import_custom.py" -unattended

Note: For Nanite to be enabled on import, the editor must be running (not -Cmd).
Use the full editor with:
  UnrealEditor.exe exorift.uproject -ExecCmds="py Scripts/import_custom.py"
"""
import unreal
import os
import re

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

PROJECT_DIR = r"C:\Users\falk\exorift"
FBX_DIR = os.path.join(PROJECT_DIR, "Assets_Raw", "Custom")
TEX_DIR = os.path.join(PROJECT_DIR, "Assets_Raw", "Custom", "Textures")

MESH_DEST = "/Game/Meshes/Custom"
TEX_DEST = "/Game/Textures/Custom"
MAT_DEST = "/Game/Materials/Instances/Custom"

# Texture suffixes we recognize
TEXTURE_SUFFIXES = ["_BaseColor", "_Normal", "_ORM", "_Emissive", "_Mask"]

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_asset_lib = unreal.EditorAssetLibrary


def log(msg):
    """Print and log to UE output."""
    unreal.log(f"[ImportCustom] {msg}")


def get_asset_name_from_texture(filename):
    """Extract the base asset name from a texture filename.
    e.g. T_ControlPanel_A_BaseColor.png -> ControlPanel_A
    """
    name = os.path.splitext(filename)[0]
    # Remove T_ prefix if present
    if name.startswith("T_"):
        name = name[2:]
    # Remove known suffixes
    for suffix in TEXTURE_SUFFIXES:
        if name.endswith(suffix):
            name = name[:-len(suffix)]
            break
    return name


def get_texture_type(filename):
    """Determine texture type from filename suffix."""
    name = os.path.splitext(filename)[0]
    for suffix in TEXTURE_SUFFIXES:
        if name.endswith(suffix):
            return suffix[1:]  # Remove leading underscore
    return None


# ---------------------------------------------------------------------------
# Import FBX meshes
# ---------------------------------------------------------------------------

def import_meshes():
    """Import all FBX files from Assets_Raw/Custom/."""
    if not os.path.isdir(FBX_DIR):
        log(f"FBX directory not found: {FBX_DIR}")
        return []

    fbx_files = sorted([f for f in os.listdir(FBX_DIR) if f.lower().endswith('.fbx')])
    if not fbx_files:
        log("No FBX files found in Assets_Raw/Custom/")
        return []

    log(f"Found {len(fbx_files)} FBX file(s) to import")

    imported_meshes = []
    tasks = []

    for fbx_file in fbx_files:
        # Determine destination name (strip SM_ prefix for the asset name,
        # UE5 will use the filename as-is)
        dest_name = os.path.splitext(fbx_file)[0]

        task = unreal.AssetImportTask()
        task.set_editor_property('automated', True)
        task.set_editor_property('destination_path', MESH_DEST)
        task.set_editor_property('destination_name', dest_name)
        task.set_editor_property('filename', os.path.join(FBX_DIR, fbx_file))
        task.set_editor_property('replace_existing', True)
        task.set_editor_property('save', True)

        # FBX import options
        options = unreal.FbxImportUI()
        options.set_editor_property('import_mesh', True)
        options.set_editor_property('import_textures', False)  # We import textures separately
        options.set_editor_property('import_materials', False)  # We create our own materials
        options.set_editor_property('import_as_skeletal', False)

        sm_data = options.get_editor_property('static_mesh_import_data')
        sm_data.set_editor_property('combine_meshes', True)
        sm_data.set_editor_property('auto_generate_collision', True)
        sm_data.set_editor_property('generate_lightmap_u_vs', False)  # Lumen, no lightmap UVs
        sm_data.set_editor_property('normal_import_method',
                                    unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
        sm_data.set_editor_property('normal_generation_method',
                                    unreal.FBXNormalGenerationMethod.MIKK_T_SPACE)

        task.set_editor_property('options', options)
        tasks.append((fbx_file, dest_name, task))

    # Run all import tasks
    all_tasks = [t[2] for t in tasks]
    asset_tools.import_asset_tasks(all_tasks)

    # Verify imports and enable Nanite
    for fbx_file, dest_name, task in tasks:
        imported_paths = task.get_editor_property('imported_object_paths')
        if imported_paths:
            asset_path = str(imported_paths[0]) if imported_paths else None
            if asset_path:
                log(f"Imported mesh: {fbx_file} -> {asset_path}")
                imported_meshes.append((dest_name, asset_path))
                enable_nanite(asset_path)
            else:
                log(f"WARNING: {fbx_file} imported but no asset path returned")
        else:
            # Fallback: check if asset exists at expected path
            expected_path = f"{MESH_DEST}/{dest_name}"
            if editor_asset_lib.does_asset_exist(expected_path):
                log(f"Imported mesh (fallback check): {fbx_file} -> {expected_path}")
                imported_meshes.append((dest_name, expected_path))
                enable_nanite(expected_path)
            else:
                log(f"FAILED to import: {fbx_file}")

    return imported_meshes


def enable_nanite(asset_path):
    """Enable Nanite on a static mesh asset."""
    try:
        mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
        if mesh and isinstance(mesh, unreal.StaticMesh):
            # Build Nanite data
            nanite_settings = mesh.get_editor_property('nanite_settings')
            nanite_settings.set_editor_property('enabled', True)
            mesh.set_editor_property('nanite_settings', nanite_settings)

            # Save the asset
            unreal.EditorAssetLibrary.save_asset(asset_path)
            log(f"  Nanite enabled: {asset_path}")
        else:
            log(f"  WARNING: Could not load as StaticMesh: {asset_path}")
    except Exception as e:
        log(f"  WARNING: Nanite enable failed for {asset_path}: {e}")


# ---------------------------------------------------------------------------
# Import textures
# ---------------------------------------------------------------------------

def import_textures():
    """Import all texture files from Assets_Raw/Custom/Textures/."""
    if not os.path.isdir(TEX_DIR):
        log(f"Texture directory not found: {TEX_DIR}")
        return {}

    tex_files = sorted([f for f in os.listdir(TEX_DIR)
                        if f.lower().endswith(('.png', '.tga', '.jpg', '.jpeg', '.bmp', '.exr'))])
    if not tex_files:
        log("No texture files found in Assets_Raw/Custom/Textures/")
        return {}

    log(f"Found {len(tex_files)} texture file(s) to import")

    # Group textures by base asset name
    texture_map = {}  # asset_name -> {type: asset_path}

    tasks = []
    for tex_file in tex_files:
        tex_name = os.path.splitext(tex_file)[0]
        asset_name = get_asset_name_from_texture(tex_file)
        tex_type = get_texture_type(tex_file)

        task = unreal.AssetImportTask()
        task.set_editor_property('automated', True)
        task.set_editor_property('destination_path', TEX_DEST)
        task.set_editor_property('destination_name', tex_name)
        task.set_editor_property('filename', os.path.join(TEX_DIR, tex_file))
        task.set_editor_property('replace_existing', True)
        task.set_editor_property('save', True)

        tasks.append((tex_file, tex_name, asset_name, tex_type, task))

    # Run all import tasks
    all_tasks = [t[4] for t in tasks]
    asset_tools.import_asset_tasks(all_tasks)

    # Post-import: set compression and sRGB settings based on texture type
    for tex_file, tex_name, asset_name, tex_type, task in tasks:
        asset_path = f"{TEX_DEST}/{tex_name}"

        if editor_asset_lib.does_asset_exist(asset_path):
            log(f"Imported texture: {tex_file} -> {asset_path}")

            # Configure texture settings based on type
            configure_texture(asset_path, tex_type)

            # Track for material creation
            if asset_name not in texture_map:
                texture_map[asset_name] = {}
            if tex_type:
                texture_map[asset_name][tex_type] = asset_path
        else:
            log(f"FAILED to import texture: {tex_file}")

    return texture_map


def configure_texture(asset_path, tex_type):
    """Set compression and sRGB based on texture type."""
    try:
        tex = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not tex or not isinstance(tex, unreal.Texture2D):
            return

        if tex_type == "Normal":
            tex.set_editor_property('compression_settings',
                                    unreal.TextureCompressionSettings.TC_NORMALMAP)
            tex.set_editor_property('srgb', False)
            log(f"  Set Normal compression: {asset_path}")

        elif tex_type == "ORM":
            tex.set_editor_property('compression_settings',
                                    unreal.TextureCompressionSettings.TC_MASKS)
            tex.set_editor_property('srgb', False)
            log(f"  Set Masks compression (linear): {asset_path}")

        elif tex_type == "Mask":
            tex.set_editor_property('compression_settings',
                                    unreal.TextureCompressionSettings.TC_MASKS)
            tex.set_editor_property('srgb', False)
            log(f"  Set Masks compression (linear): {asset_path}")

        elif tex_type == "BaseColor":
            tex.set_editor_property('compression_settings',
                                    unreal.TextureCompressionSettings.TC_DEFAULT)
            tex.set_editor_property('srgb', True)
            log(f"  Set Default compression (sRGB): {asset_path}")

        elif tex_type == "Emissive":
            tex.set_editor_property('compression_settings',
                                    unreal.TextureCompressionSettings.TC_DEFAULT)
            tex.set_editor_property('srgb', True)
            log(f"  Set Default compression (sRGB): {asset_path}")

        unreal.EditorAssetLibrary.save_asset(asset_path)

    except Exception as e:
        log(f"  WARNING: Could not configure texture {asset_path}: {e}")


# ---------------------------------------------------------------------------
# Auto-create material instances
# ---------------------------------------------------------------------------

def create_material_instances(imported_meshes, texture_map):
    """Create material instances for imported meshes that have matching textures."""
    if not texture_map:
        log("No texture sets found — skipping material instance creation")
        return

    created = 0

    for dest_name, mesh_path in imported_meshes:
        # Strip SM_ prefix to match texture naming
        asset_name = dest_name
        if asset_name.startswith("SM_"):
            asset_name = asset_name[3:]

        if asset_name not in texture_map:
            log(f"No textures found for {asset_name} — skipping material")
            continue

        textures = texture_map[asset_name]
        mi_name = f"MI_{asset_name}"
        mi_path = f"{MAT_DEST}/{mi_name}"

        log(f"Creating material instance: {mi_name}")
        log(f"  Textures available: {', '.join(textures.keys())}")

        try:
            # Find a base material to instance from. Try to find an existing
            # master material in the project. Fall back to engine default.
            parent_path = "/Engine/EngineMaterials/DefaultMaterial"

            # Check if we have a project master material
            for candidate in [
                "/Game/Materials/Master/M_Master_Opaque",
                "/Game/Materials/Master/M_PBR_Standard",
                "/Game/Materials/M_Master",
            ]:
                if editor_asset_lib.does_asset_exist(candidate):
                    parent_path = candidate
                    break

            parent_mat = unreal.EditorAssetLibrary.load_asset(parent_path)
            if not parent_mat:
                log(f"  WARNING: Could not load parent material {parent_path}")
                continue

            # Create material instance via AssetTools
            mi_factory = unreal.MaterialInstanceConstantFactoryNew()
            mi = asset_tools.create_asset(mi_name, MAT_DEST, unreal.MaterialInstanceConstant,
                                          mi_factory)

            if not mi:
                log(f"  WARNING: Failed to create material instance {mi_name}")
                continue

            mi.set_editor_property('parent', parent_mat)

            # Set texture parameters
            for tex_type, tex_path in textures.items():
                tex_asset = unreal.EditorAssetLibrary.load_asset(tex_path)
                if not tex_asset:
                    continue

                param_name = tex_type  # BaseColor, Normal, ORM, Emissive
                try:
                    # Use the set_texture_parameter_value_editor_only method
                    mi.set_editor_property('texture_parameter_values', [])
                    # For UE5, we need to use the proper API
                    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                        mi, param_name, tex_asset)
                    log(f"  Set {param_name}: {tex_path}")
                except Exception as e:
                    log(f"  Could not set {param_name} parameter: {e}")

            # Save the material instance
            unreal.EditorAssetLibrary.save_asset(mi_path)

            # Assign material to the mesh
            try:
                mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
                if mesh and isinstance(mesh, unreal.StaticMesh):
                    # Get the number of material slots
                    num_materials = mesh.get_num_sections(0)
                    for i in range(num_materials):
                        mesh.set_material(i, mi)
                    unreal.EditorAssetLibrary.save_asset(mesh_path)
                    log(f"  Assigned {mi_name} to {dest_name}")
            except Exception as e:
                log(f"  Could not assign material to mesh: {e}")

            created += 1

        except Exception as e:
            log(f"  ERROR creating material instance for {asset_name}: {e}")

    log(f"Created {created} material instance(s)")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    log("=" * 60)
    log("ExoRift Custom Asset Import")
    log(f"FBX source:     {FBX_DIR}")
    log(f"Texture source: {TEX_DIR}")
    log(f"Mesh target:    {MESH_DEST}")
    log(f"Texture target: {TEX_DEST}")
    log(f"Material target:{MAT_DEST}")
    log("=" * 60)

    # Step 1: Import meshes
    log("\n--- STEP 1: Import FBX Meshes ---")
    imported_meshes = import_meshes()

    # Step 2: Import textures
    log("\n--- STEP 2: Import Textures ---")
    texture_map = import_textures()

    # Step 3: Create material instances
    log("\n--- STEP 3: Create Material Instances ---")
    create_material_instances(imported_meshes, texture_map)

    # Summary
    log("\n" + "=" * 60)
    log("IMPORT COMPLETE")
    log(f"  Meshes imported:    {len(imported_meshes)}")
    log(f"  Texture sets:       {len(texture_map)}")
    log(f"  Assets at:          {MESH_DEST}")
    log("=" * 60)


if __name__ == "__main__":
    main()
else:
    main()
