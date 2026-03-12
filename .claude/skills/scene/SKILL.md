---
name: scene
description: Inspect and manipulate the live Unreal Editor scene via MCP. Use to spawn actors, move objects, check properties, list level contents, or set up test scenarios in the running editor.
argument-hint: [action description, e.g. "list all lights" or "spawn PointLight at 0,0,500"]
allowed-tools: mcp__unreal-chongdashu__spawn_actor, mcp__unreal-chongdashu__spawn_blueprint_actor, mcp__unreal-chongdashu__delete_actor, mcp__unreal-chongdashu__find_actors_by_name, mcp__unreal-chongdashu__get_actors_in_level, mcp__unreal-chongdashu__get_actor_properties, mcp__unreal-chongdashu__set_actor_property, mcp__unreal-chongdashu__set_actor_transform, mcp__unreal-chongdashu__set_physics_properties, mcp__unreal-chongdashu__set_static_mesh_properties, mcp__unreal-chongdashu__set_component_property, mcp__unreal-chongdashu__add_component_to_blueprint, mcp__unreal-chongdashu__create_blueprint, mcp__unreal-chongdashu__compile_blueprint, Read, Grep, Glob, Bash
---

# Live Scene Control

Interact with the running Unreal Editor via MCP tools.

Request: `$ARGUMENTS`

## Available Actions

### Inspect
- **List all actors**: `get_actors_in_level` → shows everything in the level
- **Find by name**: `find_actors_by_name` with pattern (e.g., `*Light*`, `*ExoBot*`)
- **Get properties**: `get_actor_properties` → full property dump of a specific actor

### Spawn
- **Basic actors**: `spawn_actor` with type (StaticMeshActor, PointLight, SpotLight, CameraActor, etc.)
- **Blueprint actors**: `spawn_blueprint_actor` for custom ExoRift classes
- **Location format**: `[x, y, z]` in centimeters, `[pitch, yaw, roll]` in degrees

### Modify
- **Move/rotate/scale**: `set_actor_transform` with location/rotation/scale arrays
- **Set property**: `set_actor_property` for any exposed UPROPERTY
- **Physics**: `set_physics_properties` for simulate_physics, mass, damping, gravity
- **Mesh**: `set_static_mesh_properties` to swap static meshes on components

### Delete
- **Remove actor**: `delete_actor` by name

## Common Workflows

### Test Lighting Setup
1. List current lights: `find_actors_by_name` pattern `*Light*`
2. Spawn test light: `spawn_actor` type PointLight at desired location
3. Adjust: `set_actor_property` to change Intensity, LightColor, AttenuationRadius
4. Take screenshot to verify

### Debug Actor Placement
1. Find the actor: `find_actors_by_name`
2. Get its transform: `get_actor_properties`
3. Report location, rotation, scale
4. Optionally adjust with `set_actor_transform`

### Set Up Test Scenario
1. Spawn required actors (bots, pickups, etc.)
2. Position them as needed
3. Verify with property checks
4. Take screenshot if visual confirmation needed

### Asset Verification
1. Find actors using a specific mesh/material
2. Check properties to verify asset paths are resolved
3. Flag any actors with null/missing asset references

## ExoRift Map Reference
- Map half-size: 200,000 cm (2km from center)
- Ground Z: 0
- Compounds: N (smokestack), S (Tesla coil), E (reactor), W (comms array)
- 7 ziplines between tower anchors
- 24 named POI locations

## Output Format

Report results clearly:
```
## Scene: [action performed]

### Result
[What was found/changed/spawned]

### Actor Details (if inspecting)
| Property | Value |
|----------|-------|
| Location | (X, Y, Z) |
| ... | ... |
```

### Rules
- Always confirm actor exists before trying to modify it
- When spawning test actors, prefix names with `Debug_` so they're easy to find and clean up
- Don't delete actors unless explicitly asked — they might be important
- Report actor counts when listing (e.g., "Found 47 actors matching *Light*")
