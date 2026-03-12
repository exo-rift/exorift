import unreal

def import_fbx():
    # Setup import task
    task = unreal.AssetImportTask()
    task.set_editor_property('automated', True)
    task.set_editor_property('destination_name', 'SciFiDoor')
    task.set_editor_property('destination_path', '/Game/Meshes/SciFiDoor')
    task.set_editor_property('filename', 'C:/Users/falk/exorift/Assets_Raw/SciFiDoor/source/Sci-Fi_Door_anim.fbx')
    task.set_editor_property('replace_existing', True)
    task.set_editor_property('save', True)

    # FBX import options
    options = unreal.FbxImportUI()
    options.set_editor_property('import_mesh', True)
    options.set_editor_property('import_textures', True)
    options.set_editor_property('import_materials', True)
    options.set_editor_property('import_as_skeletal', False)
    options.set_editor_property('import_animations', True)
    
    # Static mesh import data
    sm_data = options.static_mesh_import_data
    sm_data.set_editor_property('combine_meshes', True)
    
    task.set_editor_property('options', options)
    
    # Run import
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    
    if task.get_editor_property('imported_object_paths'):
        print(f"SUCCESS: Imported to {task.get_editor_property('imported_object_paths')}")
    else:
        print("Import completed (check Content/Meshes/SciFiDoor)")

import_fbx()
