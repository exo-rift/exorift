import unreal
import os

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
base_dir = r'C:\Users\falk\exorift\Assets_Raw\Quaternius_SciFi\FBX\FBX'
dest_path = '/Game/Meshes/Quaternius_SciFi'

count = 0
for root, dirs, files in os.walk(base_dir):
    for f in sorted(files):
        if f.endswith('.fbx'):
            # Create subfolder path matching source structure
            rel = os.path.relpath(root, base_dir)
            if rel == '.':
                sub_dest = dest_path
            else:
                sub_dest = dest_path + '/' + rel.replace(os.sep, '/')

            task = unreal.AssetImportTask()
            task.automated = True
            task.destination_path = sub_dest
            task.filename = os.path.join(root, f)
            task.replace_existing = True
            task.save = True

            options = unreal.FbxImportUI()
            options.import_mesh = True
            options.import_textures = False
            options.import_materials = True
            options.import_as_skeletal = False
            options.static_mesh_import_data.auto_generate_collision = True
            task.options = options

            asset_tools.import_asset_tasks([task])
            count += 1
            print(f'[{count}] Imported: {f}')

print(f'Done! Imported {count} Quaternius FBX files')
