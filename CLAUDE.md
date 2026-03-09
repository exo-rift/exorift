# ExoRift BR — Claude Code Guide

## Project
Sci-Fi Battle Royale. UE5.7, pure C++. Realistic sci-fi theme with real assets and hand-crafted levels.
**Copyright:** Spot Cloud b.v. (2026)
**Location:** `C:/Users/falk/exorift/`

## Code Style Rules
- **Max 400 lines per file, aim for 250.** Split large files into focused modules.
- No Blueprints by design — C++ throughout
- `EngineAssociation` in `.uproject` must be `"5.7"`

## Build & Compile

```bash
# Build Development Editor via UBT
"C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe" \
  ExoRiftEditor Win64 Development -Project="C:/Users/falk/exorift/exorift.uproject" -WaitMutex -FromMsBuild

# Clean rebuild (delete stale artifacts first)
rm -rf Binaries Intermediate DerivedDataCache Saved/ShaderCache
```

Requires: Visual Studio 2022 with "Game development with C++" workload.

## Project Structure

```
Source/ExoRift/
├── Core/           — GameMode, GameState, PlayerState
├── Player/         — Character, PlayerController, Input
├── AI/             — Bot character, BehaviorTree, AI controllers
├── Weapons/        — Weapon base, projectiles, heat system
├── Map/            — Zone system, spawn points, level logic
├── UI/             — HUD, menus (Canvas-based for now)
└── Visual/         — Materials, VFX, post-processing
```

## MCP Servers

### `jcodemunch` — Token-Efficient Code Exploration
Prefer over reading full files. Use to find symbols, signatures, and function defs first.

### `unreal-runreal` — Python Remote Execution (port 6776)
For querying assets, actors, viewport control. Requires UE Python Remote Execution enabled.

### `unreal-chongdashu` — UnrealMCP C++ Plugin (TCP 55557)
For creating/deleting actors, Blueprint editing. Requires UnrealMCP plugin.

### `github` — GitHub Integration
PRs, issues, code search.

### `obsidian` — Note & Documentation Management
Design docs and architecture notes in Obsidian vault.

## Environment Notes
- **Admin shell** — no permission issues for UBT, file writes, registry edits.
- **Long paths enabled** — already set in registry.
- Always quote paths containing spaces.

## Common UE5 Fixes

### Build
- **"Missing or built with different engine version"** — Check `EngineAssociation` in `.uproject` matches `"5.7"`. If still broken, delete `Binaries/` + `Intermediate/`, rebuild.
- **LNK2001/LNK2019** — Add missing module to `.Build.cs` deps.
- **"Cannot open include file"** — Add owning module to `.Build.cs`.
- **"Access Denied"** — Kill lingering `UnrealEditor.exe` processes.

### Hot Reload / Live Coding
- Header/UPROPERTY/UFUNCTION changes require full editor restart + recompile.
- Only `.cpp` function bodies are patched by Live Coding.
- Never build from VS while editor is open.

### C++ Gotchas
- Mark UObject pointers `UPROPERTY()` or GC will collect them.
- `AddDynamic` requires `UFUNCTION()` on the bound function.
- `CreateDefaultSubobject` only in constructors; `NewObject` everywhere else.
- Enhanced Input: `AddMappingContext` must be called after possession (not in BeginPlay).

## Architecture Notes
- Real assets and hand-crafted levels (not procedural)
- AI LOD: full BT <100m, simplified 100-500m, basic >500m
- Overheat bar instead of reload (heat management)
- HUD drawn on Canvas (no UMG widgets initially)
- Shrinking zone mechanic
- Drop pod spawn system
