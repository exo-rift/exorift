---
name: ue-lookup
description: Search online for UE5 bugs, API documentation, known issues, and community solutions. Use when hitting a UE5 error, crash, deprecation, or unexpected behavior and need to find the cause or workaround.
argument-hint: [error message, API name, or issue description]
allowed-tools: WebSearch, WebFetch, Read, Grep, Glob
---

# UE5 Bug & API Lookup

Search for information about a UE5 issue, API, or error.

Query: `$ARGUMENTS`

## Search Strategy

### Step 1: Identify the problem type
- **Compile error** → search for the exact error code/message
- **Runtime crash** → search for the callstack signature or assert message
- **Deprecation** → search for the old API + "UE5.7" or "deprecated"
- **Unexpected behavior** → search for the function/class name + symptoms

### Step 2: Search multiple sources (run in parallel)
Search these sources with targeted queries:

1. **Epic's official docs**: `site:dev.epicgames.com $ARGUMENTS UE5`
2. **UE forums/answers**: `site:forums.unrealengine.com $ARGUMENTS`
3. **Epic's issue tracker**: `site:issues.unrealengine.com $ARGUMENTS`
4. **Community**: `unrealengine $ARGUMENTS 2025 OR 2026` (recent results)

### Step 3: Check our own known issues
Search CLAUDE.md and MEMORY.md for any previously documented fixes:
```
Grep for the error/API name in:
- C:/Users/falk/exorift/CLAUDE.md
- C:/Users/falk/.claude/projects/C--Users-falk-exorift/memory/MEMORY.md
```

### Step 4: Fetch and analyze
For the most promising search results, fetch the page and extract:
- **Root cause**: Why does this happen?
- **Fix/workaround**: What's the solution?
- **Version info**: Which UE versions are affected?
- **Epic ticket**: Is there an official bug report?

## Output Format

```
## UE5 Lookup: [query]

### Problem
[Clear description of what's happening]

### Root Cause
[Why this happens — engine bug, API change, misuse, etc.]

### Solution
[Step-by-step fix or workaround with code examples]

### Sources
- [Title](URL) — brief note on what's relevant
- [Title](URL)

### Known Issues (if applicable)
- UE version affected: X.X
- Fixed in: X.X (or "unfixed")
- Epic ticket: UE-XXXXX
```

### Rules
- Always include source URLs so the user can verify
- Prefer official Epic docs/changelogs over random forum posts
- If the issue is UE5.7-specific, note whether it existed in earlier versions
- If you find a fix, check if it's already been applied in our codebase
- If this is a new known issue worth remembering, suggest adding it to CLAUDE.md's "Common UE5 Fixes" section
