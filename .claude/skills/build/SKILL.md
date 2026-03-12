---
name: build
description: Compile ExoRift with UBT and diagnose any errors. Use when the user wants to build, compile, check for errors, or fix build failures.
allowed-tools: Bash, Read, Edit, Grep, Glob, Write
---

# Build ExoRift

Run the Unreal Build Tool to compile ExoRift in Development Editor mode.

## Steps

1. **Kill stale editors** (if build fails with access denied):
   ```
   taskkill /F /IM UnrealEditor.exe 2>/dev/null
   ```

2. **Run UBT**:
   ```bash
   "C:/Program Files/Epic Games/UE_5.7/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe" \
     ExoRiftEditor Win64 Development \
     -Project="C:/Users/falk/exorift/exorift.uproject" \
     -WaitMutex -FromMsBuild 2>&1
   ```

3. **On success**: Report the compile time and confirm clean build.

4. **On failure**: Parse the error output and:
   - Identify which file(s) and line(s) have errors
   - Read those files to understand the context
   - Fix the errors directly
   - Re-run the build to confirm the fix
   - Repeat until it compiles clean

## Common Error Patterns

- **LNK2001/LNK2019 (unresolved external)**: Missing module in ExoRift.Build.cs `PrivateDependencyModuleNames`
- **"Cannot open include file"**: Add the owning module to Build.cs
- **"Access Denied"**: Kill lingering UnrealEditor.exe processes first
- **C4458 (shadow warning)**: Rename the local variable to avoid shadowing the member
- **Missing .generated.h**: Ensure UCLASS/USTRUCT macro is present and header naming matches class

## Rules
- Never modify Build.cs unless adding a missing module dependency
- Keep fixes minimal — only change what's needed to resolve the error
- If a fix requires architectural changes, explain the situation instead of guessing
