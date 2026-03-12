import unreal
import os

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
fbx_dir = r'C:\Users\falk\exorift\Assets_Raw\Kenney_SpaceKit\Models\FBX format'
dest_path = '/Game/Meshes/Kenney_SpaceKit'

count = 0
for f in os.listdir(fbx_dir):
    if f.endswith('.fbx'):
        task = unreal.AssetImportTask()
        task.automated = True
        task.destination_path = dest_path
        task.filename = os.path.join(fbx_dir, f)
        task.replace_existing = True
        task.save = True
        
        options = unreal.FbxImportUI()
        options.import_mesh = True
        options.import_textures = True
        options.import_materials = True
        options.import_as_skeletal = False
        options.static_mesh_import_data.auto_generate_collision = True
        task.options = options
        
        asset_tools.import_asset_tasks([task])
        count += 1
        print(f'Imported: {f}')

# Also import textures from the Textures subfolder
tex_dir = os.path.join(fbx_dir, 'Textures')
if os.path.exists(tex_dir):
    tex_dest = '/Game/Textures/Kenney_SpaceKit'
    for t in os.listdir(tex_dir):
        if t.lower().endswith(('.png', '.jpg', '.tga')):
            tex_task = unreal.AssetImportTask()
            tex_task.automated = True
            tex_task.destination_path = tex_dest
            tex_task.filename = os.path.join(tex_dir, t)
            tex_task.replace_existing = True
            tex_task.save = True
            asset_tools.import_asset_tasks([tex_task])
            print(f'Imported texture: {t}')

print(f'Done! Imported {count} FBX files from Kenney Space Kit')
