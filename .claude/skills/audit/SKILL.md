---
name: audit
description: Audit the codebase for style violations, file length limits, and code quality issues. Use when the user wants to check compliance, find problems, or review code health.
context: fork
agent: Explore
allowed-tools: Bash, Read, Grep, Glob
---

# Audit ExoRift Codebase

Scan the full codebase at `C:/Users/falk/exorift/Source/ExoRift/` and report violations.

## Checks to Run

### 1. File Length (CRITICAL — max 400 lines, aim for 250)
Count lines in every `.cpp` and `.h` file. Report:
- **Over 400 lines**: VIOLATION — must be split
- **300-400 lines**: WARNING — candidate for splitting
- Sort results by line count descending.

### 2. Missing UPROPERTY on UObject Pointers
Search for member `UObject*`-derived pointer declarations (e.g. `USomething* VarName;`) in headers that are NOT preceded by `UPROPERTY()`. These will be garbage collected. Common types to check: `UStaticMeshComponent*`, `UPointLightComponent*`, `UMaterialInstanceDynamic*`, `USceneComponent*`, `UAudioComponent*`, etc.

### 3. Missing Null Guards on MID::Create
Search for `UMaterialInstanceDynamic::Create(` calls that don't check the result for null before use.

### 4. Include Hygiene
Look for `.cpp` files that include their own header via a non-relative path (should be `"ModuleName/FileName.h"` not `"FileName.h"`).

### 5. Constructor vs NewObject Misuse
Flag any `CreateDefaultSubobject` calls outside of constructors, or `NewObject` calls inside constructors.

## Output Format

```
## Audit Results

### File Length Violations
| File | Lines | Status |
|------|-------|--------|

### UPROPERTY Issues
- file:line — description

### Null Guard Issues
- file:line — description

### Other Issues
- file:line — description

### Summary
X violations, Y warnings across Z files
```

Only report actual findings. Skip sections with zero results.
