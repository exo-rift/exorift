---
name: screenshot
description: Take screenshots from the running Unreal Editor for visual debugging. Captures the viewport, reads the image, and analyzes visual issues. Use when the user wants to see what the game looks like, debug visual artifacts, or verify VFX changes.
argument-hint: [optional: camera location or description of what to capture]
allowed-tools: Bash, Read, Glob, mcp__unreal-chongdashu__find_actors_by_name, mcp__unreal-chongdashu__get_actors_in_level, mcp__unreal-chongdashu__set_actor_transform, mcp__unreal-chongdashu__get_actor_properties, mcp__unreal-chongdashu__spawn_actor, mcp__unreal-chongdashu__delete_actor, mcp__unreal-chongdashu__set_actor_property
---

# Screenshot & Visual Debug

Take a screenshot from the running UE5 editor and analyze it.

Context: `$ARGUMENTS`

## Method 1: UE Console Command via UnrealEditor-Cmd

Run a high-res screenshot capture through the editor:

```bash
# Take screenshot via UE automation
# The editor must be running with -messaging enabled
"C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" \
  "C:/Users/falk/exorift/exorift.uproject" \
  -ExecCmds="HighResShot 1920x1080" \
  -unattended -nosplash -nullrhi 2>&1
```

## Method 2: Saved Screenshots Directory

Check for existing screenshots taken via the editor (F9 or HighResShot command):

```bash
# Check standard UE screenshot locations
ls -la "C:/Users/falk/exorift/Saved/Screenshots/" 2>/dev/null
ls -la "C:/Users/falk/exorift/Saved/Screenshots/Windows/" 2>/dev/null
# Also check project root
ls -la C:/Users/falk/exorift/screenshot* 2>/dev/null
```

## Method 3: Use MCP to Position Camera First

If the user wants to capture a specific area:

1. Use `find_actors_by_name` to locate the subject
2. Use `get_actor_properties` to get its world position
3. Use `set_actor_transform` on a CameraActor to frame the shot
4. Then trigger the screenshot

## Analysis Steps

1. **Capture**: Take or find the most recent screenshot
2. **View**: Use the Read tool on the image file (Claude can view images)
3. **Analyze**: Based on user context, check for:
   - Missing geometry or invisible meshes
   - Material issues (black surfaces = missing material, pink = broken reference)
   - Lighting problems (too dark, blown out, shadow artifacts)
   - VFX visibility (particles, emissive glow, bloom)
   - Z-fighting (flickering overlapping surfaces)
   - LOD issues (wrong detail level visible)
   - UI/HUD overlay problems

## Comparison Mode

If the user wants before/after:
1. Find the previous screenshot (sorted by date)
2. Read both images
3. Describe what changed between them

## Output Format

```
## Screenshot Analysis

### Capture
[How the screenshot was taken, resolution, location]

### Observations
- [What's visible in the scene]
- [Any issues spotted]

### Issues Found
- [Specific visual problems with likely cause and fix]

### Recommendations
- [Suggested changes to improve the visual result]
```

### Tips
- Pink/magenta materials = broken asset reference → check TSoftObjectPtr paths
- Pure black surfaces = material not assigned or BaseColor is (0,0,0)
- No bloom on emissives = check post-process bloom threshold vs emissive multiplier
- Shadow popping = VSM page budget too low or shadow distance too short
