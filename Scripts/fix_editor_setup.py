"""Fix editor setup: clear GameMode override, close Fab tab, save level."""
import unreal

# Clear the per-level GameMode override so GlobalDefaultGameMode takes effect
world = unreal.EditorLevelLibrary.get_editor_world()
ws = world.get_world_settings()
ws.set_editor_property('game_mode_override_class', None)
print('[ExoRift] Cleared GameMode override — will use ExoGameMode from DefaultEngine.ini')

# Save the level
unreal.EditorLevelLibrary.save_current_level()
print('[ExoRift] Level saved')
