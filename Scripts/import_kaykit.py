"""
Import KayKit Space Base FBX files into UE5 project.
Run from within the UE Editor: Tools -> Execute Python Script, or via:
  UnrealEditor.exe project.uproject -ExecCmds="py Scripts/import_kaykit.py"
"""
import unreal
import os

fbx_dir = r"C:\Users\falk\exorift\Assets_Raw\KayKit_FBX"
dest_path = "/Game/KayKit/SpaceBase"

def import_fbx_files():
    """Import all FBX files from the staging directory."""
    fbx_files = sorted([f for f in os.listdir(fbx_dir) if f.lower().endswith('.fbx')])
    full_paths = [os.path.join(fbx_dir, f) for f in fbx_files]
    unreal.log(f"KayKit import: Found {len(fbx_files)} FBX files")

    # Build import tasks
    tasks = []
    for fbx_file in fbx_files:
        task = unreal.AssetImportTask()
        task.set_editor_property('automated', True)
        task.set_editor_property('destination_path', dest_path)
        task.set_editor_property('destination_name', os.path.splitext(fbx_file)[0])
        task.set_editor_property('filename', os.path.join(fbx_dir, fbx_file))
        task.set_editor_property('replace_existing', True)
        task.set_editor_property('save', True)

        options = unreal.FbxImportUI()
        options.set_editor_property('import_mesh', True)
        options.set_editor_property('import_textures', True)
        options.set_editor_property('import_materials', True)
        options.set_editor_property('import_as_skeletal', False)
        options.static_mesh_import_data.set_editor_property('combine_meshes', True)
        options.static_mesh_import_data.set_editor_property('auto_generate_collision', True)
        task.set_editor_property('options', options)
        tasks.append(task)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    imported = sum(1 for t in tasks
                   if t.get_editor_property('imported_object_paths'))
    unreal.log(f"KayKit import: {imported}/{len(fbx_files)} succeeded")
    unreal.log(f"Assets at: {dest_path}")

if __name__ == "__main__":
    import_fbx_files()
