import unreal
import os

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Import FBX Door
fbx_path = r'C:\Users\falk\exorift\Assets_Raw\SciFiDoor\source\Sci-Fi_Door_anim.fbx'
dest_path = '/Game/Meshes/SciFiDoor'

task = unreal.AssetImportTask()
task.automated = True
task.destination_name = ''
task.destination_path = dest_path
task.filename = fbx_path
task.replace_existing = True
task.save = True

options = unreal.FbxImportUI()
options.import_mesh = True
options.import_textures = True  
options.import_materials = True
options.import_as_skeletal = False
options.import_animations = True
options.static_mesh_import_data.combine_meshes = False
options.static_mesh_import_data.auto_generate_collision = True

task.options = options
asset_tools.import_asset_tasks([task])

# Also import textures separately to ensure they're available
texture_dir = r'C:\Users\falk\exorift\Assets_Raw\SciFiDoor\textures'
tex_dest = '/Game/Textures/SciFiDoor'

for tex_file in os.listdir(texture_dir):
    if tex_file.endswith('.png'):
        tex_task = unreal.AssetImportTask()
        tex_task.automated = True
        tex_task.destination_path = tex_dest
        tex_task.filename = os.path.join(texture_dir, tex_file)
        tex_task.replace_existing = True
        tex_task.save = True
        asset_tools.import_asset_tasks([tex_task])
        print(f'Imported texture: {tex_file}')

print('All imports complete!')
