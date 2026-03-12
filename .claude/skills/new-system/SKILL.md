---
name: new-system
description: Scaffold a new C++ system (header + implementation) following ExoRift conventions. Use when the user wants to add a new class, actor, component, or system.
argument-hint: [Module/ClassName] [brief description]
allowed-tools: Read, Write, Edit, Glob, Grep
---

# Scaffold New ExoRift System

Create a new C++ class following project conventions. The user provides:
- `$1` — Module and class name, e.g. `Visual/ExoLaserBeam` or `Weapons/ExoRailgun`
- `$2` (optional) — Brief description of what it does

## File Locations

Map the module prefix to directory:
- `Core/` → `Source/ExoRift/Core/`
- `Player/` → `Source/ExoRift/Player/`
- `AI/` → `Source/ExoRift/AI/`
- `Weapons/` → `Source/ExoRift/Weapons/`
- `Map/` → `Source/ExoRift/Map/`
- `UI/` → `Source/ExoRift/UI/`
- `Visual/` → `Source/ExoRift/Visual/`
- `Vehicles/` → `Source/ExoRift/Vehicles/`

## Conventions (MUST follow)

### Header (.h)
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"  // or appropriate base
#include "ClassName.generated.h"

// Forward declarations (not full includes)
class USomeComponent;

/**
 * Brief description of the class.
 * Additional detail if needed.
 */
UCLASS()
class AClassName : public AActor  // or UComponent, etc.
{
    GENERATED_BODY()

public:
    AClassName();
    // Public API

private:
    UPROPERTY()  // ALL UObject pointers MUST have UPROPERTY()
    USomeComponent* Comp;

    // constexpr for numeric constants
    static constexpr float SomeValue = 1.f;
};
```

### Implementation (.cpp)
```cpp
#include "Module/ClassName.h"
// Additional includes
```

### Rules
- Use `EXORIFT_API` on the class ONLY if it needs to be accessed from other modules (rare — most don't)
- Tabs for indentation
- Forward-declare in headers, include in .cpp
- `CreateDefaultSubobject` only in constructors
- Mark UObject pointers with `UPROPERTY()` or GC will collect them
- All `UMaterialInstanceDynamic::Create()` results must be null-checked
- Plain helper structs (F-prefix) do NOT get .generated.h
- Use `FExoMaterialFactory` for materials (GetLitEmissive, GetEmissiveAdditive, GetEmissiveOpaque)
- Max 400 lines per file, aim for 250 — split into focused modules if needed
- No Blueprints — pure C++

## Steps

1. Parse `$ARGUMENTS` to determine module, class name, and description
2. Check if files already exist (don't overwrite!)
3. Determine the appropriate base class:
   - World-placed thing → `AActor`
   - Attachable behavior → `UActorComponent` or `USceneComponent`
   - HUD element → likely goes in ExoHUD (no separate class)
   - Data-only → `UObject` or plain `F`-struct
4. Look at 1-2 similar existing classes in the same module for patterns
5. Create the `.h` file with proper structure
6. Create the `.cpp` file with constructor and key methods stubbed
7. Report what was created and what the user should implement next
