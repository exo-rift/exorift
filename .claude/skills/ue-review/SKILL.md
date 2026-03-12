---
name: ue-review
description: Deep UE5 code quality review for performance, engine best practices, memory safety, and networking readiness. Use when the user wants a code review, wants to improve code quality, or asks about UE5 best practices for specific files.
argument-hint: [file or module path]
allowed-tools: Read, Grep, Glob, Bash
---

# UE5 Best Practices Code Review

Review the specified file(s) or module for UE5 engine best practices, performance, and correctness.

Target: `$ARGUMENTS` (if empty, review recently modified files via `git diff --name-only`)

## Review Checklist

### Memory & GC Safety (CRITICAL)
- [ ] All `UObject*` member pointers have `UPROPERTY()` — GC will collect untagged pointers
- [ ] `TWeakObjectPtr` used for cross-actor references that may go stale
- [ ] `CreateDefaultSubobject` only in constructors, `NewObject`/`SpawnActor` everywhere else
- [ ] No raw `new`/`delete` for UObjects — always use `NewObject<>` or factory methods
- [ ] `UMaterialInstanceDynamic::Create()` results null-checked before use
- [ ] No dangling pointers after `Destroy()` — check for stored refs to actors that may be destroyed

### Performance
- [ ] `PrimaryActorTick.bCanEverTick` only enabled when Tick is actually needed
- [ ] Tick-heavy actors use tick intervals or disable tick when idle (e.g., `SetActorTickEnabled(false)`)
- [ ] Large `TArray` operations avoid per-frame allocation — `Reserve()`, reuse arrays
- [ ] `FindComponentByClass` / `GetAllActorsOfClass` not called every frame — cache results
- [ ] Line traces not run every frame unless necessary — use timers or intervals
- [ ] No `LoadObject`/`StaticLoadObject` at runtime — use `TSoftObjectPtr` + async loading
- [ ] `ConstructorHelpers::FObjectFinder` only used in constructors (crashes if used elsewhere)
- [ ] Material parameter updates batched where possible, not per-frame for static values

### UE5 Patterns
- [ ] `UFUNCTION()` on any function bound with `AddDynamic`
- [ ] Enhanced Input: `AddMappingContext` called after possession, not in BeginPlay
- [ ] `UPROPERTY(EditDefaultsOnly)` / `EditAnywhere` / `VisibleAnywhere` used appropriately
- [ ] Actor components attached properly (`SetupAttachment` in constructor, `AttachToComponent` at runtime)
- [ ] Collision settings explicit — not relying on defaults that may conflict
- [ ] `InitialLifeSpan` used for self-destroying actors instead of manual timer+Destroy

### Asset References
- [ ] Marketplace/external assets use `TSoftObjectPtr<>` so missing assets don't break the build
- [ ] Hard asset paths (`/Game/...`, `/Engine/...`) only for guaranteed engine primitives
- [ ] `ConstructorHelpers::FObjectFinder` success checked before using `.Object`

### Code Quality (ExoRift-specific)
- [ ] File under 400 lines (split if over, aim for 250)
- [ ] Includes use module-relative paths (`"Module/File.h"` not `"File.h"`)
- [ ] Forward declarations in headers, full includes in .cpp
- [ ] No `.generated.h` on plain F-prefix structs/helpers
- [ ] Materials created via `FExoMaterialFactory` (GetLitEmissive, GetEmissiveAdditive, GetEmissiveOpaque)
- [ ] PBR values follow tier system: terrain (M:0.02 R:0.82-0.94), structural (M:0.35-0.60 R:0.40-0.60), accent (M:0.70-0.85 R:0.20-0.30)

### Networking Readiness (advisory)
- [ ] State that needs replication is in separate UPROPERTY(Replicated) or uses RPCs
- [ ] Cosmetic-only code (VFX, sounds) runs client-side only
- [ ] Authority checks (`HasAuthority()`) before gameplay state changes

## Output Format

```
## UE5 Code Review: [filename]

### CRITICAL (must fix)
- file:line — issue description → recommended fix

### WARNING (should fix)
- file:line — issue description → recommended fix

### SUGGESTION (nice to have)
- file:line — suggestion

### Summary
X critical, Y warnings, Z suggestions
Overall: [PASS / NEEDS WORK / FAIL]
```

Read each file fully before reviewing. Only report real issues — no false positives. Include the specific line and a concrete fix for each finding.
